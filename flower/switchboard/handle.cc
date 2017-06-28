#include <algorithm>
#include <set>
#include <sstream>
#include <vector>

#include <flower/proto/switchboard.h>
#include <flower/switchboard/handle.h>

using flower::proto::switchboard::ClientConnectRequest;
using flower::proto::switchboard::ClientConnectResponse;
using flower::proto::switchboard::ConstrainRequest;
using flower::proto::switchboard::ConstrainResponse;
using flower::proto::switchboard::EgressStartRequest;
using flower::proto::switchboard::EgressStartResponse;
using flower::proto::switchboard::IngressConnectRequest;
using flower::proto::switchboard::IngressConnectResponse;
using flower::proto::switchboard::ResolverStartRequest;
using flower::proto::switchboard::ResolverStartResponse;
using flower::proto::switchboard::Right;
using flower::proto::switchboard::Right_Name;
using flower::proto::switchboard::ServerStartRequest;
using flower::proto::switchboard::ServerStartResponse;
using flower::switchboard::Handle;

arpc::Status Handle::Constrain(arpc::ServerContext* context,
                               const ConstrainRequest* request,
                               ConstrainResponse* response) {
  std::set<Right> new_rights(request->rights().begin(),
                             request->rights().end());
  if (arpc::Status status = CheckRights_(new_rights); !status.ok())
    return status;

  return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "TODO(ed): Implement!");
}

arpc::Status Handle::ClientConnect(arpc::ServerContext* context,
                                   const ClientConnectRequest* request,
                                   ClientConnectResponse* response) {
  if (arpc::Status status = CheckRights_({Right::CLIENT_CONNECT}); !status.ok())
    return status;

  return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "TODO(ed): Implement!");
}

arpc::Status Handle::EgressStart(arpc::ServerContext* context,
                                 const EgressStartRequest* request,
                                 EgressStartResponse* response) {
  if (arpc::Status status = CheckRights_({Right::EGRESS_START}); !status.ok())
    return status;

  return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "TODO(ed): Implement!");
}

arpc::Status Handle::IngressConnect(arpc::ServerContext* context,
                                    const IngressConnectRequest* request,
                                    IngressConnectResponse* response) {
  if (arpc::Status status = CheckRights_({Right::INGRESS_CONNECT});
      !status.ok())
    return status;

  return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "TODO(ed): Implement!");
}

arpc::Status Handle::ResolverStart(arpc::ServerContext* context,
                                   const ResolverStartRequest* request,
                                   ResolverStartResponse* response) {
  if (arpc::Status status = CheckRights_({Right::RESOLVER_START}); !status.ok())
    return status;

  return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "TODO(ed): Implement!");
}

arpc::Status Handle::ServerStart(arpc::ServerContext* context,
                                 const ServerStartRequest* request,
                                 ServerStartResponse* response) {
  if (arpc::Status status = CheckRights_({Right::SERVER_START}); !status.ok())
    return status;

  return arpc::Status(arpc::StatusCode::UNIMPLEMENTED, "TODO(ed): Implement!");
}

arpc::Status Handle::CheckRights_(const std::set<Right>& requested_rights) {
  std::vector<Right> missing_rights;
  std::set_difference(requested_rights.begin(), requested_rights.end(),
                      rights_.begin(), rights_.end(),
                      std::back_inserter(missing_rights));
  if (!missing_rights.empty()) {
    std::ostringstream ss;
    ss << "Rights { ";
    std::transform(missing_rights.begin(), missing_rights.end(),
                   std::ostream_iterator<const char*>(ss, ", "), Right_Name);
    ss << " } are not present on this handle";
    return arpc::Status(arpc::StatusCode::PERMISSION_DENIED, ss.str());
  }
  return arpc::Status::OK;
}
