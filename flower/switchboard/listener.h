#ifndef FLOWER_SWITCHBOARD_LISTENER_H
#define FLOWER_SWITCHBOARD_LISTENER_H

#include <memory>

#include <arpc++/arpc++.h>

#include <flower/switchboard/label_map.h>

namespace flower {
namespace switchboard {

class Listener {
 public:
  virtual ~Listener() {
  }

  virtual arpc::Status Start(std::unique_ptr<arpc::FileDescriptor>* fd) = 0;

  virtual arpc::Status ConnectWithSocket(
      const LabelMap& resolved_labels,
      const std::shared_ptr<arpc::FileDescriptor>& fd) = 0;
  virtual arpc::Status ConnectWithoutSocket(
      const LabelMap& resolved_labels,
      std::shared_ptr<arpc::FileDescriptor>* fd) = 0;
};

}  // namespace switchboard
}  // namespace flower

#endif
