// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <memory>
#include <thread>

#include <arpc++/arpc++.h>

#include <flower/switchboard/worker_pool.h>

using arpc::FileDescriptor;
using arpc::Server;
using arpc::ServerBuilder;
using arpc::Service;
using arpc::Status;
using flower::switchboard::WorkerPool;

Status WorkerPool::StartWorker(
    const std::shared_ptr<FileDescriptor>& connection,
    std::unique_ptr<Service> service) {
  // TODO(ed): Deal with thread creation errors!
  // TODO(ed): Place limits on the number of channels?
  // TODO(ed): Switch to something event driven?
  std::thread([ connection, service{std::move(service)} ]() {
    ServerBuilder builder(connection);
    builder.RegisterService(service.get());
    std::shared_ptr<Server> server = builder.Build();
    while (server->HandleRequest() == 0) {
    }
    // TODO(ed): Log error.
  }).detach();
  return Status::OK;
}
