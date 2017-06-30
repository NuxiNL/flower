// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <iterator>

#include <flower/switchboard/label_map.h>
#include <flower/util/map_union_difference.h>

using flower::util::map_union_difference;

void flower::switchboard::MergeLabelMaps(const LabelMap& a, const LabelMap& b,
                                         LabelMap* merged,
                                         LabelVector* conflicts) {
  merged->clear();
  conflicts->clear();
  map_union_difference(a.begin(), a.end(), b.begin(), b.end(),
                       std::inserter(*merged, merged->end()),
                       std::back_inserter(*conflicts));
}
