#ifndef FLOWER_SWITCHBOARD_EGRESS_LISTENER_H
#define FLOWER_SWITCHBOARD_EGRESS_LISTENER_H

#include <memory>

#include <arpc++/arpc++.h>

#include <flower/switchboard/label_map.h>
#include <flower/switchboard/listener.h>

namespace flower {
namespace switchboard {

class EgressListener final : public Listener {
 public:
  arpc::Status ConnectWithSocket(
      const LabelMap& connection_labels,
      const std::shared_ptr<arpc::FileDescriptor>& fd) override;
  arpc::Status ConnectWithoutSocket(
      const LabelMap& connection_labels,
      std::shared_ptr<arpc::FileDescriptor>* fd) override;
};

}  // namespace switchboard
}  // namespace flower

#endif
