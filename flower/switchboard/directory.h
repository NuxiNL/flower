// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_SWITCHBOARD_DIRECTORY_H
#define FLOWER_SWITCHBOARD_DIRECTORY_H

#include <memory>
#include <shared_mutex>
#include <utility>
#include <variant>
#include <vector>

#include <arpc++/arpc++.h>

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

 private:
  void PruneDeadTargets();

  std::shared_mutex lock_;
  std::vector<std::pair<util::LabelMap, Target>> targets_;
};

}  // namespace switchboard
}  // namespace flower

#endif
