// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <shared_mutex>
#include <sstream>
#include <thread>

#include <flower/switchboard/directory.h>
#include <flower/util/ostream_infix_iterator.h>

using arpc::Status;
using arpc::StatusCode;
using flower::switchboard::Listener;
using flower::switchboard::Directory;
using flower::switchboard::LabelMap;
using flower::util::ostream_infix_iterator;

Status Directory::RegisterListener(const LabelMap& in_labels,
                                   std::unique_ptr<Listener> listener) {
  // TODO(ed): Reject duplicate registration, overlapping labels, etc.
  std::unique_lock<std::shared_mutex> lock(lock_);
  listeners_.emplace_back(in_labels, std::move(listener));
  return Status::OK;
}

Status Directory::LookupListener(const LabelMap& out_labels,
                                 LabelMap* connection_labels,
                                 std::shared_ptr<Listener>* result) {
  // TODO(ed): Add support for resolvers.
  std::shared_lock<std::shared_mutex> lock(lock_);
  const std::pair<LabelMap, std::shared_ptr<Listener>>* match = nullptr;
  for (const auto& listener : listeners_) {
    connection_labels->clear();
    LabelVector conflicts;
    MergeLabelMaps(out_labels, listener.first, connection_labels, &conflicts);
    if (conflicts.empty()) {
      // Found a matching target. Continue searching, ensuring that this
      // is the only target that matches these constraints. In case of
      // ambiguities, report how they can be resolved.
      if (match != nullptr) {
        LabelMap unused;
        LabelVector disambiguation;
        MergeLabelMaps(match->first, listener.first, &unused, &disambiguation);
        std::ostringstream ss;
        ss << "Labels match multiple targets, "
              "which can be resolved by adding one of the labels { ";
        std::copy(disambiguation.begin(), disambiguation.end(),
                  ostream_infix_iterator<>(ss, ", "));
        ss << " }";
        return Status(StatusCode::FAILED_PRECONDITION, ss.str());
      }
      match = &listener;
    }
  }
  if (match == nullptr)
    return Status(StatusCode::NOT_FOUND, "No matching destination found");
  *result = match->second;
  return Status::OK;
}
