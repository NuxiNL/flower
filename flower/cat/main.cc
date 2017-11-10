// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <signal.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

#include <arpc++/arpc++.h>

#include <flower/cat/configuration.ad.h>
#include <flower/cat/start.h>
#include <flower/protocol/switchboard.ad.h>
#include <flower/util/switchboard.h>

using arpc::FileDescriptor;
using arpc::Status;
using flower::cat::Configuration;
using flower::cat::Start;
using flower::protocol::switchboard::Right;
using flower::util::CreateSwitchboardChannel;

namespace {

void usage(void) {
  std::cerr << "Usage: flower_cat [-l] switchboard_path switchboard_labels"
            << std::endl;
  std::exit(1);
}

}  // namespace

int main(int argc, char* argv[]) {
  Configuration configuration;
  {
    // Parse command line flags.
    int ch;
    while ((ch = getopt(argc, argv, "l")) != -1) {
      switch (ch) {
        case 'l':
          configuration.set_listen(true);
          break;
        default:
          usage();
      }
    }
    argc -= optind;
    argv += optind;
    if (argc != 2)
      usage();

    signal(SIGPIPE, SIG_IGN);

    std::shared_ptr<FileDescriptor> switchboard_socket;
    if (Status status = CreateSwitchboardChannel(
            argv[0],
            {configuration.listen() ? Right::SERVER_START
                                    : Right::CLIENT_CONNECT},
            argv[1], &switchboard_socket);
        !status.ok()) {
      std::cerr << status.error_message() << std::endl;
      return 1;
    }
    configuration.set_switchboard_socket(std::move(switchboard_socket));

    configuration.set_stream_input(
        std::make_unique<FileDescriptor>(STDIN_FILENO));
    configuration.set_stream_output(
        std::make_unique<FileDescriptor>(STDOUT_FILENO));
    configuration.set_logger_output(
        std::make_unique<FileDescriptor>(STDERR_FILENO));

    configuration.set_buffer_size(4096);
  }
  Start(configuration);
}
