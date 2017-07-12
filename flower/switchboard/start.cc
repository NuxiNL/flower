// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <memory>
#include <ostream>
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
using flower::util::AcceptSocketConnection;
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
  std::ostream logger_ostream(logger_streambuf.get());
  Logger logger(&logger_ostream);

  // Check that all required fields are present.
  const auto& listening_socket = configuration.listening_socket();
  if (!listening_socket) {
    logger.Log() << "Cannot start without a listening socket";
    return;
  }

  Directory directory;
  TargetPicker target_picker;
  WorkerPool worker_pool;
  for (;;) {
    // Accept incoming connection to switchboard.
    std::unique_ptr<FileDescriptor> connection;
    if (Status status = AcceptSocketConnection(*listening_socket, &connection,
                                               nullptr, nullptr);
        !status.ok()) {
      logger.Log() << "Failed to accept incoming connection: "
                   << status.error_message();
      return;
    }

    if (Status status = worker_pool.StartWorker(
            std::move(connection),
            std::make_unique<Handle>(&directory, &target_picker, &worker_pool));
        !status.ok())
      logger.Log() << "Failed to start worker: " << status.error_message();
  }
}
