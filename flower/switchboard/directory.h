// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_SWITCHBOARD_DIRECTORY_H
#define FLOWER_SWITCHBOARD_DIRECTORY_H

#include <memory>

#include <arpc++/arpc++.h>

#include <flower/switchboard/label_map.h>

namespace flower {
namespace switchboard {

class Listener;

class Directory {
 public:
  arpc::Status RegisterListener(const LabelMap& in_labels,
                                std::unique_ptr<Listener> listener);
  arpc::Status LookupListener(const LabelMap& out_labels,
                              LabelMap* connection_labels,
                              std::shared_ptr<Listener>* listener);
};

}  // namespace switchboard
}  // namespace flower

#endif
