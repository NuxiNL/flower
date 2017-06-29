#ifndef FLOWER_SWITCHBOARD_LISTENER_H
#define FLOWER_SWITCHBOARD_LISTENER_H

#include <memory>
#include <mutex>

#include <arpc++/arpc++.h>

#include <flower/switchboard/label_map.h>

namespace flower {
namespace switchboard {

class Listener {
 public:
  virtual ~Listener() {
  }

  arpc::Status Start(std::unique_ptr<arpc::FileDescriptor>* fd);

  virtual arpc::Status ConnectWithSocket(
      const LabelMap& connection_labels,
      const std::shared_ptr<arpc::FileDescriptor>& fd) = 0;
  virtual arpc::Status ConnectWithoutSocket(
      const LabelMap& connection_labels,
      std::shared_ptr<arpc::FileDescriptor>* fd) = 0;

 protected:
  std::shared_ptr<arpc::Channel> channel_;
  std::mutex channel_lock_;
};

}  // namespace switchboard
}  // namespace flower

#endif
