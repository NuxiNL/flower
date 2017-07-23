// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <signal.h>

#include <iostream>
#include <memory>

#include <arpc++/arpc++.h>

#include <flower/ingress/accept/configuration.ad.h>
#include <flower/ingress/accept/start.h>
#include <flower/protocol/switchboard.ad.h>
#include <flower/util/socket.h>
#include <flower/util/switchboard.h>

using arpc::FileDescriptor;
using arpc::Status;
using flower::ingress::accept::Configuration;
using flower::ingress::accept::Start;
using flower::protocol::switchboard::Right;
using flower::util::CreateListeningSocket;
using flower::util::CreateSwitchboardChannel;

int main(int argc, char* argv[]) {
  Configuration configuration;
  {
    if (argc != 4) {
      std::cerr << "Usage: flower_ingress_accept listen_address "
                   "switchboard_path switchboard_labels"
                << std::endl;
      return 1;
    }

    signal(SIGPIPE, SIG_IGN);

    std::unique_ptr<FileDescriptor> listening_socket;
    if (Status status = CreateListeningSocket(argv[1], &listening_socket);
        !status.ok()) {
      std::cerr << status.error_message() << std::endl;
      return 1;
    }
    configuration.set_listening_socket(std::move(listening_socket));

    std::shared_ptr<FileDescriptor> switchboard_socket;
    if (Status status = CreateSwitchboardChannel(
            argv[2], {Right::INGRESS_CONNECT}, argv[3], &switchboard_socket);
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
