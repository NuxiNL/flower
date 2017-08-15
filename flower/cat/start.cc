// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <sys/socket.h>

#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <optional>

#include <arpc++/arpc++.h>

#include <flower/cat/configuration.ad.h>
#include <flower/cat/start.h>
#include <flower/protocol/server.ad.h>
#include <flower/protocol/switchboard.ad.h>
#include <flower/util/fd_streambuf.h>
#include <flower/util/label_map.h>
#include <flower/util/logger.h>
#include <flower/util/null_streambuf.h>

using arpc::Channel;
using arpc::ClientContext;
using arpc::FileDescriptor;
using arpc::ServerBuilder;
using arpc::ServerContext;
using arpc::Status;
using flower::protocol::server::ConnectRequest;
using flower::protocol::server::ConnectResponse;
using flower::protocol::server::Server;
using flower::protocol::switchboard::ClientConnectRequest;
using flower::protocol::switchboard::ClientConnectResponse;
using flower::protocol::switchboard::ServerStartRequest;
using flower::protocol::switchboard::ServerStartResponse;
using flower::protocol::switchboard::Switchboard;
using flower::util::LabelMapToJson;
using flower::util::LogTransaction;
using flower::util::Logger;
using flower::util::fd_streambuf;
using flower::util::null_streambuf;

namespace {

// Simple service implementation that accepts an incoming connection
// that can be extracted through GetIncomingConnection().
class ConnectionExtractor : public Server::Service {
 public:
  Status Connect(ServerContext* context, const ConnectRequest* request,
                 ConnectResponse* response) override {
    incoming_connection_ = *request;
    return Status::OK;
  }

  std::optional<ConnectRequest> GetIncomingConnection() {
    std::optional<ConnectRequest> result;
    result.swap(incoming_connection_);
    return result;
  }

 private:
  std::optional<ConnectRequest> incoming_connection_;
};

// Copies data from one file descriptor to another.
void TransferUnidirectionally(FileDescriptor* input, FileDescriptor* output,
                              Logger* logger, size_t buffer_size) {
  std::string buffer;
  while ((input != nullptr || !buffer.empty()) && output != nullptr) {
    // Call into poll() to wait for reading/writing on the file descriptors.
    struct pollfd pfds[2] = {};
    if (input != nullptr && buffer.size() < buffer_size) {
      pfds[0].fd = input->get();
      pfds[0].events = POLLIN;
    } else {
      pfds[0].fd = -1;
    }
    if (buffer.size() > 0) {
      pfds[1].fd = output->get();
      pfds[1].events = POLLOUT;
    } else {
      pfds[1].fd = -1;
    }
    if (poll(pfds, 2, -1) == -1) {
      logger->Log() << "poll(): " << std::strerror(errno);
      break;
    }

    // Read data from the input file descriptor.
    if (pfds[0].revents != 0) {
      size_t offset = buffer.size();
      buffer.resize(buffer_size);
      ssize_t read_length =
          read(input->get(), &buffer[offset], buffer_size - offset);
      if (read_length > 0) {
        buffer.resize(offset + read_length);
      } else {
        if (read_length < 0)
          logger->Log() << "read(): " << std::strerror(errno);
        buffer.resize(offset);
        shutdown(input->get(), SHUT_RD);
        input = nullptr;
      }
    }

    // Write data to the output file descriptor.
    if (pfds[1].revents != 0) {
      ssize_t write_length = write(output->get(), buffer.data(), buffer.size());
      if (write_length >= 0) {
        buffer.erase(0, write_length);
      } else {
        logger->Log() << "write(): " << std::strerror(errno);
        shutdown(output->get(), SHUT_WR);
        output = nullptr;
      }
    }
  }

  // Shut down file descriptors if not done so already.
  if (input != nullptr)
    shutdown(input->get(), SHUT_RD);
  if (output != nullptr)
    shutdown(output->get(), SHUT_WR);
}

}  // namespace

void flower::cat::Start(const Configuration& configuration) {
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
  const auto& switchboard_socket = configuration.switchboard_socket();
  if (!switchboard_socket) {
    logger.Log() << "Cannot start without a switchboard socket";
    std::exit(1);
  }
  size_t buffer_size = configuration.buffer_size();
  if (buffer_size < 1) {
    logger.Log() << "Buffer size not configured";
    std::exit(1);
  }

  // Establish a connection through the switchboard.
  std::shared_ptr<FileDescriptor> connection;
  if (configuration.listen()) {
    // Start listening.
    std::shared_ptr<Channel> channel = CreateChannel(switchboard_socket);
    std::unique_ptr<Switchboard::Stub> stub = Switchboard::NewStub(channel);
    ClientContext context;
    ServerStartRequest request;
    ServerStartResponse response;
    if (Status status = stub->ServerStart(&context, request, &response);
        !status.ok()) {
      logger.Log() << status.error_message();
      std::exit(1);
    }
    if (!response.server()) {
      logger.Log() << "Switchboard did not return a server";
      std::exit(1);
    }

    // Wait for an incoming connection.
    // TODO(ed): Should we extend ARPC to have the async API, so that we
    // don't need the ConnectionExtractor class?
    ServerBuilder builder(response.server());
    ConnectionExtractor connection_extractor;
    builder.RegisterService(&connection_extractor);
    auto server = builder.Build();
    std::optional<ConnectRequest> connect_request;
    logger.Log() << "Waiting for an incoming connection";
    while (!(connect_request = connection_extractor.GetIncomingConnection())) {
      if (int error = server->HandleRequest(); error != 0) {
        logger.Log() << "Failed to obtain incoming connection: "
                     << std::strerror(errno);
        std::exit(1);
      }
    }

    LogTransaction log_transaction = logger.Log();
    log_transaction << "Established connection with client ";
    LabelMapToJson(connect_request->connection_labels(), &log_transaction);
    connection = connect_request->client();
  } else {
    // Connect to a server.
    std::shared_ptr<Channel> channel = CreateChannel(switchboard_socket);
    std::unique_ptr<Switchboard::Stub> stub = Switchboard::NewStub(channel);
    ClientContext context;
    ClientConnectRequest request;
    ClientConnectResponse response;
    if (Status status = stub->ClientConnect(&context, request, &response);
        !status.ok()) {
      logger.Log() << status.error_message();
      std::exit(1);
    }

    LogTransaction log_transaction = logger.Log();
    log_transaction << "Established connection with server ";
    LabelMapToJson(response.connection_labels(), &log_transaction);
    connection = response.server();
  }
  if (!connection) {
    logger.Log() << "Switchboard did not return a connection";
    std::exit(1);
  }

  // Run two threads to copy data on the descriptors in both directions.
  std::thread thread_input(TransferUnidirectionally,
                           configuration.stream_input().get(), connection.get(),
                           &logger, buffer_size);
  TransferUnidirectionally(connection.get(),
                           configuration.stream_output().get(), &logger,
                           buffer_size);
  thread_input.join();
  std::exit(0);
}
