// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <cstring>
#include <memory>
#include <thread>

#include <arpc++/arpc++.h>

#include <flower/switchboard/worker_pool.h>

using arpc::FileDescriptor;
using arpc::Server;
using arpc::ServerBuilder;
using arpc::Service;
using arpc::Status;
using arpc::StatusCode;
using flower::switchboard::WorkerPool;

Status WorkerPool::StartWorker(
    const std::shared_ptr<FileDescriptor>& connection,
    std::unique_ptr<Service> service) {
  // See if we're allowed to spawn another worker.
  unsigned int workers_remaining = 1;
  do {
    if (workers_remaining == 0)
      return Status(StatusCode::RESOURCE_EXHAUSTED, "Worker pool exhausted");
  } while (!workers_remaining_.compare_exchange_weak(
      workers_remaining, workers_remaining - 1, std::memory_order_relaxed,
      std::memory_order_relaxed));

  try {
    // Attempt to create a new thread that processes incoming RPCs.
    // TODO(ed): Detaching threads makes unit tests unsafe, as the
    // worker pool and logger can no longer be destroyed. Should the
    // destructor of this class block?
    std::thread([ this, connection, service{std::move(service)} ]() {
      ServerBuilder builder(connection);
      builder.RegisterService(service.get());
      std::shared_ptr<Server> server = builder.Build();
      int error;
      while ((error = server->HandleRequest()) == 0) {
      }
      if (error > 0)
        logger_->Log() << "Failed to process incoming request: "
                       << std::strerror(error);

      workers_remaining_.fetch_add(1, std::memory_order_relaxed);
    }).detach();
  } catch (const std::system_error& e) {
    // Creation failed.
    workers_remaining_.fetch_add(1, std::memory_order_relaxed);
    return Status(StatusCode::INTERNAL, "Failed to create worker thread");
  }
  return Status::OK;
}
