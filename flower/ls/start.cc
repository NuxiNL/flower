// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <cstdlib>
#include <ostream>

#include <arpc++/arpc++.h>

#include <flower/ls/configuration.ad.h>
#include <flower/ls/start.h>
#include <flower/protocol/switchboard.ad.h>
#include <flower/util/fd_streambuf.h>
#include <flower/util/label_map.h>
#include <flower/util/logger.h>
#include <flower/util/null_streambuf.h>

using arpc::Channel;
using arpc::ClientContext;
using arpc::Status;
using flower::protocol::switchboard::ListRequest;
using flower::protocol::switchboard::ListResponse;
using flower::protocol::switchboard::Switchboard;
using flower::protocol::switchboard::TargetInfo;
using flower::util::LabelMapToJson;
using flower::util::Logger;
using flower::util::fd_streambuf;
using flower::util::null_streambuf;

void flower::ls::Start(const Configuration& configuration) {
  // Make logging work.
  std::unique_ptr<std::streambuf> logger_streambuf;
  if (const auto& logger_output = configuration.logger_output();
      logger_output) {
    logger_streambuf = std::make_unique<fd_streambuf>(logger_output);
  } else {
    logger_streambuf = std::make_unique<null_streambuf>();
  }
  Logger logger(logger_streambuf.get());

  // Check that all required fields are present.
  const auto& switchboard_socket = configuration.switchboard_socket();
  if (!switchboard_socket) {
    logger.Log() << "Cannot start without a switchboard socket";
    std::exit(1);
  }
  const auto& list_output_fd = configuration.list_output();
  if (!list_output_fd) {
    logger.Log() << "Cannot start without an descriptor for listing output";
    std::exit(1);
  }
  fd_streambuf list_output_streambuf(list_output_fd);
  std::ostream list_output(&list_output_streambuf);

  // Request a list of registered targets from the Switchboard.
  std::shared_ptr<Channel> channel = CreateChannel(switchboard_socket);
  std::unique_ptr<Switchboard::Stub> stub = Switchboard::NewStub(channel);
  ClientContext context;
  ListRequest request;
  ListResponse response;
  if (Status status = stub->List(&context, request, &response); !status.ok()) {
    logger.Log() << status.error_message();
    std::exit(1);
  }

  // Print the targets.
  for (const TargetInfo& info : response.targets()) {
    LabelMapToJson(info.labels(), &list_output);
    list_output << std::endl;
  }
  std::exit(0);
}
