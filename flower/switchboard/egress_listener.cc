// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <memory>
#include <mutex>

#include <arpc++/arpc++.h>

#include <flower/protocol/egress.ad.h>
#include <flower/switchboard/egress_listener.h>
#include <flower/switchboard/label_map.h>

using arpc::ClientContext;
using arpc::FileDescriptor;
using arpc::Status;
using arpc::StatusCode;
using flower::protocol::egress::ConnectRequest;
using flower::protocol::egress::ConnectResponse;
using flower::protocol::egress::Egress::NewStub;
using flower::protocol::egress::Egress::Stub;
using flower::switchboard::EgressListener;

Status EgressListener::ConnectWithSocket(
    const LabelMap& connection_labels,
    const std::shared_ptr<FileDescriptor>& fd) {
  return Status(StatusCode::FAILED_PRECONDITION,
                "Cannot connect an ingress with an egress");
}

Status EgressListener::ConnectWithoutSocket(
    const LabelMap& connection_labels, std::shared_ptr<FileDescriptor>* fd) {
  std::lock_guard<std::mutex> lock_(channel_lock_);
  std::unique_ptr<Stub> stub = NewStub(channel_);

  // Forward incoming connection to the egress process.
  ClientContext context;
  ConnectRequest request;
  *request.mutable_connection_labels() = connection_labels;
  ConnectResponse response;
  if (Status status = stub->Connect(&context, request, &response); !status.ok())
    return status;
  if (!response.server())
    return Status(StatusCode::INTERNAL,
                  "Egress did not return a file descriptor");
  *fd = response.server();
  return Status::OK;
}
