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

#include <json/value.h>
#include <json/writer.h>

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

void LabelMapToJSON(const LabelMap& labels, std::ostream* ostream) {
  Json::Value root(Json::objectValue);
  for (const auto& label : labels)
    root[label.first] = label.second;
  Json::StreamWriterBuilder builder;
  builder["indentation"] = "";
  builder.newStreamWriter()->write(root, ostream);
}

void LabelVectorToJSON(const LabelVector& labels, std::ostream* ostream) {
  Json::Value root(Json::arrayValue);
  for (const auto& label : labels)
    root.append(label);
  Json::StreamWriterBuilder builder;
  builder["indentation"] = "";
  builder.newStreamWriter()->write(root, ostream);
}

}  // namespace
}  // namespace util
}  // namespace flower

#endif
