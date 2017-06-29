// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <memory>
#include <utility>

#include <arpc++/arpc++.h>

#include <flower/switchboard/egress_listener.h>
#include <flower/switchboard/label_map.h>
#include <flower/util/socket.h>

using arpc::CreateChannel;
using arpc::FileDescriptor;
using arpc::Status;
using flower::switchboard::Listener;
using flower::util::CreateSocketpair;

Status Listener::Start(std::unique_ptr<FileDescriptor>* fd) {
  std::unique_ptr<FileDescriptor> server_fd, channel_fd;
  if (Status status = CreateSocketpair(&server_fd, &channel_fd); !status.ok())
    return status;
  *fd = std::move(server_fd);
  channel_ = CreateChannel(std::move(channel_fd));
  return Status::OK;
}
