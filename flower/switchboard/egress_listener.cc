#include <memory>

#include <arpc++/arpc++.h>

#include <flower/switchboard/egress_listener.h>
#include <flower/switchboard/label_map.h>

using arpc::FileDescriptor;
using arpc::Status;
using arpc::StatusCode;
using flower::switchboard::EgressListener;

Status EgressListener::Start(std::unique_ptr<FileDescriptor>* fd) {
  return Status(StatusCode::UNIMPLEMENTED, "TODO(ed): Implement!");
}

Status EgressListener::ConnectWithSocket(
    const LabelMap& resolved_labels,
    const std::shared_ptr<FileDescriptor>& fd) {
  return Status(StatusCode::FAILED_PRECONDITION,
                "Cannot connect an ingress with an egress");
}

Status EgressListener::ConnectWithoutSocket(
    const LabelMap& resolved_labels, std::shared_ptr<FileDescriptor>* fd) {
  return Status(StatusCode::UNIMPLEMENTED, "TODO(ed): Implement!");
}
