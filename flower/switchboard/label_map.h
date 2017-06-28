#ifndef FLOWER_SWITCHBOARD_LABEL_MAP_H
#define FLOWER_SWITCHBOARD_LABEL_MAP_H

#include <functional>
#include <map>
#include <set>
#include <string>

namespace flower {
namespace switchboard {

typedef std::map<std::string, std::string, std::less<>> LabelMap;
typedef std::set<std::string, std::less<>> LabelSet;

void MergeLabelMaps(const LabelMap& a, const LabelMap& b, LabelMap* merged,
                    LabelSet* conflicts);

}  // namespace switchboard
}  // namespace flower

#endif
