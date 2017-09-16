// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <signal.h>

#include <cstdlib>
#include <iostream>

#include <arpc++/arpc++.h>

#include <flower/ls/configuration.ad.h>
#include <flower/ls/start.h>
#include <flower/protocol/switchboard.ad.h>
#include <flower/util/switchboard.h>

using arpc::FileDescriptor;
using arpc::Status;
using flower::ls::Configuration;
using flower::ls::Start;
using flower::protocol::switchboard::Right;
using flower::util::CreateSwitchboardChannel;

int main(int argc, char* argv[]) {
  Configuration configuration;
  {
    if (argc != 3) {
      std::cerr << "Usage: flower_ls switchboard_path switchboard_labels"
                << std::endl;
      std::exit(1);
    }

    signal(SIGPIPE, SIG_IGN);

    std::shared_ptr<FileDescriptor> switchboard_socket;
    if (Status status = CreateSwitchboardChannel(argv[1], {Right::LIST},
                                                 argv[2], &switchboard_socket);
        !status.ok()) {
      std::cerr << status.error_message() << std::endl;
      return 1;
    }
    configuration.set_switchboard_socket(std::move(switchboard_socket));

    configuration.set_list_output(
        std::make_unique<FileDescriptor>(STDOUT_FILENO));
    configuration.set_logger_output(
        std::make_unique<FileDescriptor>(STDERR_FILENO));
  }
  Start(configuration);
}
