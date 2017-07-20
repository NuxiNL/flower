// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <algorithm>
#include <memory>
#include <set>
#include <sstream>
#include <string_view>
#include <thread>
#include <vector>

#include <arpc++/arpc++.h>

#include <flower/protocol/switchboard.ad.h>
#include <flower/switchboard/directory.h>
#include <flower/switchboard/egress_listener.h>
#include <flower/switchboard/handle.h>
#include <flower/switchboard/listener.h>
#include <flower/switchboard/server_listener.h>
#include <flower/switchboard/target_picker.h>
#include <flower/switchboard/worker_pool.h>
#include <flower/util/label_map.h>
#include <flower/util/socket.h>

using arpc::FileDescriptor;
using arpc::ServerContext;
using arpc::Status;
using arpc::StatusCode;
using flower::protocol::switchboard::ClientConnectRequest;
using flower::protocol::switchboard::ClientConnectResponse;
using flower::protocol::switchboard::ConstrainRequest;
using flower::protocol::switchboard::ConstrainResponse;
using flower::protocol::switchboard::EgressStartRequest;
using flower::protocol::switchboard::EgressStartResponse;
using flower::protocol::switchboard::IngressConnectRequest;
using flower::protocol::switchboard::IngressConnectResponse;
using flower::protocol::switchboard::ResolverStartRequest;
using flower::protocol::switchboard::ResolverStartResponse;
using flower::protocol::switchboard::Right;
using flower::protocol::switchboard::Right_Name;
using flower::protocol::switchboard::ServerStartRequest;
using flower::protocol::switchboard::ServerStartResponse;
using flower::switchboard::Handle;
using flower::util::CreateSocketpair;
using flower::util::LabelMap;
using flower::util::LabelVector;
using flower::util::LabelVectorToJSON;
using flower::util::MergeLabelMaps;

Status Handle::Constrain(ServerContext* context,
                         const ConstrainRequest* request,
                         ConstrainResponse* response) {
  // Don't allow extending rights.
  std::set<Right> rights(request->rights().begin(), request->rights().end());
  if (Status status = CheckRights_(rights); !status.ok())
    return status;
  LabelMap in_labels;
  if (Status status = GetInLabels_(request->in_labels(), &in_labels);
      !status.ok())
    return status;
  LabelMap out_labels;
  if (Status status = GetOutLabels_(request->out_labels(), &out_labels);
      !status.ok())
    return status;

  // Create a new connection to the switchboard and spawn a worker thread.
  std::unique_ptr<FileDescriptor> fd1, fd2;
  if (Status status = CreateSocketpair(&fd1, &fd2); !status.ok())
    return status;
  response->set_switchboard(std::move(fd1));
  return worker_pool_->StartWorker(
      std::move(fd2), std::unique_ptr<Handle>(
                          new Handle(directory_, target_picker_, worker_pool_,
                                     rights, in_labels, out_labels)));
}

Status Handle::ClientConnect(ServerContext* context,
                             const ClientConnectRequest* request,
                             ClientConnectResponse* response) {
  if (Status status = CheckRights_({Right::CLIENT_CONNECT}); !status.ok())
    return status;
  LabelMap out_labels;
  if (Status status = GetOutLabels_(request->out_labels(), &out_labels);
      !status.ok())
    return status;

  return target_picker_->Pick(
      directory_, out_labels,
      [response](const LabelMap& connection_labels,
                 const std::shared_ptr<Listener>& listener) {
        std::shared_ptr<FileDescriptor> server;
        if (Status status =
                listener->ConnectWithoutSocket(connection_labels, &server);
            !status.ok())
          return status;
        response->set_server(std::move(server));
        *response->mutable_connection_labels() = connection_labels;
        return Status::OK;
      });
}

Status Handle::EgressStart(ServerContext* context,
                           const EgressStartRequest* request,
                           EgressStartResponse* response) {
  if (Status status = CheckRights_({Right::EGRESS_START}); !status.ok())
    return status;
  LabelMap in_labels;
  if (Status status = GetInLabels_(request->in_labels(), &in_labels);
      !status.ok())
    return status;

  std::unique_ptr<FileDescriptor> fd1, fd2;
  if (Status status = CreateSocketpair(&fd1, &fd2); !status.ok())
    return status;
  if (Status status = directory_->Register(
          in_labels, std::make_unique<EgressListener>(std::move(fd1)));
      !status.ok())
    return status;
  response->set_egress(std::move(fd2));
  return Status::OK;
}

