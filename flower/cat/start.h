// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_CAT_START_H
#define FLOWER_CAT_START_H

namespace flower {
namespace cat {

class Configuration;

[[noreturn]] void Start(const Configuration& configuration);

}  // namespace cat
}  // namespace flower

#endif
