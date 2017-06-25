#include <memory>

#include <arpc++/arpc++.h>

#include "flower_resolver.h"
#include "flower_switchboard.h"

namespace {
class GetaddrinfoResolver final : public flower::resolver::Resolver::Service {
 public:
  arpc::Status Resolve(arpc::ServerContext* context,
                       const flower::resolver::ResolveRequest* request,
                       flower::resolver::ResolveResponse* response) override {
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED,
                        "Operation not provided by this implementation");
  }
};
}

int main() {
  // TODO(ed): Get file descriptor from somewhere.
  auto switchboard_fd = std::make_shared<arpc::FileDescriptor>(-1);
  auto channel = arpc::CreateChannel(switchboard_fd);
  auto stub = flower::switchboard::Switchboard::NewStub(channel);

  // Call into the switchboard to register a resolver capable of
  // translating hostname/service labels.
  arpc::ClientContext context;
  flower::switchboard::ResolverStartRequest request;
  request.add_in_labels("dst_hostname");
  request.add_in_labels("dst_service");
  flower::switchboard::ResolverStartResponse response;
  arpc::Status status = stub->ResolverStart(&context, request, &response);
  if (!status.ok()) {
    // TODO(ed): Log error.
    return 1;
  }
  if (!response.resolver()) {
    // TODO(ed): File descriptor must be present.
    return 1;
  }

  // Resolver has been registered. Process incoming requests.
  arpc::ServerBuilder builder(response.resolver());
  GetaddrinfoResolver getaddrinfo_resolver;
  builder.RegisterService(&getaddrinfo_resolver);
  auto server = builder.Build();
  while (server->HandleRequest() == 0) {
  }
  // TODO(ed): Log error.
  return 1;
}
