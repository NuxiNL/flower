// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <memory>
#include <shared_mutex>
#include <sstream>
#include <thread>
#include <utility>

#include <flower/switchboard/directory.h>
#include <flower/util/label_map.h>

using arpc::Status;
using arpc::StatusCode;
using flower::switchboard::Directory;
using flower::switchboard::Listener;
using flower::util::LabelMap;
using flower::util::LabelVector;
using flower::util::LabelVectorToJson;
using flower::util::MergeLabelMaps;

Status Directory::Register(const LabelMap& in_labels, const Target& target) {
  // Scan through all the targets, ensuring that we don't introduce new
  // targets that make lookups ambiguous (e.g., identical, subsets or
  // supersets of each other).
  // TODO(ed): Could the error message include more details without
  // leaking private information?
  std::unique_lock<std::shared_mutex> lock(lock_);
  PruneDeadTargets();

  for (const auto& target : targets_) {
    LabelMap unused;
    LabelVector conflicts;
    MergeLabelMaps(in_labels, target.first, &unused, &conflicts);
    if (conflicts.empty())
      return Status(StatusCode::FAILED_PRECONDITION,
                    "Another target with no conflicting labels exists");
  }
  targets_.emplace_back(in_labels, std::move(target));
  return Status::OK;
}

Status Directory::Lookup(const LabelMap& out_labels,
                         LabelMap* connection_labels, Target* result) {
  std::shared_lock<std::shared_mutex> lock(lock_);
  PruneDeadTargets();

  const std::pair<LabelMap, Target>* match = nullptr;
  for (const auto& target : targets_) {
    LabelMap merged;
    LabelVector conflicts;
    MergeLabelMaps(out_labels, target.first, &merged, &conflicts);
    if (conflicts.empty()) {
      // Found a matching target. Continue searching, ensuring that this
      // is the only target that matches these constraints. In case of
      // ambiguities, report how they can be resolved.
      if (match != nullptr) {
        LabelMap unused;
        LabelVector disambiguation;
        MergeLabelMaps(match->first, target.first, &unused, &disambiguation);
        std::ostringstream ss;
        ss << "Labels match multiple targets, "
              "which can be resolved by adding one of the labels ";
        LabelVectorToJson(disambiguation, &ss);
        return Status(StatusCode::FAILED_PRECONDITION, ss.str());
      }
      match = &target;
      *connection_labels = merged;
    }
  }
  if (match == nullptr)
    return Status(StatusCode::NOT_FOUND, "No matching target found");
  *result = match->second;
  return Status::OK;
}

void Directory::PruneDeadTargets() {
  // TODO(ed): This invokes O(n) calls to poll(). Maybe we could extract
  // all of the listeners' file descriptors and combine them into a
  // single call to poll()?
  targets_.erase(
      std::remove_if(
          targets_.begin(), targets_.end(),
          [](const std::pair<util::LabelMap, Target>& target) {
            return std::get<std::shared_ptr<Listener>>(target.second)->IsDead();
          }),
      targets_.end());
}
