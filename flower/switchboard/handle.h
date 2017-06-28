#ifndef FLOWER_SWITCHBOARD_HANDLE_H
#define FLOWER_SWITCHBOARD_HANDLE_H

#include <set>

#include <arpc++/arpc++.h>

#include <flower/proto/switchboard.h>
#include <flower/switchboard/label_map.h>

namespace flower {
namespace switchboard {

class Directory;

class Handle final : public proto::switchboard::Switchboard::Service {
 public:
  Handle(Directory* directory,
         const std::set<proto::switchboard::Right>& rights,
         const LabelMap& in_labels, const LabelMap& out_labels)
      : directory_(directory),
        rights_(rights),
        in_labels_(in_labels),
        out_labels_(out_labels) {
  }

  arpc::Status Constrain(
      arpc::ServerContext* context,
      const proto::switchboard::ConstrainRequest* request,
      proto::switchboard::ConstrainResponse* response) override;

  arpc::Status ClientConnect(
      arpc::ServerContext* context,
      const proto::switchboard::ClientConnectRequest* request,
      proto::switchboard::ClientConnectResponse* response) override;
  arpc::Status EgressStart(
      arpc::ServerContext* context,
      const proto::switchboard::EgressStartRequest* request,
      proto::switchboard::EgressStartResponse* response) override;
  arpc::Status IngressConnect(
      arpc::ServerContext* context,
      const proto::switchboard::IngressConnectRequest* request,
      proto::switchboard::IngressConnectResponse* response) override;
  arpc::Status ResolverStart(
      arpc::ServerContext* context,
      const proto::switchboard::ResolverStartRequest* request,
      proto::switchboard::ResolverStartResponse* response) override;
  arpc::Status ServerStart(
      arpc::ServerContext* context,
      const proto::switchboard::ServerStartRequest* request,
      proto::switchboard::ServerStartResponse* response) override;

 private:
  arpc::Status CheckRights_(
      const std::set<proto::switchboard::Right>& requested_rights);

  Directory* const directory_;
  const std::set<proto::switchboard::Right> rights_;
  const LabelMap in_labels_;
  const LabelMap out_labels_;
};

}  // namespace switchboard
}  // namespace flower

#endif
