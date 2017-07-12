// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_SWITCHBOARD_WORKER_POOL_H
#define FLOWER_SWITCHBOARD_WORKER_POOL_H

#include <memory>

#include <arpc++/arpc++.h>

namespace flower {
namespace switchboard {

class WorkerPool {
 public:
  arpc::Status StartWorker(
      const std::shared_ptr<arpc::FileDescriptor>& connection,
      std::unique_ptr<arpc::Service> service);
};

}  // namespace switchboard
}  // namespace flower

#endif
