#include <sys/socket.h>
#include <sys/un.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <arpc++/arpc++.h>

#include <map>
#include <memory>
#include <string>

#include "flower_service.h"

// Converts a sockaddr to a set of labels, containing a string
// representation of the address within.
namespace {
union SocketAddress {
 public:
  template <typename T>
  void ExtractLabels(const std::string& prefix, T* map) const {
    switch (sa.sa_family) {
      case AF_INET: {
        // IPv4 address.
        (*map)["address_family"] = "inet";
        char addr[INET_ADDRSTRLEN];
        (*map)[prefix + "address"] =
            inet_ntop(AF_INET, static_cast<const void*>(&sin_.sin_addr), addr,
                      sizeof(addr));
        (*map)[prefix + "port"] = std::to_string(ntohs(sin_.sin_port));
        break;
      }
      case AF_INET6: {
        // IPv6 address. Don't convert sin6_flowinfo, as that tends to
        // contain nothing useful.
        (*map)["address_family"] = "inet6";
        char addr[INET6_ADDRSTRLEN];
        (*map)[prefix + "address"] =
            inet_ntop(AF_INET6, static_cast<const void*>(&sin6_.sin6_addr),
                      addr, sizeof(addr));
        (*map)[prefix + "port"] = std::to_string(ntohs(sin6_.sin6_port));
        // TODO(ed): Should this be resolved to an interface name?
        (*map)[prefix + "scope_id"] =
            std::to_string(ntohs(sin6_.sin6_scope_id));
        break;
      }
      case AF_UNIX:
        // UNIX socket address. Don't convert sun_path, as it tends to
        // be of little use. For example, it may be relative to the
        // working directory of the peer.
        (*map)["address_family"] = "unix";
        break;
      default:
        (*map)["address_family"] = "unknown";
        break;
    }
  }

  sockaddr sa;

 private:
  sockaddr_in sin_;
  sockaddr_in6 sin6_;
};
}

int main() {
  // TODO(ed): Get file descriptors from somewhere.
  auto switchboard_fd = std::make_shared<arpc::FileDescriptor>(-1);
  auto accept_fd = std::make_shared<arpc::FileDescriptor>(-1);

  std::shared_ptr<arpc::Channel> channel = arpc::CreateChannel(switchboard_fd);
  std::unique_ptr<flower_service::IngressChannelService::Stub> stub =
      flower_service::IngressChannelService::NewStub(channel);
  for (;;) {
    // Call accept() to request an incoming connection and store the
    // return values in the switchboard request.
    flower_service::IngressSubmitRequest request;
    SocketAddress src_address;
    socklen_t src_address_len = sizeof(src_address);
    int ret = accept(accept_fd->get(), &src_address.sa, &src_address_len);
    if (ret < 0) {
      // TODO(ed): Log errors!
      return 1;
    }
    request.set_socket(std::make_unique<arpc::FileDescriptor>(ret));
    src_address.ExtractLabels("src_", request.mutable_additional_constraints());

    // Similarly, convert getsockname() output to labels.
    SocketAddress dst_address;
    socklen_t dst_address_len = sizeof(dst_address);
    if (getsockname(request.socket()->get(), &dst_address.sa,
                    &dst_address_len) == 0)
      dst_address.ExtractLabels("dst_",
                                request.mutable_additional_constraints());

    // Submit connection to the switchboard.
    arpc::ClientContext context;
    flower_service::IngressSubmitResponse response;
    arpc::Status status = stub->Submit(&context, request, &response);
    if (!status.ok()) {
      // TODO(ed): Log errors!
      // TODO(ed): Distinguish between transient and non-transient errors.
      return 1;
    }
  }
}
