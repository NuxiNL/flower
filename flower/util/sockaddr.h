#ifndef FLOWER_UTIL_SOCKADDR_H
#define FLOWER_UTIL_SOCKADDR_H

#include <sys/socket.h>

#include <netdb.h>

#include <cstddef>
#include <string>

namespace flower {
namespace util {

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

}  // namespace util
}  // namespace flower

#endif
