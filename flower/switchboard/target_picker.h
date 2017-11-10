// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FLOWER_SWITCHBOARD_TARGET_PICKER_H
#define FLOWER_SWITCHBOARD_TARGET_PICKER_H

#include <functional>
#include <memory>

#include <arpc++/arpc++.h>

#include <flower/util/label_map.h>

namespace flower {
namespace switchboard {

class Directory;
class Listener;

class TargetPicker {
 public:
  arpc::Status Pick(
      Directory* directory, const util::LabelMap& out_labels,
      std::function<arpc::Status(const util::LabelMap& connection_labels,
                                 const std::shared_ptr<Listener>& listener)>
          selector);
};

}  // namespace switchboard
}  // namespace flower

#endif
