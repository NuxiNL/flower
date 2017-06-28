#include <iterator>

#include <flower/switchboard/label_map.h>
#include <flower/util/map_union_difference.h>

using flower::util::map_union_difference;

void flower::switchboard::MergeLabelMaps(const LabelMap& a, const LabelMap& b,
                                         LabelMap* merged,
                                         LabelSet* conflicts) {
  map_union_difference(a.begin(), a.end(), b.begin(), b.end(),
                       std::inserter(*merged, merged->end()),
                       std::inserter(*conflicts, conflicts->end()));
}
