// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <netdb.h>

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>

#include <arpc++/arpc++.h>

#include <flower/protocol/resolver.ad.h>
#include <flower/protocol/switchboard.ad.h>
#include <flower/util/socket.h>

using arpc::ClientContext;
using arpc::FileDescriptor;
using arpc::ServerBuilder;
using arpc::ServerContext;
using arpc::Status;
using arpc::StatusCode;
using flower::protocol::resolver::Resolver;
using flower::protocol::resolver::ResolveRequest;
using flower::protocol::resolver::ResolveResponse;
using flower::protocol::resolver::Target;
using flower::protocol::switchboard::ResolverStartRequest;
using flower::protocol::switchboard::ResolverStartResponse;
using flower::protocol::switchboard::Switchboard;
using flower::util::AddrinfoDeleter;
using flower::util::ConvertSockaddrToLabels;
using flower::util::GetAddrinfo;

namespace {

class GetaddrinfoResolver final : public Resolver::Service {
 public:
  Status Resolve(ServerContext* context, const ResolveRequest* request,
                 ResolveResponse* response) override {
    // Extract parameters.
    const auto& in_labels = request->resolve_labels();
    const char* hostname;
    if (Status status = ExtractLabel(in_labels, "server_hostname", &hostname);
        !status.ok())
      return status;
    const char* service;
    if (Status status = ExtractLabel(in_labels, "server_service", &service);
        !status.ok())
      return status;

    // Invoke getaddrinfo().
    std::unique_ptr<addrinfo, AddrinfoDeleter> result;
    if (int error = GetAddrinfo(hostname, service, nullptr, &result);
        error != 0)
      return Status(StatusCode::NOT_FOUND, gai_strerror(error));

    // Convert results.
    for (addrinfo* info = result.get(); info != nullptr; info = info->ai_next) {
      Target* target = response->add_targets();
      ConvertSockaddrToLabels(info->ai_addr, info->ai_addrlen, "server",
                              target->mutable_out_labels());
      target->set_weight(1);
    }
    // TODO(ed): This should be configurable.
    response->set_max_cache_duration_seconds(60);
    return Status::OK;
  }

 private:
  // Extracts an optional label value from a map of labels, throwing an
  // error when it cannot be used as a C string.
  template <typename T>
  static Status ExtractLabel(const T& labels, std::string_view label,
                             const char** result) {
    if (auto match = labels.find(label); match != labels.end()) {
      const std::string& str = match->second;
      if (std::find(str.begin(), str.end(), '\0') != str.end()) {
        std::ostringstream ss;
        ss << "Label " << label
           << " contains a null byte, which is not allowed";
        return Status(StatusCode::INVALID_ARGUMENT, ss.str());
      }
      *result = str.c_str();
    } else {
      *result = nullptr;
    }
    return Status::OK;
  }
};

}  // namespace

int main() {
  // TODO(ed): Get file descriptor from somewhere.
  auto switchboard_fd = std::make_shared<FileDescriptor>(-1);
  auto channel = CreateChannel(switchboard_fd);
  auto stub = Switchboard::NewStub(channel);

  // Call into the switchboard to register a resolver capable of
  // translating hostname/service labels.
  ClientContext context;
  ResolverStartRequest request;
  request.add_resolve_labels("server_hostname");
  request.add_resolve_labels("server_service");
  ResolverStartResponse response;
  Status status = stub->ResolverStart(&context, request, &response);
  if (!status.ok()) {
    // TODO(ed): Log error.
    return 1;
  }
  if (!response.resolver()) {
    // TODO(ed): File descriptor must be present.
    return 1;
  }

  // Resolver has been registered. Process incoming requests.
  ServerBuilder builder(response.resolver());
  GetaddrinfoResolver getaddrinfo_resolver;
  builder.RegisterService(&getaddrinfo_resolver);
  auto server = builder.Build();
  while (server->HandleRequest() == 0) {
  }
  // TODO(ed): Log error.
  return 1;
}
