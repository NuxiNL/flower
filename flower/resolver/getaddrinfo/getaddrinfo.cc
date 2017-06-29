#include <netdb.h>

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>

#include <arpc++/arpc++.h>

#include <flower/protocol/resolver.ad.h>
#include <flower/protocol/switchboard.ad.h>
#include <flower/util/sockaddr.h>

using namespace flower;

namespace {
class GetaddrinfoResolver final : public protocol::resolver::Resolver::Service {
 public:
  arpc::Status Resolve(arpc::ServerContext* context,
                       const protocol::resolver::ResolveRequest* request,
                       protocol::resolver::ResolveResponse* response) override {
    // Extract parameters.
    const auto& in_labels = request->resolve_labels();
    const char* hostname;
    if (arpc::Status status =
            ExtractLabel(in_labels, "server_hostname", &hostname);
        !status.ok())
      return status;
    const char* service;
    if (arpc::Status status =
            ExtractLabel(in_labels, "server_service", &hostname);
        !status.ok())
      return status;

    // Invoke getaddrinfo().
    addrinfo* result;
    if (int error = getaddrinfo(hostname, service, nullptr, &result);
        error != 0)
      return arpc::Status(arpc::StatusCode::NOT_FOUND, gai_strerror(error));

    // Convert results.
    for (addrinfo* info = result; info != nullptr; info = info->ai_next) {
      protocol::resolver::Target* target = response->add_targets();
      util::ConvertSockaddrToLabels(info->ai_addr, info->ai_addrlen, "server",
                                    target->mutable_out_labels());
      target->set_weight(1);
    }
    // TODO(ed): This should be configurable.
    response->set_max_cache_duration_seconds(60);
    freeaddrinfo(result);
    return arpc::Status::OK;
  }

 private:
  // Extracts an optional label value from a map of labels, throwing an
  // error when it cannot be used as a C string.
  template <typename T>
  static arpc::Status ExtractLabel(const T& labels, std::string_view label,
                                   const char** result) {
    if (auto match = labels.find(label); match != labels.end()) {
      const std::string& str = match->second;
      if (std::find(str.begin(), str.end(), '\0') != str.end()) {
        std::ostringstream ss;
        ss << "Label " << label
           << " contains a null byte, which is not allowed";
        return arpc::Status(arpc::StatusCode::INVALID_ARGUMENT, ss.str());
      }
      *result = str.c_str();
    } else {
      *result = nullptr;
    }
    return arpc::Status::OK;
  }
};
}

int main() {
  // TODO(ed): Get file descriptor from somewhere.
  auto switchboard_fd = std::make_shared<arpc::FileDescriptor>(-1);
  auto channel = arpc::CreateChannel(switchboard_fd);
  auto stub = protocol::switchboard::Switchboard::NewStub(channel);

  // Call into the switchboard to register a resolver capable of
  // translating hostname/service labels.
  arpc::ClientContext context;
  protocol::switchboard::ResolverStartRequest request;
  request.add_resolve_labels("server_hostname");
  request.add_resolve_labels("server_service");
  protocol::switchboard::ResolverStartResponse response;
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
