// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <signal.h>

#include <iostream>

#include <arpc++/arpc++.h>

#include <flower/switchboard/configuration.ad.h>
#include <flower/switchboard/start.h>
#include <flower/util/socket.h>

using arpc::FileDescriptor;
using arpc::Status;
using flower::switchboard::Configuration;
using flower::switchboard::Start;
using flower::util::CreateListeningUnixSocket;

int main(int argc, char* argv[]) {
  Configuration configuration;
  {
    if (argc != 2) {
      std::cerr << "Usage: flower_switchboard socket_path" << std::endl;
      return 1;
    }

    std::unique_ptr<FileDescriptor> listening_socket;
    if (Status status = CreateListeningUnixSocket(argv[1], &listening_socket);
        !status.ok()) {
      std::cerr << status.error_message() << std::endl;
      return 1;
    }
    configuration.set_listening_socket(std::move(listening_socket));

    signal(SIGPIPE, SIG_IGN);

    configuration.set_logger_output(
        std::make_unique<FileDescriptor>(STDERR_FILENO));
    // TODO(ed): Make this configurable.
    configuration.set_worker_pool_size(10);
  }
  Start(configuration);
}
