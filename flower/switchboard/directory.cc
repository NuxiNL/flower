#include <flower/switchboard/directory.h>

using arpc::Status;
using arpc::StatusCode;
using flower::switchboard::Listener;
using flower::switchboard::Directory;
using flower::switchboard::LabelMap;

Status Directory::RegisterListener(const LabelMap& in_labels,
                                   std::unique_ptr<Listener> listener) {
  return Status(StatusCode::UNIMPLEMENTED, "TODO(ed): Implement!");
}

Status Directory::LookupListener(const LabelMap& out_labels,
                                 LabelMap* connection_labels,
                                 std::shared_ptr<Listener>* listener) {
  return Status(StatusCode::UNIMPLEMENTED, "TODO(ed): Implement!");
}
