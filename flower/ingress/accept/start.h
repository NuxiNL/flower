// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_INGRESS_ACCEPT_START_H
#define FLOWER_INGRESS_ACCEPT_START_H

namespace flower {
namespace ingress {
namespace accept {

class Configuration;

[[noreturn]] void Start(const Configuration& configuration);

}  // namespace accept
}  // namespace ingress
}  // namespace flower

#endif
