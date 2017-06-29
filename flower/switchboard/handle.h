#ifndef FLOWER_SWITCHBOARD_HANDLE_H
#define FLOWER_SWITCHBOARD_HANDLE_H

#include <set>

#include <arpc++/arpc++.h>

#include <flower/proto/switchboard.h>
#include <flower/switchboard/label_map.h>

namespace flower {
namespace switchboard {

class Directory;
class Listener;

class Handle final : public proto::switchboard::Switchboard::Service {
 public:
  // Constructs a handle to a switchboard directory that provides access
  // without any restrictions.
  Handle(Directory* directory)
      : directory_(directory),
        rights_({
            proto::switchboard::Right::CLIENT_CONNECT,
            proto::switchboard::Right::EGRESS_START,
            proto::switchboard::Right::INGRESS_CONNECT,
            proto::switchboard::Right::RESOLVER_START,
            proto::switchboard::Right::SERVER_START,
        }) {
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
  // Constructs a handle to a switchboard with limited access.
  Handle(Directory* directory,
         const std::set<proto::switchboard::Right>& rights,
         const LabelMap& in_labels, const LabelMap& out_labels)
      : directory_(directory),
        rights_(rights),
        in_labels_(in_labels),
        out_labels_(out_labels) {
  }

  arpc::Status CheckRights_(
      const std::set<proto::switchboard::Right>& requested_rights);
  arpc::Status ListenerStart_(std::unique_ptr<Listener> listener,
                              std::unique_ptr<arpc::FileDescriptor>* fd);

  Directory* const directory_;
  const std::set<proto::switchboard::Right> rights_;
  const LabelMap in_labels_;
  const LabelMap out_labels_;
};

}  // namespace switchboard
}  // namespace flower

#endif
