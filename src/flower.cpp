#include <sys/socket.h>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include "flower_service.h"

struct overlap_error : public std::runtime_error {
  overlap_error(std::string const& i)
      : std::runtime_error("Overlapping item: " + i), item(i) {
  }
  std::string item;
};

template <typename T, typename U>
static void merge_unique(T& l, U const& r) {
  for (auto const& item : r) {
    if (l.find(item.first) == l.end()) {
      l.insert(item);
    } else {
      throw overlap_error(item.first);
    }
  }
}

template <typename ChannelHandler, typename Container>
static void start_new_channel(int fd, Container const& constraints) {
  std::thread thr([fd, constraints]() {
    try {
      arpc::ServerBuilder builder(fd);
      ChannelHandler ch_new(constraints);
      builder.RegisterService(&ch_new);
      std::unique_ptr<arpc::Server> server(builder.Build());
      server->HandleRequests();
    } catch (std::exception& e) {
      std::cerr << "Destroying channel because of an uncaught exception: "
                << e.what() << std::endl;
    }
    pthread_exit(nullptr);
  });
  thr.detach();
}

template <typename ChannelHandler, typename Container>
static std::shared_ptr<arpc::FileDescriptor> create_new_channel(ChannelHandler const& ch,
                              Container const& additional_constraints) {
  auto constraints = ch.get_constraints();
  merge_unique(constraints, additional_constraints);

  int fds[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
    throw std::runtime_error("socketpair() failed");
  }
  start_new_channel<ChannelHandler>(fds[0], constraints);
  return std::make_shared<arpc::FileDescriptor>(fds[1]);
}

class ServerChannelHandler final
    : public flower_service::ServerChannelService::Service {
  std::map<std::string, std::string> constraints;

 public:
  ServerChannelHandler(decltype(constraints) const & c) : constraints(c) {
  }

  decltype(constraints) const & get_constraints() const {
    return constraints;
  }

  arpc::Status AcceptIncomingConnections(
      arpc::ServerContext* context,
      const flower_service::AcceptOptions* request,
      arpc::ServerWriter<flower_service::Connection>* writer) override {
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED,
                        "Operation not provided by this implementation");
  }

  arpc::Status CreateNewChannel(
      arpc::ServerContext* context,
      const flower_service::ServerChannelOptions* request,
      flower_service::ServerChannel* response) override {
    try {
      response->set_channel(
          create_new_channel(*this, request->additional_constraints()));
      return {};
    } catch (overlap_error& e) {
      return {arpc::StatusCode::PERMISSION_DENIED,
              "Constraint overlap in item '" + e.item +
                  "', cannot decrease constraints"};
    } catch (...) {
      return {arpc::StatusCode::UNKNOWN, "An exception occurred"};
    }
  }
};

class ClientChannelHandler final
    : public flower_service::ClientChannelService::Service {
  std::map<std::string, std::string> constraints;

 public:
  ClientChannelHandler(decltype(constraints) const & c) : constraints(c) {
  }

  decltype(constraints) const & get_constraints() const {
    return constraints;
  }

  arpc::Status Connect(arpc::ServerContext* context,
                       const flower_service::ConnectOptions* request,
                       flower_service::Connection* response) override {
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED,
                        "Operation not provided by this implementation");
  }

  arpc::Status CreateNewChannel(
      arpc::ServerContext* context,
      const flower_service::ClientChannelOptions* request,
      flower_service::ClientChannel* response) override {
    try {
      response->set_channel(
          create_new_channel(*this, request->additional_constraints()));
      return {};
    } catch (overlap_error& e) {
      return {arpc::StatusCode::PERMISSION_DENIED,
              "Constraint overlap in item '" + e.item +
                  "', cannot decrease constraints"};
    } catch (...) {
      return {arpc::StatusCode::UNKNOWN, "An exception occurred"};
    }
  }
};

void flower_start(int serverfd, int clientfd) {
  const std::map<std::string, std::string> no_constraints;

  start_new_channel<ServerChannelHandler>(serverfd, no_constraints);
  start_new_channel<ClientChannelHandler>(clientfd, no_constraints);
}
