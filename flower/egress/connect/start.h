// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FLOWER_EGRESS_CONNECT_START_H
#define FLOWER_EGRESS_CONNECT_START_H

namespace flower {
namespace egress {
namespace connect {

class Configuration;

[[noreturn]] void Start(const Configuration& configuration);

}  // namespace connect
}  // namespace egress
}  // namespace flower

#endif
