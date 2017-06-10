#include <program.h>

#include "flower_service.h"

class ServerHandler final
    : public flower_service::ServerChannelService::Service {
public:
  arpc::Status AcceptIncomingConnections(
      arpc::ServerContext *context,
      const flower_service::AcceptOptions *request,
      arpc::ServerWriter<flower_service::Connection> *writer) override {
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED,
                        "Operation not provided by this implementation");
  }

  arpc::Status
  CreateNewChannel(arpc::ServerContext *context,
                   const flower_service::ServerChannelOptions *request,
                   flower_service::ServerChannel *response) override {
    return arpc::Status(arpc::StatusCode::UNIMPLEMENTED,
                        "Operation not provided by this implementation");
  }
};

void program_main(const argdata_t *ad) {
  arpc::ServerBuilder builder(-1);
  ServerHandler server_handler;
  builder.RegisterService(&server_handler);
  std::unique_ptr<arpc::Server> server(builder.Build());
  server->HandleRequests();
  std::exit(1);
}
