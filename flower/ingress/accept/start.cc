// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <sys/socket.h>

#include <cstdlib>
#include <memory>
#include <streambuf>

#include <arpc++/arpc++.h>

#include <flower/ingress/accept/configuration.ad.h>
#include <flower/ingress/accept/start.h>
#include <flower/protocol/switchboard.ad.h>
#include <flower/util/fd_streambuf.h>
#include <flower/util/label_map.h>
#include <flower/util/logger.h>
#include <flower/util/null_streambuf.h>
#include <flower/util/socket.h>

using arpc::Channel;
using arpc::ClientContext;
using arpc::FileDescriptor;
using arpc::Status;
using flower::protocol::switchboard::IngressConnectRequest;
using flower::protocol::switchboard::IngressConnectResponse;
using flower::protocol::switchboard::Switchboard;
using flower::util::AcceptSocketConnection;
using flower::util::ConvertSockaddrToLabels;
using flower::util::LabelMapToJson;
using flower::util::LogTransaction;
using flower::util::Logger;
using flower::util::fd_streambuf;
using flower::util::null_streambuf;

void flower::ingress::accept::Start(const Configuration& configuration) {
  // Make logging work.
  std::unique_ptr<std::streambuf> logger_streambuf;
  if (const auto& logger_output = configuration.logger_output();
      logger_output) {
    logger_streambuf = std::make_unique<fd_streambuf>(logger_output);
  } else {
    logger_streambuf = std::make_unique<null_streambuf>();
  }
  Logger logger(logger_streambuf.get());

  // Check that all required fields are present.
  const auto& listening_socket = configuration.listening_socket();
  if (!listening_socket) {
    logger.Log() << "Cannot start without a listening socket";
    std::exit(1);
  }
  const auto& switchboard_socket = configuration.switchboard_socket();
  if (!switchboard_socket) {
    logger.Log() << "Cannot start without a switchboard socket";
    std::exit(1);
  }

  std::shared_ptr<Channel> channel = CreateChannel(switchboard_socket);
  std::unique_ptr<Switchboard::Stub> stub = Switchboard::NewStub(channel);
  for (;;) {
    // Accept an incoming connection.
    union {
      sockaddr sa;
      sockaddr_storage ss;
    } client_address;
    socklen_t client_address_len = sizeof(client_address);
    std::unique_ptr<FileDescriptor> connection;
    if (Status status =
            AcceptSocketConnection(*listening_socket, &connection,
                                   &client_address.sa, &client_address_len);
        !status.ok()) {
      logger.Log() << status.error_message();
      std::exit(1);
    }

    // Store file descriptors in request and convert peer adress to labels.
    IngressConnectRequest request;
    request.set_client(std::move(connection));
    ConvertSockaddrToLabels(&client_address.sa, client_address_len, "client",
                            request.mutable_out_labels());

    // Convert local address to labels.
    union {
      sockaddr sa;
      sockaddr_storage ss;
    } server_address;
    socklen_t server_address_len = sizeof(server_address);
    if (getsockname(request.client()->get(), &server_address.sa,
                    &server_address_len) == 0)
      ConvertSockaddrToLabels(&server_address.sa, server_address_len, "server",
                              request.mutable_out_labels());

    // Submit connection to the switchboard.
    ClientContext context;
    IngressConnectResponse response;
    Status status = stub->IngressConnect(&context, request, &response);
    if (status.ok()) {
      LogTransaction log_transaction = logger.Log();
      log_transaction << "Successfully forwarded connection with labels ";
      LabelMapToJson(response.connection_labels(), &log_transaction);
    } else {
      // TODO(ed): Terminate in case the switchboard connection is dead.
      LogTransaction log_transaction = logger.Log();
      log_transaction << "Failed to forward connection with labels ";
      LabelMapToJson(request.out_labels(), &log_transaction);
      log_transaction << ": " << status.error_message();
    }
  }
}
