// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <cassert>
#include <cstring>
#include <memory>
#include <mutex>
#include <system_error>
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
  {
    std::lock_guard<std::mutex> lock(lock_);
    if (workers_running_ >= workers_permitted_)
      return Status(StatusCode::RESOURCE_EXHAUSTED, "Worker pool exhausted");
    ++workers_running_;
  }

  try {
    // Attempt to create a new thread that processes incoming RPCs.
    std::thread([this, connection, service{std::move(service)}]() {
      ServerBuilder builder(connection);
      builder.RegisterService(service.get());
      std::shared_ptr<Server> server = builder.Build();
      int error;
      while ((error = server->HandleRequest()) == 0) {
      }
      if (error > 0)
        logger_->Log() << "Failed to process incoming request: "
                       << std::strerror(error);
      DecrementWorkersRunning();
    })
        .detach();
  } catch (const std::system_error& e) {
    // Creation failed.
    DecrementWorkersRunning();
    return Status(StatusCode::INTERNAL, "Failed to create worker thread");
  }
  return Status::OK;
}

WorkerPool::~WorkerPool() {
  // Block destruction of this class until all of the workers have
  // terminated. This is necessary for using this class in cases where
  // worker pools need to be created temporarily (e.g., unit tests).
  std::unique_lock<std::mutex> lock(lock_);
  condvar_.wait(lock, [this]() { return workers_running_ == 0; });
}

void WorkerPool::DecrementWorkersRunning() {
  std::lock_guard<std::mutex> lock(lock_);
  assert(workers_running_ > 0 && "Number of workers cannot be decremented");
  if (--workers_running_ == 0)
    condvar_.notify_all();
}
