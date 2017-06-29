#include <sys/socket.h>
#include <sys/un.h>

#include <netdb.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <sstream>
#include <string_view>

#include <arpc++/arpc++.h>

#include <flower/proto/egress.h>
#include <flower/proto/switchboard.h>
#include <flower/util/sockaddr.h>
#include <flower/util/socket.h>

using arpc::ClientContext;
using arpc::FileDescriptor;
using arpc::ServerBuilder;
using arpc::ServerContext;
using arpc::Status;
using arpc::StatusCode;
using flower::proto::egress::ConnectRequest;
using flower::proto::egress::ConnectResponse;
using flower::proto::egress::Egress::Service;
using flower::proto::switchboard::EgressStartRequest;
using flower::proto::switchboard::EgressStartResponse;
using flower::proto::switchboard::EgressStartResponse;
using flower::proto::switchboard::Switchboard::NewStub;
using flower::util::CreateSocket;
using flower::util::InitializeSockaddrUn;

namespace {
class ConnectEgress final : public Service {
 public:
  Status Connect(ServerContext* context, const ConnectRequest* request,
                 ConnectResponse* response) override {
    // Convert the provided labels to a socket address.
    const auto& labels = request->labels();
    auto address_family = labels.find("address_family");
    if (address_family == labels.end()) {
      return Status(StatusCode::INVALID_ARGUMENT,
                    "Label address_family is absent");
    } else if (address_family->second == "inet" ||
               address_family->second == "inet6") {
      // IPv4 and IPv6 addresses: call into getaddrinfo() to convert to
      // a sockaddr structure. Don't allow DNS lookups, as the resolver
      // process must be used for that.
      const char* address;
      if (Status status = ExtractLabel(labels, "server_address", &address);
          !status.ok())
        return status;
      const char* port;
      if (Status status = ExtractLabel(labels, "server_port", &port);
          !status.ok())
        return status;

      addrinfo hints = {};
      hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;
      hints.ai_family = address_family->second == "inet" ? AF_INET : AF_INET6;
      hints.ai_socktype = SOCK_STREAM;

      struct addrinfo* result;
      if (int error = getaddrinfo(address, port, &hints, &result); error != 0)
        return Status(StatusCode::INVALID_ARGUMENT, gai_strerror(error));
      Status status =
          MakeConnection(result->ai_addr, result->ai_addrlen, response);
      freeaddrinfo(result);
      return status;
    } else if (address_family->second == "unix") {
      // UNIX sockets: copy the path label into a sockaddr_un structure.
      const char* path;
      if (Status status = ExtractLabel(labels, "server_path", &path);
          !status.ok())
        return status;
      union {
        sockaddr sa;
        sockaddr_un sun;
      } address;
      if (Status status = InitializeSockaddrUn(path, &address.sun);
          !status.ok())
        return status;
      return MakeConnection(&address.sa, sizeof(address), response);
    } else {
      return Status(StatusCode::INVALID_ARGUMENT,
                    "Label address_family has an unsupported value");
    }
  }

 private:
  // Extracts a label value from a map of labels, throwing an error when
  // it cannot be used as a C string.
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
      std::ostringstream ss;
      ss << "Label " << label << " is absent";
      return Status(StatusCode::INVALID_ARGUMENT, ss.str());
    }
    return Status::OK;
  }

  // Performs the actual connect operation, storing the resulting
  // connected file descriptor in the RPC response.
  Status MakeConnection(const sockaddr* sa, socklen_t salen,
                        ConnectResponse* response) {
    std::unique_ptr<FileDescriptor> fd;
    if (Status status = CreateSocket(sa->sa_family, &fd); !status.ok())
      return status;
    if (connect(fd->get(), sa, salen) != 0)
      return Status(StatusCode::INTERNAL, std::strerror(errno));
    response->set_server(std::move(fd));
    return Status::OK;
  }
};
}

int main() {
  // TODO(ed): Get file descriptor from somewhere.
  auto switchboard_fd = std::make_shared<FileDescriptor>(-1);
  auto channel = CreateChannel(switchboard_fd);
  auto stub = NewStub(channel);

  // Call into the switchboard to register a egress capable of
  // translating hostname/service labels.
  ClientContext context;
  EgressStartRequest request;
  EgressStartResponse response;
  Status status = stub->EgressStart(&context, request, &response);
  if (!status.ok()) {
    // TODO(ed): Log error.
    return 1;
  }
  if (!response.egress()) {
    // TODO(ed): File descriptor must be present.
    return 1;
  }

  // Egress has been registered. Process incoming requests.
  ServerBuilder builder(response.egress());
  ConnectEgress connect_egress;
  builder.RegisterService(&connect_egress);
  auto server = builder.Build();
  while (server->HandleRequest() == 0) {
  }
  // TODO(ed): Log error.
  return 1;
}
