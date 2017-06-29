// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_SWITCHBOARD_HANDLE_H
#define FLOWER_SWITCHBOARD_HANDLE_H

#include <set>

#include <arpc++/arpc++.h>

#include <flower/protocol/switchboard.ad.h>
#include <flower/switchboard/label_map.h>

namespace flower {
namespace switchboard {

class Directory;
class Listener;

class Handle final : public protocol::switchboard::Switchboard::Service {
 public:
  // Constructs a handle to a switchboard directory that provides access
  // without any restrictions.
  explicit Handle(Directory* directory)
      : directory_(directory),
        rights_({
            protocol::switchboard::Right::CLIENT_CONNECT,
            protocol::switchboard::Right::EGRESS_START,
            protocol::switchboard::Right::INGRESS_CONNECT,
            protocol::switchboard::Right::RESOLVER_START,
            protocol::switchboard::Right::SERVER_START,
        }) {
  }

  arpc::Status Constrain(
      arpc::ServerContext* context,
      const protocol::switchboard::ConstrainRequest* request,
      protocol::switchboard::ConstrainResponse* response) override;

  arpc::Status ClientConnect(
      arpc::ServerContext* context,
      const protocol::switchboard::ClientConnectRequest* request,
      protocol::switchboard::ClientConnectResponse* response) override;
  arpc::Status EgressStart(
      arpc::ServerContext* context,
      const protocol::switchboard::EgressStartRequest* request,
      protocol::switchboard::EgressStartResponse* response) override;
  arpc::Status IngressConnect(
      arpc::ServerContext* context,
      const protocol::switchboard::IngressConnectRequest* request,
      protocol::switchboard::IngressConnectResponse* response) override;
  arpc::Status ResolverStart(
      arpc::ServerContext* context,
      const protocol::switchboard::ResolverStartRequest* request,
      protocol::switchboard::ResolverStartResponse* response) override;
  arpc::Status ServerStart(
      arpc::ServerContext* context,
      const protocol::switchboard::ServerStartRequest* request,
      protocol::switchboard::ServerStartResponse* response) override;

 private:
  // Constructs a handle to a switchboard with limited access.
  Handle(Directory* directory,
         const std::set<protocol::switchboard::Right>& rights,
         const LabelMap& in_labels, const LabelMap& out_labels)
      : directory_(directory),
        rights_(rights),
        in_labels_(in_labels),
        out_labels_(out_labels) {
  }

  arpc::Status CheckRights_(
      const std::set<protocol::switchboard::Right>& requested_rights) const;
  arpc::Status GetInLabels_(const LabelMap& additional_labels,
                            LabelMap* merged_labels) const;
  arpc::Status GetOutLabels_(const LabelMap& additional_labels,
                             LabelMap* merged_labels) const;

  arpc::Status ListenerStart_(const LabelMap& in_labels,
                              std::unique_ptr<Listener> listener,
                              std::unique_ptr<arpc::FileDescriptor>* fd) const;

  Directory* const directory_;
  const std::set<protocol::switchboard::Right> rights_;
  const LabelMap in_labels_;
  const LabelMap out_labels_;
};

}  // namespace switchboard
}  // namespace flower

#endif
