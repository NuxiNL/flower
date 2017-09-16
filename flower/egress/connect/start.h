// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

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
