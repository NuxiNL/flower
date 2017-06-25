#ifndef FLOWER_UTIL_SOCKADDR_H
#define FLOWER_UTIL_SOCKADDR_H

#include <sys/socket.h>
#include <sys/un.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <cstddef>
#include <string>

namespace flower {
namespace util {
template <typename T>
void ConvertSockaddrToLabels(const sockaddr* sa, socklen_t salen,
                             const std::string& prefix, T* map) {
  if (salen < offsetof(sockaddr, sa_family) + sizeof(sockaddr::sa_family)) {
    (*map)["address_family"] = "unknown";
  } else if (sa->sa_family == AF_INET && salen >= sizeof(sockaddr_in)) {
    // IPv4 address.
    auto sin = reinterpret_cast<const sockaddr_in*>(sa);
    (*map)["address_family"] = "inet";
    char addr[INET_ADDRSTRLEN];
    (*map)[prefix + "_address"] = inet_ntop(
        AF_INET, static_cast<const void*>(&sin->sin_addr), addr, sizeof(addr));
    (*map)[prefix + "_port"] = std::to_string(ntohs(sin->sin_port));
  } else if (sa->sa_family == AF_INET6 && salen >= sizeof(sockaddr_in6)) {
    // IPv6 address. Don't convert sin6_flowinfo, as that tends to
    // contain nothing useful.
    auto sin6 = reinterpret_cast<const sockaddr_in6*>(&sa);
    (*map)["address_family"] = "inet6";
    char addr[INET6_ADDRSTRLEN];
    (*map)[prefix + "_address"] =
        inet_ntop(AF_INET6, static_cast<const void*>(&sin6->sin6_addr), addr,
                  sizeof(addr));
    (*map)[prefix + "_port"] = std::to_string(ntohs(sin6->sin6_port));
    // TODO(ed): Should this be resolved to an interface name?
    (*map)[prefix + "_scope_id"] = std::to_string(ntohs(sin6->sin6_scope_id));
  } else if (sa->sa_family == AF_UNIX) {
    // UNIX socket address. Don't convert sun_path, as it tends to
    // be of little use. For example, it may be relative to the
    // working directory of the peer.
    (*map)["address_family"] = "unix";
  } else {
    (*map)["address_family"] = "unknown";
  }
}
}
}

#endif
