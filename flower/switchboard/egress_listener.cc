// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <memory>

#include <arpc++/arpc++.h>

#include <flower/protocol/egress.ad.h>
#include <flower/switchboard/egress_listener.h>
#include <flower/util/label_map.h>

using arpc::ClientContext;
using arpc::FileDescriptor;
using arpc::Status;
using arpc::StatusCode;
using flower::protocol::egress::ConnectRequest;
using flower::protocol::egress::ConnectResponse;
using flower::protocol::egress::Egress;
using flower::switchboard::EgressListener;
using flower::util::LabelMap;

Status EgressListener::ConnectWithSocket(
    const LabelMap& connection_labels,
    const std::shared_ptr<FileDescriptor>& fd) {
  return Status(StatusCode::FAILED_PRECONDITION,
                "Cannot connect an ingress with an egress");
}

Status EgressListener::ConnectWithoutSocket(
    const LabelMap& connection_labels, std::shared_ptr<FileDescriptor>* fd) {
  std::unique_ptr<Egress::Stub> stub = Egress::NewStub(channel_);

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

bool EgressListener::IsDead() {
  return channel_->GetState(false) == ARPC_CHANNEL_SHUTDOWN;
}
