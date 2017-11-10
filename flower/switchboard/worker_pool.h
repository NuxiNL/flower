// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FLOWER_SWITCHBOARD_WORKER_POOL_H
#define FLOWER_SWITCHBOARD_WORKER_POOL_H

#include <condition_variable>
#include <memory>
#include <mutex>

#include <arpc++/arpc++.h>

#include <flower/util/logger.h>

namespace flower {
namespace switchboard {

class WorkerPool {
 public:
  WorkerPool(unsigned int workers_permitted, util::Logger* logger)
      : workers_permitted_(workers_permitted),
        logger_(logger),
        workers_running_(0) {
  }
  ~WorkerPool();

  arpc::Status StartWorker(
      const std::shared_ptr<arpc::FileDescriptor>& connection,
      std::unique_ptr<arpc::Service> service);

 private:
  void DecrementWorkersRunning();

  const unsigned int workers_permitted_;
  util::Logger* const logger_;

  std::mutex lock_;
  std::condition_variable condvar_;
  unsigned int workers_running_;
};

}  // namespace switchboard
}  // namespace flower

#endif
