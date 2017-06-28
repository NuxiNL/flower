#ifndef FLOWER_SWITCHBOARD_DIRECTORY_H
#define FLOWER_SWITCHBOARD_DIRECTORY_H

#include <memory>

#include <arpc++/arpc++.h>

#include <flower/switchboard/label_map.h>

namespace flower {
namespace switchboard {

class Listener;

class Directory {
 public:
  arpc::Status LookupListener(const LabelMap& labels, LabelMap* resolved_labels,
                              std::shared_ptr<Listener>* listener);
};

}  // namespace switchboard
}  // namespace flower

#endif
