// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <signal.h>

#include <iostream>
#include <memory>

#include <arpc++/arpc++.h>

#include <flower/egress/connect/configuration.ad.h>
#include <flower/egress/connect/start.h>
#include <flower/protocol/switchboard.ad.h>
#include <flower/util/switchboard.h>

using arpc::FileDescriptor;
using arpc::Status;
using flower::egress::connect::Configuration;
using flower::egress::connect::Start;
using flower::protocol::switchboard::Right;
using flower::util::CreateSwitchboardChannel;

int main(int argc, char* argv[]) {
  Configuration configuration;
  {
    if (argc != 3) {
      std::cerr << "Usage: flower_egress_connect "
                   "switchboard_path switchboard_labels"
                << std::endl;
      return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    std::shared_ptr<FileDescriptor> switchboard_socket;
    if (Status status = CreateSwitchboardChannel(argv[1], {Right::EGRESS_START},
                                                 argv[2], &switchboard_socket);
        !status.ok()) {
      std::cerr << status.error_message() << std::endl;
      return 1;
    }
    configuration.set_switchboard_socket(std::move(switchboard_socket));

    configuration.set_logger_output(
        std::make_unique<FileDescriptor>(STDERR_FILENO));
  }
  Start(configuration);
}
