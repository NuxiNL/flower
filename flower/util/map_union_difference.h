#ifndef FLOWER_UTIL_MAP_UNION_DIFFERENCE_H
#define FLOWER_UTIL_MAP_UNION_DIFFERENCE_H

#include <utility>

namespace flower {
namespace util {
namespace {

// Merges two maps together, only retaining duplicate elements in case
// the values match. Keys of mismatches are stored as well.
template <class InputIt1, class InputIt2, class OutputItUnion,
          class OutputItDifference>
std::pair<OutputItUnion, OutputItDifference> map_union_difference(
    InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2,
    OutputItUnion union_first, OutputItDifference difference_first) {
  while (first1 != last1) {
    if (first2 == last2)
      return std::make_pair(std::copy(first1, last1, union_first),
                            difference_first);
    if (first1->first < first2->first) {
      *union_first++ = *first1++;
    } else if (first2->first < first1->first) {
      *union_first++ = *first2++;
    } else if (first1->second == first2->second) {
      *union_first++ = *first1++;
      ++first2;
    } else {
      *difference_first++ = first1->first;
      ++first1;
      ++first2;
    }
  }
  return std::make_pair(std::copy(first2, last2, union_first),
                        difference_first);
}

}  // namespace
}  // namespace util
}  // namespace flower

#endif
