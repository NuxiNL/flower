#include <thread>
#include "flower_service.h"

class ServerChannelHandler final
    : public flower_service::ServerChannelService::Service {
 public:
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
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED,
                        "Operation not provided by this implementation");
  }
};

class ClientChannelHandler final
    : public flower_service::ClientChannelService::Service {
 public:
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
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED,
                        "Operation not provided by this implementation");
  }
};

int flower_start(int serverfd, int clientfd) {
  std::thread client_thread([clientfd]() {
    arpc::ServerBuilder builder(clientfd);
    ClientChannelHandler client_handler;
    builder.RegisterService(&client_handler);
    std::unique_ptr<arpc::Server> server(builder.Build());
    server->HandleRequests();
  });

  {
    arpc::ServerBuilder builder(serverfd);
    ServerChannelHandler server_handler;
    builder.RegisterService(&server_handler);
    std::unique_ptr<arpc::Server> server(builder.Build());
    server->HandleRequests();
  }

  client_thread.join();
  return 1;
}
