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
using arpc::ClientReader;
using arpc::Status;
using flower::protocol::switchboard::ListRequest;
using flower::protocol::switchboard::ListResponse;
using flower::protocol::switchboard::Switchboard;
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
  std::unique_ptr<ClientReader<ListResponse>> reader =
      stub->List(&context, request);

  // Print the targets.
  for (ListResponse response; reader->Read(&response);) {
    LabelMapToJson(response.labels(), &list_output);
    list_output << std::endl;
  }

  if (Status status = reader->Finish(); !status.ok()) {
    logger.Log() << "Cannot start without a switchboard socket";
    std::exit(1);
  }
  std::exit(0);
}
