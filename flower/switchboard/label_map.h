#ifndef FLOWER_SWITCHBOARD_LABEL_MAP_H
#define FLOWER_SWITCHBOARD_LABEL_MAP_H

#include <functional>
#include <map>
#include <string>

namespace flower {
namespace switchboard {

typedef std::map<std::string, std::string, std::less<>> LabelMap;

}  // namespace switchboard
}  // namespace flower

#endif
