// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_SWITCHBOARD_DIRECTORY_H
#define FLOWER_SWITCHBOARD_DIRECTORY_H

#include <memory>
#include <shared_mutex>
#include <variant>

#include <arpc++/arpc++.h>

#include <flower/switchboard/label_map.h>
#include <flower/switchboard/listener.h>

namespace flower {
namespace switchboard {

class Directory {
 public:
  // TODO(ed): Let this be a variant between listeners and resolvers.
  typedef std::variant<std::shared_ptr<Listener>> Target;

  arpc::Status RegisterTarget(const LabelMap& in_labels, const Target& target);
  arpc::Status LookupListener(const LabelMap& out_labels,
                              LabelMap* connection_labels,
                              std::shared_ptr<Listener>* listener);

 private:
  arpc::Status LookupTarget(const LabelMap& out_labels,
                            LabelMap* connection_labels, Target* target);

  std::shared_mutex lock_;
  std::vector<std::pair<LabelMap, Target>> targets_;
};

}  // namespace switchboard
}  // namespace flower

#endif
