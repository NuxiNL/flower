// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FLOWER_SWITCHBOARD_START_H
#define FLOWER_SWITCHBOARD_START_H

namespace flower {
namespace switchboard {

class Configuration;

[[noreturn]] void Start(const Configuration& configuration);

}  // namespace switchboard
}  // namespace flower

#endif
