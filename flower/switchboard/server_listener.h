// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_SWITCHBOARD_SERVER_LISTENER_H
#define FLOWER_SWITCHBOARD_SERVER_LISTENER_H

#include <memory>
#include <mutex>

#include <arpc++/arpc++.h>

#include <flower/switchboard/listener.h>
#include <flower/util/label_map.h>

namespace flower {
namespace switchboard {

class ServerListener final : public Listener {
 public:
  explicit ServerListener(std::unique_ptr<arpc::FileDescriptor> fd)
      : channel_(arpc::CreateChannel(std::move(fd))) {
  }

  arpc::Status ConnectWithSocket(
      const util::LabelMap& connection_labels,
      const std::shared_ptr<arpc::FileDescriptor>& fd) override;
  arpc::Status ConnectWithoutSocket(
      const util::LabelMap& connection_labels,
      std::shared_ptr<arpc::FileDescriptor>* fd) override;

 protected:
  std::shared_ptr<arpc::Channel> channel_;
  std::mutex channel_lock_;
};

}  // namespace switchboard
}  // namespace flower

#endif
