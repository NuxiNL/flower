// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FLOWER_CAT_START_H
#define FLOWER_CAT_START_H

namespace flower {
namespace cat {

class Configuration;

[[noreturn]] void Start(const Configuration& configuration);

}  // namespace cat
}  // namespace flower

#endif