Status Handle::IngressConnect(ServerContext* context,
                              const IngressConnectRequest* request,
                              IngressConnectResponse* response) {
  if (Status status = CheckRights_({Right::INGRESS_CONNECT}); !status.ok())
    return status;
  LabelMap out_labels;
  if (Status status = GetOutLabels_(request->out_labels(), &out_labels);
      !status.ok())
    return status;

  const std::shared_ptr<FileDescriptor>& client = request->client();
  if (!client)
    return Status(StatusCode::INVALID_ARGUMENT,
                  "Ingress must provide a file descriptor");
  return target_picker_->Pick(
      directory_, out_labels,
      [&client, response](const LabelMap& connection_labels,
                          const std::shared_ptr<Listener>& listener) {
        if (Status status =
                listener->ConnectWithSocket(connection_labels, client);
            !status.ok())
          return status;
        *response->mutable_connection_labels() = connection_labels;
        return Status::OK;
      });
}

Status Handle::ResolverStart(ServerContext* context,
                             const ResolverStartRequest* request,
                             ResolverStartResponse* response) {
  if (Status status = CheckRights_({Right::RESOLVER_START}); !status.ok())
    return status;
  LabelMap in_labels;
  if (Status status = GetInLabels_(request->in_labels(), &in_labels);
      !status.ok())
    return status;
  LabelMap out_labels;
  if (Status status = GetOutLabels_(request->out_labels(), &out_labels);
      !status.ok())
    return status;

  return Status(StatusCode::UNIMPLEMENTED, "TODO(ed): Implement!");
}

Status Handle::ServerStart(ServerContext* context,
                           const ServerStartRequest* request,
                           ServerStartResponse* response) {
  if (Status status = CheckRights_({Right::SERVER_START}); !status.ok())
    return status;
  LabelMap in_labels;
  if (Status status = GetInLabels_(request->in_labels(), &in_labels);
      !status.ok())
    return status;

  std::unique_ptr<FileDescriptor> fd1, fd2;
  if (Status status = CreateSocketpair(&fd1, &fd2); !status.ok())
    return status;
  if (Status status = directory_->Register(
          in_labels, std::make_unique<ServerListener>(std::move(fd1)));
      !status.ok())
    return status;
  response->set_server(std::move(fd2));
  return Status::OK;
}

Status Handle::CheckRights_(const std::set<Right>& requested_rights) const {
  std::vector<Right> missing_rights;
  std::set_difference(requested_rights.begin(), requested_rights.end(),
                      rights_.begin(), rights_.end(),
                      std::back_inserter(missing_rights));
  if (!missing_rights.empty()) {
    std::ostringstream ss;
    ss << "Rights [";
    bool first = true;
    for (Right right : missing_rights) {
      if (!first)
        ss << ", ";
      first = false;
      ss << '"' << Right_Name(right) << '"';
    }
    ss << "] are not present on this handle";
    return Status(StatusCode::PERMISSION_DENIED, ss.str());
  }
  return Status::OK;
}

Status Handle::GetInLabels_(const LabelMap& additional_labels,
                            LabelMap* merged_labels) const {
  LabelVector conflicts;
  MergeLabelMaps(in_labels_, additional_labels, merged_labels, &conflicts);
  if (!conflicts.empty()) {
    std::ostringstream ss;
    ss << "In-labels ";
    LabelVectorToJSON(conflicts, &ss);
    ss << " are already defined with different values";
    return Status(StatusCode::PERMISSION_DENIED, ss.str());
  }
  return Status::OK;
}

Status Handle::GetOutLabels_(const LabelMap& additional_labels,
                             LabelMap* merged_labels) const {
  LabelVector conflicts;
  MergeLabelMaps(out_labels_, additional_labels, merged_labels, &conflicts);
  if (!conflicts.empty()) {
    std::ostringstream ss;
    ss << "Out-labels ";
    LabelVectorToJSON(conflicts, &ss);
    ss << " are already defined with different values";
    return Status(StatusCode::PERMISSION_DENIED, ss.str());
  }
  return Status::OK;
}
