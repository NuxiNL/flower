// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <shared_mutex>
#include <thread>

#include <flower/switchboard/directory.h>

using arpc::Status;
using arpc::StatusCode;
using flower::switchboard::Listener;
using flower::switchboard::Directory;
using flower::switchboard::LabelMap;

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
  for (const auto& listener : listeners_) {
    if (std::includes(out_labels.begin(), out_labels.end(),
                      listener.first.begin(), listener.first.end())) {
      *connection_labels = out_labels;
      *result = listener.second;
      return Status::OK;
    }
  }
  return Status(StatusCode::NOT_FOUND, "No matching destination found");
}
