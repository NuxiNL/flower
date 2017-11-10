// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FLOWER_UTIL_SWITCHBOARD_H
#define FLOWER_UTIL_SWITCHBOARD_H

#include <algorithm>
#include <iterator>
#include <memory>
#include <sstream>

#include <arpc++/arpc++.h>

#include <flower/protocol/switchboard.ad.h>
#include <flower/util/label_map.h>
#include <flower/util/socket.h>

namespace flower {
namespace util {
namespace {

arpc::Status CreateSwitchboardChannel(
    const char* socket_path,
    const std::set<protocol::switchboard::Right>& rights, const char* labels,
    std::shared_ptr<arpc::FileDescriptor>* switchboard_channel) {
  // Parse user-provided JSON containing the labels.
  LabelMap parsed_labels;
  std::istringstream ss(labels);
  if (!JsonToLabelMap(&ss, &parsed_labels))
    return arpc::Status(arpc::StatusCode::INVALID_ARGUMENT,
                        "Failed to extract labels from JSON input");

  // Open a connection to the switchboard.
  std::unique_ptr<arpc::FileDescriptor> switchboard_socket;
  if (arpc::Status status =
          CreateConnectedUnixSocket(socket_path, &switchboard_socket);
      !status.ok())
    return status;
  std::shared_ptr<arpc::Channel> channel(
      arpc::CreateChannel(std::move(switchboard_socket)));
  std::unique_ptr<protocol::switchboard::Switchboard::Stub> stub =
      protocol::switchboard::Switchboard::NewStub(channel);

  // Obtain a new constrained channel with limited rights.
  arpc::ClientContext context;
  protocol::switchboard::ConstrainRequest request;
  std::copy(rights.begin(), rights.end(),
            std::back_inserter(*request.mutable_rights()));
  *request.mutable_in_labels() = parsed_labels;
  *request.mutable_out_labels() = parsed_labels;
  protocol::switchboard::ConstrainResponse response;
  if (arpc::Status status = stub->Constrain(&context, request, &response);
      !status.ok())
    return status;

  *switchboard_channel = response.switchboard();
  return arpc::Status::OK;
}

}  // namespace
}  // namespace util
}  // namespace flower

#endif
