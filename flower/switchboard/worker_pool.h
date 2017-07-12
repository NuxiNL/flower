// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_SWITCHBOARD_WORKER_POOL_H
#define FLOWER_SWITCHBOARD_WORKER_POOL_H

#include <atomic>
#include <memory>

#include <arpc++/arpc++.h>

#include <flower/util/logger.h>

namespace flower {
namespace switchboard {

class WorkerPool {
 public:
  WorkerPool(unsigned int workers, util::Logger* logger)
      : workers_remaining_(workers), logger_(logger) {
  }

  arpc::Status StartWorker(
      const std::shared_ptr<arpc::FileDescriptor>& connection,
      std::unique_ptr<arpc::Service> service);

 private:
  std::atomic_uint workers_remaining_;
  util::Logger* const logger_;
};

}  // namespace switchboard
}  // namespace flower

#endif
