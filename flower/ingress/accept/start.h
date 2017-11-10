// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

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
