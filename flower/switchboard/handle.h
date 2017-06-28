#ifndef FLOWER_SWITCHBOARD_HANDLE_H
#define FLOWER_SWITCHBOARD_HANDLE_H

#include <flower/proto/switchboard.h>

namespace flower {
namespace switchboard {

class Handle final : public proto::switchboard::Switchboard::Service {
 public:
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
};

}  // namespace switchboard
}  // namespace flower

#endif
