// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <sys/socket.h>
#include <sys/un.h>

#include <signal.h>

#include <iostream>

#include <arpc++/arpc++.h>

#include <flower/switchboard/configuration.ad.h>
#include <flower/switchboard/start.h>
#include <flower/util/sockaddr.h>
#include <flower/util/socket.h>

using arpc::FileDescriptor;
using arpc::Status;
using flower::switchboard::Configuration;
using flower::switchboard::Start;
using flower::util::CreateSocket;
using flower::util::InitializeSockaddrUn;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: flower_switchboard socket_path" << std::endl;
    return 1;
  }

  // Bind to a UNIX socket.
  std::unique_ptr<FileDescriptor> s;
  if (Status status = CreateSocket(AF_UNIX, &s); !status.ok()) {
    std::cerr << status.error_message() << std::endl;
    return 1;
  }
  union {
    sockaddr sa;
    sockaddr_un sun;
  } path = {};
  if (Status status = InitializeSockaddrUn(argv[1], &path.sun); !status.ok()) {
    std::cerr << status.error_message() << std::endl;
    return 1;
  }
  std::remove(argv[1]);
  if (bind(s->get(), &path.sa, sizeof(path.sun)) != 0) {
    std::cerr << "Failed to bind socket: " << std::strerror(errno) << std::endl;
    return 1;
  }
  if (listen(s->get(), 0) != 0) {
    std::cerr << "Failed to listen: " << std::strerror(errno) << std::endl;
    return 1;
  }

  signal(SIGPIPE, SIG_IGN);

  Configuration configuration;
  configuration.set_listening_socket(std::move(s));
  configuration.set_logger_output(
      std::make_unique<FileDescriptor>(STDERR_FILENO));
  // TODO(ed): Make this configurable.
  configuration.set_worker_pool_size(10);
  Start(configuration);
}
