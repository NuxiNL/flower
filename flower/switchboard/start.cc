// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <cstdlib>
#include <memory>
#include <streambuf>

#include <arpc++/arpc++.h>

#include <flower/switchboard/configuration.ad.h>
#include <flower/switchboard/directory.h>
#include <flower/switchboard/handle.h>
#include <flower/switchboard/start.h>
#include <flower/switchboard/target_picker.h>
#include <flower/switchboard/worker_pool.h>
#include <flower/util/fd_streambuf.h>
#include <flower/util/logger.h>
#include <flower/util/null_streambuf.h>
#include <flower/util/socket.h>

using arpc::FileDescriptor;
using arpc::Status;
using flower::switchboard::Directory;
using flower::switchboard::Handle;
using flower::switchboard::TargetPicker;
#ifndef __CloudABI__
using flower::util::AcceptSocketConnection;
#endif
using flower::util::Logger;
using flower::util::fd_streambuf;
using flower::util::null_streambuf;

void flower::switchboard::Start(const Configuration& configuration) {
  // Make logging work.
  std::unique_ptr<std::streambuf> logger_streambuf;
  if (const auto& logger_output = configuration.logger_output();
      logger_output) {
    logger_streambuf = std::make_unique<fd_streambuf>(logger_output);
  } else {
    logger_streambuf = std::make_unique<null_streambuf>();
  }
  Logger logger(logger_streambuf.get());

  Directory directory;
  TargetPicker target_picker;
  {
    WorkerPool worker_pool(configuration.worker_pool_size(), &logger);

    // If the caller provides a socket on which it wants an initial
    // channel, push it in the worker pool. This is useful for running
    // on top of CloudABI, as we can't call accept() there.
    if (const auto& initial_channel = configuration.initial_channel();
        initial_channel) {
      if (Status status = worker_pool.StartWorker(
              initial_channel, std::make_unique<Handle>(
                                   &directory, &target_picker, &worker_pool));
          !status.ok()) {
        logger.Log() << status.error_message();
        std::exit(1);
      }
    }

#ifndef __CloudABI__
    // If a bound socket has been provided, repeatedly call accept() and
    // push resulting connections into the worker pool. This is useful
    // for running on traditional UNIX-like systems.
    if (const auto& listening_socket = configuration.listening_socket();
        listening_socket) {
      for (;;) {
        std::unique_ptr<FileDescriptor> connection;
        if (Status status = AcceptSocketConnection(
                *listening_socket, &connection, nullptr, nullptr);
            !status.ok()) {
          logger.Log() << status.error_message();
          std::exit(1);
        }

        if (Status status = worker_pool.StartWorker(
                std::move(connection),
                std::make_unique<Handle>(&directory, &target_picker,
                                         &worker_pool));
            !status.ok())
          logger.Log() << status.error_message();
      }
    }
#endif
  }
  std::exit(0);
}
