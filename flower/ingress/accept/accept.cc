#include <sys/socket.h>

#include <memory>

#include <arpc++/arpc++.h>

#include <flower/proto/switchboard.h>
#include <flower/util/sockaddr.h>

using namespace flower;

int main() {
  // TODO(ed): Get file descriptors from somewhere.
  auto switchboard_fd = std::make_shared<arpc::FileDescriptor>(-1);
  auto accept_fd = std::make_shared<arpc::FileDescriptor>(-1);

  std::shared_ptr<arpc::Channel> channel = arpc::CreateChannel(switchboard_fd);
  std::unique_ptr<proto::switchboard::Switchboard::Stub> stub =
      proto::switchboard::Switchboard::NewStub(channel);
  for (;;) {
    // Accept an incoming connection.
    union {
      sockaddr sa;
      sockaddr_storage ss;
    } client_address;
    socklen_t client_address_len = sizeof(client_address);
    int ret = accept(accept_fd->get(), &client_address.sa, &client_address_len);
    if (ret < 0) {
      // TODO(ed): Log errors!
      return 1;
    }

    // Store file descriptors in request and convert peer adress to labels.
    proto::switchboard::IngressConnectRequest request;
    request.set_client(std::make_unique<arpc::FileDescriptor>(ret));
    util::ConvertSockaddrToLabels(&client_address.sa, client_address_len,
                                  "client", request.mutable_labels());

    // Convert local address to labels.
    union {
      sockaddr sa;
      sockaddr_storage ss;
    } server_address;
    socklen_t server_address_len = sizeof(server_address);
    if (getsockname(request.client()->get(), &server_address.sa,
                    &server_address_len) == 0)
      util::ConvertSockaddrToLabels(&server_address.sa, server_address_len,
                                    "server", request.mutable_labels());

    // Submit connection to the switchboard.
    arpc::ClientContext context;
    proto::switchboard::IngressConnectResponse response;
    arpc::Status status = stub->IngressConnect(&context, request, &response);
    if (!status.ok()) {
      // TODO(ed): Log errors!
      // TODO(ed): Distinguish between transient and non-transient errors.
      return 1;
    }
  }
}
