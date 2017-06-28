#include <flower/switchboard/directory.h>

using arpc::Status;
using flower::switchboard::Listener;
using flower::switchboard::Directory;
using flower::switchboard::LabelMap;

Status Directory::LookupListener(const LabelMap& labels,
                                 LabelMap* resolved_labels,
                                 std::shared_ptr<Listener>* listener) {
  // TODO(ed): Implement.
  return Status::OK;
}
