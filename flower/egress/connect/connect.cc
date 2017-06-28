#include <sys/socket.h>
#include <sys/un.h>

#include <netdb.h>

#include <cstring>
#include <memory>
#include <sstream>

#include <arpc++/arpc++.h>

#include <flower/proto/egress.h>
#include <flower/proto/switchboard.h>

using namespace flower;

namespace {
class ConnectEgress final : public proto::egress::Egress::Service {
 public:
  arpc::Status Connect(arpc::ServerContext* context,
                       const proto::egress::ConnectRequest* request,
                       proto::egress::ConnectResponse* response) override {
    // Convert the provided labels to a socket address.
    const auto& out_labels = request->out_labels();
    auto address_family = out_labels.find("address_family");
    if (address_family == out_labels.end()) {
      return arpc::Status(arpc::StatusCode::INVALID_ARGUMENT,
                          "Label address_family is absent");
    } else if (address_family->second == "inet" ||
               address_family->second == "inet6") {
      // IPv4 and IPv6 addresses: call into getaddrinfo() to convert to
      // a sockaddr structure. Don't allow DNS lookups, as the resolver
      // process must be used for that.
      const char* address;
      if (arpc::Status status =
              ExtractLabel(out_labels, "server_address", &address);
          !status.ok())
        return status;
      const char* port;
      if (arpc::Status status = ExtractLabel(out_labels, "server_port", &port);
          !status.ok())
        return status;

      addrinfo hints = {};
      hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;
      hints.ai_family = address_family->second == "inet" ? AF_INET : AF_INET6;
      hints.ai_socktype = SOCK_STREAM;

      struct addrinfo* result;
      if (int error = getaddrinfo(address, port, &hints, &result); error != 0)
        return arpc::Status(arpc::StatusCode::INVALID_ARGUMENT,
                            gai_strerror(error));
      arpc::Status status =
          MakeConnection(result->ai_addr, result->ai_addrlen, response);
      freeaddrinfo(result);
      return status;
    } else if (address_family->second == "unix") {
      // UNIX sockets: copy the path label into a sockaddr_un structure.
      const char* path;
      if (arpc::Status status = ExtractLabel(out_labels, "server_path", &path);
          !status.ok())
        return status;

      union {
        sockaddr sa;
        sockaddr_un sun;
      } address;
      if (std::strlen(path) >= sizeof(address.sun.sun_path))
        return arpc::Status(arpc::StatusCode::INVALID_ARGUMENT,
                            "Label server_path has a value that is too long");
      address.sun.sun_family = AF_UNIX;
      std::strcpy(address.sun.sun_path, path);

      return MakeConnection(&address.sa, sizeof(address), response);
    } else {
      return arpc::Status(arpc::StatusCode::INVALID_ARGUMENT,
                          "Label address_family has an unsupported value");
    }
  }

 private:
  // Extracts a label value from a map of labels, throwing an error when
  // it cannot be used as a C string.
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
      std::ostringstream ss;
      ss << "Label " << label << " is absent";
      return arpc::Status(arpc::StatusCode::INVALID_ARGUMENT, ss.str());
    }
    return arpc::Status::OK;
  }

  // Performs the actual connect operation, storing the resulting
  // connected file descriptor in the RPC response.
  arpc::Status MakeConnection(const sockaddr* sa, socklen_t salen,
                              proto::egress::ConnectResponse* response) {
    int s = socket(sa->sa_family, SOCK_STREAM, 0);
    if (s < 0)
      return arpc::Status(arpc::StatusCode::INTERNAL, std::strerror(errno));
    auto fd = std::make_unique<arpc::FileDescriptor>(s);
    if (connect(fd->get(), sa, salen) != 0)
      return arpc::Status(arpc::StatusCode::INTERNAL, std::strerror(errno));
    response->set_server(std::move(fd));
    return arpc::Status::OK;
  }
};
}

int main() {
  // TODO(ed): Get file descriptor from somewhere.
  auto switchboard_fd = std::make_shared<arpc::FileDescriptor>(-1);
  auto channel = arpc::CreateChannel(switchboard_fd);
  auto stub = proto::switchboard::Switchboard::NewStub(channel);

  // Call into the switchboard to register a egress capable of
  // translating hostname/service labels.
  arpc::ClientContext context;
  proto::switchboard::EgressStartRequest request;
  proto::switchboard::EgressStartResponse response;
  arpc::Status status = stub->EgressStart(&context, request, &response);
  if (!status.ok()) {
    // TODO(ed): Log error.
    return 1;
  }
  if (!response.egress()) {
    // TODO(ed): File descriptor must be present.
    return 1;
  }

  // Egress has been registered. Process incoming requests.
  arpc::ServerBuilder builder(response.egress());
  ConnectEgress connect_egress;
  builder.RegisterService(&connect_egress);
  auto server = builder.Build();
  while (server->HandleRequest() == 0) {
  }
  // TODO(ed): Log error.
  return 1;
}
