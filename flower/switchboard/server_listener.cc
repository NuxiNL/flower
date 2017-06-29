#include <memory>
#include <mutex>

#include <arpc++/arpc++.h>

#include <flower/protocol/server.ad.h>
#include <flower/switchboard/label_map.h>
#include <flower/switchboard/server_listener.h>
#include <flower/util/socket.h>

using arpc::ClientContext;
using arpc::FileDescriptor;
using arpc::Status;
using arpc::StatusCode;
using flower::protocol::server::ConnectRequest;
using flower::protocol::server::ConnectResponse;
using flower::protocol::server::Server::NewStub;
using flower::protocol::server::Server::Stub;
using flower::switchboard::ServerListener;
using flower::util::CreateSocketpair;

Status ServerListener::ConnectWithSocket(
    const LabelMap& connection_labels,
    const std::shared_ptr<FileDescriptor>& fd) {
  std::lock_guard<std::mutex> lock_(channel_lock_);
  std::unique_ptr<Stub> stub = NewStub(channel_);

  // Forward incoming connection to the server process.
  ClientContext context;
  ConnectRequest request;
  request.set_client(fd);
  *request.mutable_connection_labels() = connection_labels;
  ConnectResponse response;
  return stub->Connect(&context, request, &response);
}

Status ServerListener::ConnectWithoutSocket(
    const LabelMap& connection_labels, std::shared_ptr<FileDescriptor>* fd) {
  // Attempting to create a direct connection between a client and a
  // server. As none of these two parties already possess a socket,
  // allocate a socket pair and hand out both ends.
  std::unique_ptr<FileDescriptor> fd1, fd2;
  if (Status status = CreateSocketpair(&fd1, &fd2); !status.ok())
    return status;
  if (Status status = ConnectWithSocket(connection_labels, std::move(fd1));
      !status.ok())
    return status;
  *fd = std::move(fd2);
  return Status::OK;
}
