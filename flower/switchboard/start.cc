// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <cstring>
#include <memory>
#include <ostream>
#include <streambuf>
#include <thread>

#include <arpc++/arpc++.h>

#include <flower/switchboard/configuration.ad.h>
#include <flower/switchboard/directory.h>
#include <flower/switchboard/handle.h>
#include <flower/switchboard/start.h>
#include <flower/switchboard/target_picker.h>
#include <flower/util/fd_streambuf.h>
#include <flower/util/logger.h>
#include <flower/util/null_streambuf.h>
#include <flower/util/socket.h>

using arpc::FileDescriptor;
using arpc::Server;
using arpc::ServerBuilder;
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

    // Process incoming RPCs in a separate thread.
    // TODO(ed): Deal with thread creation errors!
    std::thread([
      connection{std::move(connection)}, &directory, &logger, &target_picker
    ]() mutable {
      ServerBuilder builder(std::move(connection));
      Handle handle(&directory, &target_picker);
      builder.RegisterService(&handle);
      std::shared_ptr<Server> server = builder.Build();
      int error;
      while ((error = server->HandleRequest()) == 0) {
      }
      if (error > 0)
        logger.Log() << "Failed to process incoming request: "
                     << std::strerror(errno);
    }).detach();
  }
}
