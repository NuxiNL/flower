// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_LS_START_H
#define FLOWER_LS_START_H

namespace flower {
namespace ls {

class Configuration;

[[noreturn]] void Start(const Configuration& configuration);

}  // namespace ls
}  // namespace flower

#endif
