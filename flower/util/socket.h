#ifndef FLOWER_UTIL_SOCKETPAIR_H
#define FLOWER_UTIL_SOCKETPAIR_H

#include <sys/socket.h>

#include <cstring>
#include <memory>
#include <sstream>

#include <arpc++/arpc++.h>

namespace flower {
namespace util {
namespace {

arpc::Status CreateSocket(std::unique_ptr<arpc::FileDescriptor>* fd) {
  int ret = socket(AF_UNIX, SOCK_STREAM, 0);
  if (ret < 0) {
    std::ostringstream ss;
    ss << "Failed to create socket: " << std::strerror(errno);
    return arpc::Status(arpc::StatusCode::INTERNAL, ss.str());
  }
  *fd = std::make_unique<arpc::FileDescriptor>(ret);
  return arpc::Status::OK;
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

}  // namespace
}  // namespace util
}  // namespace flower

#endif
