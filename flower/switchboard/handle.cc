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
#include <flower/switchboard/label_map.h>
#include <flower/switchboard/listener.h>
#include <flower/switchboard/server_listener.h>
#include <flower/util/ostream_infix_iterator.h>
#include <flower/util/socket.h>

using arpc::FileDescriptor;
using arpc::Server;
using arpc::ServerBuilder;
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
using flower::util::ostream_infix_iterator;

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
  auto handle = std::unique_ptr<Handle>(
      new Handle(directory_, rights, in_labels, out_labels));
  // TODO(ed): Deal with thread creation errors!
  // TODO(ed): Place limits on the number of channels?
  // TODO(ed): Switch to something event driven?
  std::thread([
    connection{std::move(fd1)}, handle{std::move(handle)}
  ]() mutable {
    ServerBuilder builder(std::move(connection));
    builder.RegisterService(handle.get());
    std::shared_ptr<Server> server = builder.Build();
    while (server->HandleRequest() == 0) {
    }
    // TODO(ed): Log error.
  }).detach();
  response->set_switchboard(std::move(fd2));
  return Status::OK;
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

  auto connection_labels = response->mutable_connection_labels();
  std::shared_ptr<Listener> listener;
  if (Status status =
          directory_->LookupListener(out_labels, connection_labels, &listener);
      !status.ok())
    return status;
  std::shared_ptr<FileDescriptor> fd;
  if (Status status = listener->ConnectWithoutSocket(*connection_labels, &fd);
      !status.ok())
    return status;
  response->set_server(std::move(fd));
  return Status::OK;
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

  std::unique_ptr<FileDescriptor> fd;
  if (Status status =
          ListenerStart_(in_labels, std::make_unique<EgressListener>(), &fd);
      !status.ok())
    return status;
  response->set_egress(std::move(fd));
  return Status::OK;
}

Status Handle::IngressConnect(ServerContext* context,
                              const IngressConnectRequest* request,
                              IngressConnectResponse* response) {
  if (!request->client())
    return Status(StatusCode::INVALID_ARGUMENT,
                  "Ingress must provide a file descriptor");
  if (Status status = CheckRights_({Right::INGRESS_CONNECT}); !status.ok())
    return status;
  LabelMap out_labels;
  if (Status status = GetOutLabels_(request->out_labels(), &out_labels);
      !status.ok())
    return status;

  LabelMap connection_labels;
  std::shared_ptr<Listener> listener;
  if (Status status =
          directory_->LookupListener(out_labels, &connection_labels, &listener);
      !status.ok())
    return status;
  if (Status status =
          listener->ConnectWithSocket(connection_labels, request->client());
      !status.ok())
    return status;
  return Status::OK;
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

  std::unique_ptr<FileDescriptor> fd;
  if (Status status =
          ListenerStart_(in_labels, std::make_unique<ServerListener>(), &fd);
      !status.ok())
    return status;
  response->set_server(std::move(fd));
  return Status::OK;
}

Status Handle::CheckRights_(const std::set<Right>& requested_rights) const {
  std::vector<Right> missing_rights;
  std::set_difference(requested_rights.begin(), requested_rights.end(),
                      rights_.begin(), rights_.end(),
                      std::back_inserter(missing_rights));
  if (!missing_rights.empty()) {
    std::ostringstream ss;
    ss << "Rights { ";
    std::transform(missing_rights.begin(), missing_rights.end(),
                   ostream_infix_iterator<>(ss, ", "), Right_Name);
    ss << " } are not present on this handle";
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
    ss << "In-labels { ";
    std::copy(conflicts.begin(), conflicts.end(),
              ostream_infix_iterator<>(ss, ", "));
    ss << " } are already defined with different values";
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
    ss << "Out-labels { ";
    std::copy(conflicts.begin(), conflicts.end(),
              ostream_infix_iterator<>(ss, ", "));
    ss << " } are already defined with different values";
    return Status(StatusCode::PERMISSION_DENIED, ss.str());
  }
  return Status::OK;
}

Status Handle::ListenerStart_(const LabelMap& in_labels,
                              std::unique_ptr<Listener> listener,
                              std::unique_ptr<FileDescriptor>* fd) const {
  if (Status status = listener->Start(fd); !status.ok())
    return status;
  if (Status status =
          directory_->RegisterListener(in_labels, std::move(listener));
      !status.ok())
    return status;
  return Status::OK;
}
