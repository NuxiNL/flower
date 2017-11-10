// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FLOWER_SWITCHBOARD_DIRECTORY_H
#define FLOWER_SWITCHBOARD_DIRECTORY_H

#include <memory>
#include <mutex>
#include <utility>
#include <variant>
#include <vector>

#include <arpc++/arpc++.h>

#include <flower/protocol/switchboard.ad.h>
#include <flower/switchboard/listener.h>
#include <flower/util/label_map.h>

namespace flower {
namespace switchboard {

class Directory {
 public:
  // TODO(ed): Let this be a variant between listeners and resolvers.
  typedef std::variant<std::shared_ptr<Listener>> Target;

  arpc::Status Register(const util::LabelMap& in_labels, const Target& target);
  arpc::Status Lookup(const util::LabelMap& out_labels,
                      util::LabelMap* connection_labels, Target* target);

  void List(const util::LabelMap& out_labels,
            std::vector<protocol::switchboard::ListResponse>* targets);

 private:
  void PruneDeadTargets();

  std::mutex lock_;
  std::vector<std::pair<util::LabelMap, Target>> targets_;
};

}  // namespace switchboard
}  // namespace flower

#endif
