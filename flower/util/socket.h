// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_UTIL_SOCKETPAIR_H
#define FLOWER_UTIL_SOCKETPAIR_H

#include <sys/socket.h>
#include <sys/un.h>

#include <netdb.h>

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>

#include <arpc++/arpc++.h>

namespace flower {
namespace util {
namespace {

class AddrinfoDeleter {
 public:
  void operator()(addrinfo* ai) const {
    if (ai != nullptr)
      freeaddrinfo(ai);
  }
};

int GetAddrinfo(const char* hostname, const char* servname,
                const addrinfo* hints,
                std::unique_ptr<addrinfo, AddrinfoDeleter>* res) {
  addrinfo* res_ptr = nullptr;
  int error = getaddrinfo(hostname, servname, hints, &res_ptr);
  res->reset(res_ptr);
  return error;
}

arpc::Status CreateSocketpair(std::unique_ptr<arpc::FileDescriptor>* fd1,
                              std::unique_ptr<arpc::FileDescriptor>* fd2) {
  int fds[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
    std::ostringstream ss;
    ss << "Failed to create socket pair: " << std::strerror(errno);
    return arpc::Status(arpc::StatusCode::INTERNAL, ss.str());
  }
  *fd1 = std::make_unique<arpc::FileDescriptor>(fds[0]);
  *fd2 = std::make_unique<arpc::FileDescriptor>(fds[1]);
  return arpc::Status::OK;
}

arpc::Status AcceptSocketConnection(const arpc::FileDescriptor& fd,
                                    std::unique_ptr<arpc::FileDescriptor>* conn,
                                    sockaddr* sa, socklen_t* salen) {
  int ret = accept(fd.get(), sa, salen);
  if (ret < 0) {
    std::ostringstream ss;
    ss << "Failed to accept incoming connection: " << std::strerror(errno);
    return arpc::Status(arpc::StatusCode::INTERNAL, ss.str());
  }
  *conn = std::make_unique<arpc::FileDescriptor>(ret);
  return arpc::Status::OK;
}

template <typename T>
void ConvertSockaddrToLabels(const sockaddr* sa, socklen_t salen,
                             const std::string& prefix, T* map) {
  // Convert address family.
  if (salen >= offsetof(sockaddr, sa_family) + sizeof(sockaddr::sa_family)) {
    switch (sa->sa_family) {
      case AF_INET:
        (*map)["address_family"] = "inet";
        break;
      case AF_INET6:
        (*map)["address_family"] = "inet6";
        break;
      case AF_UNIX:
        (*map)["address_family"] = "unix";
        break;
    }
  }

  // For AF_INET and AF_INET6, add address and port labels.
  char address[NI_MAXHOST];
  char port[NI_MAXSERV];
  if (getnameinfo(sa, salen, address, sizeof(address), port, sizeof(port),
                  NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
    (*map)[prefix + "_address"] = address;
    (*map)[prefix + "_port"] = port;
  }
}

#ifndef __CloudABI__

arpc::Status CreateSocket(int domain,
                          std::unique_ptr<arpc::FileDescriptor>* fd) {
  int ret = socket(domain, SOCK_STREAM, 0);
  if (ret < 0) {
    std::ostringstream ss;
    ss << "Failed to create socket: " << std::strerror(errno);
    return arpc::Status(arpc::StatusCode::INTERNAL, ss.str());
  }
  *fd = std::make_unique<arpc::FileDescriptor>(ret);
  return arpc::Status::OK;
}

arpc::Status InitializeSockaddrUn(const char* path, sockaddr_un* sun) {
  if (std::strlen(path) >= sizeof(sun->sun_path))
    return arpc::Status(arpc::StatusCode::INVALID_ARGUMENT, "Path too long");
  sun->sun_family = AF_UNIX;
  std::strcpy(sun->sun_path, path);
  return arpc::Status::OK;
}

arpc::Status CreateConnectedUnixSocket(
    const char* path, std::unique_ptr<arpc::FileDescriptor>* connected_socket) {
  // Create socket.
  std::unique_ptr<arpc::FileDescriptor> s;
  if (arpc::Status status = CreateSocket(AF_UNIX, &s); !status.ok())
    return status;

  // Attempt to connect.
  union {
    sockaddr sa;
    sockaddr_un sun;
  } addr;
  if (arpc::Status status = InitializeSockaddrUn(path, &addr.sun); !status.ok())
    return status;
  if (connect(s->get(), &addr.sa, sizeof(addr)) != 0) {
    std::ostringstream ss;
    ss << "Failed to connect to " << path << ": " << std::strerror(errno);
    return arpc::Status(arpc::StatusCode::INTERNAL, ss.str());
  }

  *connected_socket = std::move(s);
  return arpc::Status::OK;
}

arpc::Status CreateListeningNetworkSocket(
    const char* address,
    std::unique_ptr<arpc::FileDescriptor>* listening_socket) {
  // Decompose address string into hostname and port number.
  const char *hostname_begin, *hostname_end, *servname;
  if (*address == '[') {
    hostname_begin = address + 1;
    hostname_end = std::strstr(hostname_begin, "]:");
    if (hostname_end == nullptr)
      return arpc::Status(arpc::StatusCode::INVALID_ARGUMENT,
                          "Malformed address");
    servname = hostname_end + 2;
  } else {
    hostname_begin = address;
    hostname_end = std::strchr(hostname_begin, ':');
    if (hostname_end == nullptr)
      return arpc::Status(arpc::StatusCode::INVALID_ARGUMENT,
                          "Malformed address");
    servname = hostname_end + 1;
  }

  // Parse address.
  addrinfo hints = {};
  hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV;
  hints.ai_socktype = SOCK_STREAM;
  std::unique_ptr<addrinfo, AddrinfoDeleter> res;
  if (int error = GetAddrinfo(std::string(hostname_begin, hostname_end).c_str(),
                              servname, &hints, &res);
      error != 0) {
    std::ostringstream ss;
    ss << "Failed to parse address" << address << ": " << gai_strerror(error);
    return arpc::Status(arpc::StatusCode::INVALID_ARGUMENT, ss.str());
  }

  // Create socket.
  std::unique_ptr<arpc::FileDescriptor> s;
  if (arpc::Status status = CreateSocket(res->ai_family, &s); !status.ok())
    return status;

  // Allow fast reuse of previously allocated ports.
  {
    int on = 1;
    setsockopt(s->get(), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
  }

  // Attempt to bind it.
  if (bind(s->get(), res->ai_addr, res->ai_addrlen) != 0) {
    std::ostringstream ss;
    ss << "Failed to bind to " << address << ": " << std::strerror(errno);
    return arpc::Status(arpc::StatusCode::INTERNAL, ss.str());
  }

  // Listen for incoming connections.
  if (listen(s->get(), SOMAXCONN) != 0) {
    std::ostringstream ss;
    ss << "Failed to listen for incoming connections: " << std::strerror(errno);
    return arpc::Status(arpc::StatusCode::INTERNAL, ss.str());
  }

  *listening_socket = std::move(s);
  return arpc::Status::OK;
}

arpc::Status CreateListeningUnixSocket(
    const char* path, std::unique_ptr<arpc::FileDescriptor>* listening_socket) {
  // Create socket.
  std::unique_ptr<arpc::FileDescriptor> s;
  if (arpc::Status status = CreateSocket(AF_UNIX, &s); !status.ok())
    return status;

  // Attempt to bind it.
  union {
    sockaddr sa;
    sockaddr_un sun;
  } addr;
  if (arpc::Status status = InitializeSockaddrUn(path, &addr.sun); !status.ok())
    return status;
  std::remove(path);
  if (bind(s->get(), &addr.sa, sizeof(addr)) != 0) {
    std::ostringstream ss;
    ss << "Failed to bind to " << path << ": " << std::strerror(errno);
    return arpc::Status(arpc::StatusCode::INTERNAL, ss.str());
  }

  // Listen for incoming connections.
  if (listen(s->get(), SOMAXCONN) != 0) {
    std::ostringstream ss;
    ss << "Failed to listen for incoming connections: " << std::strerror(errno);
    return arpc::Status(arpc::StatusCode::INTERNAL, ss.str());
  }

  *listening_socket = std::move(s);
  return arpc::Status::OK;
}

arpc::Status CreateListeningSocket(
    const char* address,
    std::unique_ptr<arpc::FileDescriptor>* listening_socket) {
  if (address[0] == '/')
    return CreateListeningUnixSocket(address, listening_socket);
  return CreateListeningNetworkSocket(address, listening_socket);
}

#endif

}  // namespace
}  // namespace util
}  // namespace flower

#endif
