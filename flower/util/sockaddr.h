// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_UTIL_SOCKADDR_H
#define FLOWER_UTIL_SOCKADDR_H

#include <sys/socket.h>
#include <sys/un.h>

#include <netdb.h>

#include <cstddef>
#include <cstring>
#include <string>

#include <arpc++/arpc++.h>

namespace flower {
namespace util {
namespace {

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
arpc::Status InitializeSockaddrUn(const char* path, sockaddr_un* sun) {
  if (std::strlen(path) >= sizeof(sun->sun_path))
    return arpc::Status(arpc::StatusCode::INVALID_ARGUMENT, "Path too long");
  sun->sun_family = AF_UNIX;
  std::strcpy(sun->sun_path, path);
  return arpc::Status::OK;
}
#endif

}  // namespace
}  // namespace util
}  // namespace flower

#endif
