// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_LABEL_MAP_H
#define FLOWER_LABEL_MAP_H

#include <functional>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include <flower/util/map_union_difference.h>

namespace flower {
namespace util {
namespace {

// TODO(ed): Would it make sense not to use maps, but vectors of sorted
// pairs internally? We never perform lookups, but only apply scanning
// operations (e.g., merging) on them.
typedef std::map<std::string, std::string, std::less<>> LabelMap;
typedef std::vector<std::string> LabelVector;

void MergeLabelMaps(const LabelMap& a, const LabelMap& b, LabelMap* merged,
                    LabelVector* conflicts) {
  merged->clear();
  conflicts->clear();
  flower::util::map_union_difference(a.begin(), a.end(), b.begin(), b.end(),
                                     std::inserter(*merged, merged->end()),
                                     std::back_inserter(*conflicts));
}

template <typename T>
void StringToJSON(const std::string& label, T* ostream) {
  // TODO(ed): Escape values properly.
  // TODO(ed): Ensure we always print proper UTF-8.
  *ostream << '"' << label << '"';
}

template <typename T>
void LabelMapToJSON(const LabelMap& labels, T* ostream) {
  *ostream << "{";
  bool first = true;
  for (const auto& label : labels) {
    if (!first)
      *ostream << ", ";
    first = false;
    StringToJSON(label.first, ostream);
    *ostream << ": ";
    StringToJSON(label.second, ostream);
  }
  *ostream << "}";
}

template <typename T>
void LabelVectorToJSON(const LabelVector& labels, T* ostream) {
  *ostream << "[";
  bool first = true;
  for (const auto& label : labels) {
    if (!first)
      *ostream << ", ";
    first = false;
    StringToJSON(label, ostream);
  }
  *ostream << "]";
}

}  // namespace
}  // namespace util
}  // namespace flower

#endif
