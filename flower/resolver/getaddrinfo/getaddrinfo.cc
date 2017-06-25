#include <netdb.h>

#include <algorithm>
#include <memory>
#include <string>

#include <arpc++/arpc++.h>

#include <flower/proto/resolver.h>
#include <flower/proto/switchboard.h>
#include <flower/util/sockaddr.h>

using namespace flower;

namespace {
class GetaddrinfoResolver final : public proto::resolver::Resolver::Service {
 public:
  arpc::Status Resolve(arpc::ServerContext* context,
                       const proto::resolver::ResolveRequest* request,
                       proto::resolver::ResolveResponse* response) override {
    // Convert string labels.
    const auto& in_labels = request->in_labels();
    const char* hostname = nullptr;
    if (auto match = in_labels.find("dst_hostname"); match != in_labels.end()) {
      const std::string& str = match->second;
      if (std::find(str.begin(), str.end(), '\0') != str.end())
        return arpc::Status(
            arpc::StatusCode::INVALID_ARGUMENT,
            "dst_hostname contains a null byte, which is not allowed");
      hostname = str.c_str();
    }
    const char* service = nullptr;
    if (auto match = in_labels.find("dst_service"); match != in_labels.end()) {
      const std::string& str = match->second;
      if (std::find(str.begin(), str.end(), '\0') != str.end())
        return arpc::Status(
            arpc::StatusCode::INVALID_ARGUMENT,
            "dst_service contains a null byte, which is not allowed");
      service = str.c_str();
    }

    // Invoke getaddrinfo().
    addrinfo* result;
    int error = getaddrinfo(hostname, service, nullptr, &result);
    if (error != 0)
      return arpc::Status(arpc::StatusCode::NOT_FOUND, gai_strerror(error));

    // Convert results.
    for (addrinfo* info = result; info != nullptr; info = info->ai_next) {
      proto::resolver::Target* target = response->add_targets();
      util::ConvertSockaddrToLabels(info->ai_addr, info->ai_addrlen,
                                    "dst", target->mutable_out_labels());
      target->set_weight(1);
    }
    // TODO(ed): This should be configurable.
    response->set_max_cache_duration_seconds(60);
    freeaddrinfo(result);
    return arpc::Status::OK;
  }
};
}

int main() {
  // TODO(ed): Get file descriptor from somewhere.
  auto switchboard_fd = std::make_shared<arpc::FileDescriptor>(-1);
  auto channel = arpc::CreateChannel(switchboard_fd);
  auto stub = proto::switchboard::Switchboard::NewStub(channel);

  // Call into the switchboard to register a resolver capable of
  // translating hostname/service labels.
  arpc::ClientContext context;
  proto::switchboard::ResolverStartRequest request;
  request.add_in_labels("dst_hostname");
  request.add_in_labels("dst_service");
  proto::switchboard::ResolverStartResponse response;
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
