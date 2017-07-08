// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <memory>
#include <thread>

#include <arpc++/arpc++.h>

#include <flower/switchboard/configuration.ad.h>
#include <flower/switchboard/directory.h>
#include <flower/switchboard/handle.h>
#include <flower/switchboard/start.h>
#include <flower/switchboard/target_picker.h>
#include <flower/util/socket.h>

using arpc::FileDescriptor;
using arpc::Server;
using arpc::ServerBuilder;
using arpc::Status;
using flower::switchboard::Directory;
using flower::switchboard::Handle;
using flower::switchboard::TargetPicker;
using flower::util::AcceptSocketConnection;

void flower::switchboard::Start(const Configuration& configuration) {
  // TODO(ed): Check that all required fields are present!

  // Start processing incoming requests.
  Directory directory;
  TargetPicker target_picker;
  for (;;) {
    std::unique_ptr<FileDescriptor> connection;
    if (Status status = AcceptSocketConnection(
            *configuration.listening_socket(), &connection, nullptr, nullptr);
        !status.ok()) {
      // TODO(ed): Report error.
      return;
    }
    // TODO(ed): Deal with thread creation errors!
    std::thread([
      connection{std::move(connection)}, &directory, &target_picker
    ]() mutable {
      ServerBuilder builder(std::move(connection));
      Handle handle(&directory, &target_picker);
      builder.RegisterService(&handle);
      std::shared_ptr<Server> server = builder.Build();
      while (server->HandleRequest() == 0) {
      }
      // TODO(ed): Log error.
    }).detach();
  }
}
