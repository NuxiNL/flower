#include <sys/socket.h>

#include <memory>

#include <arpc++/arpc++.h>

#include <flower/protocol/switchboard.ad.h>
#include <flower/util/sockaddr.h>
#include <flower/util/socket.h>

using arpc::Channel;
using arpc::ClientContext;
using arpc::FileDescriptor;
using arpc::Status;
using flower::protocol::switchboard::IngressConnectRequest;
using flower::protocol::switchboard::IngressConnectResponse;
using flower::protocol::switchboard::Switchboard::NewStub;
using flower::protocol::switchboard::Switchboard::Stub;
using flower::util::AcceptSocketConnection;
using flower::util::ConvertSockaddrToLabels;

int main() {
  // TODO(ed): Get file descriptors from somewhere.
  auto switchboard_fd = std::make_shared<FileDescriptor>(-1);
  auto accept_fd = std::make_shared<FileDescriptor>(-1);

  std::shared_ptr<Channel> channel = CreateChannel(switchboard_fd);
  std::unique_ptr<Stub> stub = NewStub(channel);
  for (;;) {
    // Accept an incoming connection.
    union {
      sockaddr sa;
      sockaddr_storage ss;
    } client_address;
    socklen_t client_address_len = sizeof(client_address);
    std::unique_ptr<FileDescriptor> connection;
    if (Status status = AcceptSocketConnection(
            *accept_fd, &connection, &client_address.sa, &client_address_len);
        !status.ok()) {
      // TODO(ed): Log errors!
      return 1;
    }

    // Store file descriptors in request and convert peer adress to labels.
    IngressConnectRequest request;
    request.set_client(std::move(connection));
    ConvertSockaddrToLabels(&client_address.sa, client_address_len, "client",
                            request.mutable_labels());

    // Convert local address to labels.
    union {
      sockaddr sa;
      sockaddr_storage ss;
    } server_address;
    socklen_t server_address_len = sizeof(server_address);
    if (getsockname(request.client()->get(), &server_address.sa,
                    &server_address_len) == 0)
      ConvertSockaddrToLabels(&server_address.sa, server_address_len, "server",
                              request.mutable_labels());

    // Submit connection to the switchboard.
    ClientContext context;
    IngressConnectResponse response;
    Status status = stub->IngressConnect(&context, request, &response);
    if (!status.ok()) {
      // TODO(ed): Log errors!
      // TODO(ed): Distinguish between transient and non-transient errors.
      return 1;
    }
  }
}
