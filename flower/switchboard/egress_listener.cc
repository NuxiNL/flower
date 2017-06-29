#include <memory>
#include <mutex>

#include <arpc++/arpc++.h>

#include <flower/proto/egress.h>
#include <flower/switchboard/egress_listener.h>
#include <flower/switchboard/label_map.h>

using arpc::ClientContext;
using arpc::FileDescriptor;
using arpc::Status;
using arpc::StatusCode;
using flower::proto::egress::Egress::Stub;
using flower::proto::egress::ConnectRequest;
using flower::proto::egress::ConnectResponse;
using flower::switchboard::EgressListener;

Status EgressListener::ConnectWithSocket(
    const LabelMap& resolved_labels,
    const std::shared_ptr<FileDescriptor>& fd) {
  return Status(StatusCode::FAILED_PRECONDITION,
                "Cannot connect an ingress with an egress");
}

Status EgressListener::ConnectWithoutSocket(
    const LabelMap& resolved_labels, std::shared_ptr<FileDescriptor>* fd) {
  std::lock_guard<std::mutex> lock_(channel_lock_);
  std::unique_ptr<Stub> stub = proto::egress::Egress::NewStub(channel_);

  // Forward incoming connection to the egress process.
  ClientContext context;
  ConnectRequest request;
  *request.mutable_labels() = resolved_labels;
  ConnectResponse response;
  if (Status status = stub->Connect(&context, request, &response); !status.ok())
    return status;
  *fd = response.server();
  return Status::OK;
}
