// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_SWITCHBOARD_LABEL_MAP_H
#define FLOWER_SWITCHBOARD_LABEL_MAP_H

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace flower {
namespace switchboard {

// TODO(ed): Would it make sense not to use maps, but vectors of sorted
// pairs internally? We never perform lookups, but only apply scanning
// operations (e.g., merging) on them.
typedef std::map<std::string, std::string, std::less<>> LabelMap;
typedef std::vector<std::string> LabelVector;

void MergeLabelMaps(const LabelMap& a, const LabelMap& b, LabelMap* merged,
                    LabelVector* conflicts);

}  // namespace switchboard
}  // namespace flower

#endif
