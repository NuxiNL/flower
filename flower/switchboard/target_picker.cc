// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <functional>
#include <memory>
#include <variant>

#include <flower/switchboard/directory.h>
#include <flower/switchboard/target_picker.h>
#include <flower/util/label_map.h>

using arpc::Status;
using flower::switchboard::TargetPicker;
using flower::util::LabelMap;

Status TargetPicker::Pick(
    Directory* directory, const LabelMap& out_labels,
    std::function<Status(const LabelMap& connection_labels,
                         const std::shared_ptr<Listener>& listener)>
        selector) {
  // TODO(ed): Add support for resolvers.
  LabelMap connection_labels;
  Directory::Target target;
  if (Status status =
          directory->Lookup(out_labels, &connection_labels, &target);
      !status.ok())
    return status;
  return selector(connection_labels,
                  std::get<std::shared_ptr<Listener>>(target));
}
