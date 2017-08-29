// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <unistd.h>

#include <memory>
#include <thread>

#include <arpc++/arpc++.h>
#include <gtest/gtest.h>

#include <flower/protocol/server.ad.h>
#include <flower/protocol/switchboard.ad.h>
#include <flower/switchboard/directory.h>
#include <flower/switchboard/handle.h>
#include <flower/switchboard/target_picker.h>
#include <flower/switchboard/worker_pool.h>
#include <flower/util/label_map.h>

using arpc::ClientContext;
using arpc::CreateChannel;
using arpc::FileDescriptor;
using arpc::ServerBuilder;
using arpc::ServerContext;
using arpc::Status;
using arpc::StatusCode;
using flower::protocol::server::ConnectRequest;
using flower::protocol::server::ConnectResponse;
using flower::protocol::server::Server;
using flower::protocol::switchboard::ClientConnectRequest;
using flower::protocol::switchboard::ClientConnectResponse;
using flower::protocol::switchboard::ConstrainRequest;
using flower::protocol::switchboard::ConstrainResponse;
using flower::protocol::switchboard::Right;
using flower::protocol::switchboard::ServerStartRequest;
using flower::protocol::switchboard::ServerStartResponse;
using flower::protocol::switchboard::Switchboard;
using flower::switchboard::Directory;
using flower::switchboard::Handle;
using flower::switchboard::TargetPicker;
using flower::switchboard::WorkerPool;
using flower::util::LabelMap;

TEST(Handle, Constrain) {
  // A handle starts out without any constraints configured. Create a
  // second handle that has some basic constraints set up.
  WorkerPool worker_pool(10, nullptr);
  std::shared_ptr<FileDescriptor> connection;
  {
    ConstrainRequest request;
    request.add_rights(Right::CLIENT_CONNECT);
    request.add_rights(Right::SERVER_START);
    auto in_labels = request.mutable_in_labels();
    (*in_labels)["hello"] = "world";
    (*in_labels)["toast"] = "french";
    auto out_labels = request.mutable_out_labels();
    (*out_labels)["dog"] = "brown";
    (*out_labels)["sheep"] = "white";

    ServerContext context;
    ConstrainResponse response;
    EXPECT_TRUE(Handle(nullptr, nullptr, &worker_pool)
                    .Constrain(&context, &request, &response)
                    .ok());
    connection = response.switchboard();
  }

  // Increasing the rights should fail.
  {
    ConstrainRequest request;
    request.add_rights(Right::CLIENT_CONNECT);
    request.add_rights(Right::EGRESS_START);
    request.add_rights(Right::INGRESS_CONNECT);
    request.add_rights(Right::SERVER_START);

    auto channel = CreateChannel(connection);
    std::unique_ptr<Switchboard::Stub> stub = Switchboard::NewStub(channel);
    ClientContext context;
    ConstrainResponse response;
    Status status = stub->Constrain(&context, request, &response);
    EXPECT_EQ(StatusCode::PERMISSION_DENIED, status.error_code());
    EXPECT_EQ(
        "Rights [\"EGRESS_START\",\"INGRESS_CONNECT\"] "
        "are not present on this handle",
        status.error_message());
  }

  // Input labels cannot be overwritten with different values.
  {
    ConstrainRequest request;
    auto in_labels = request.mutable_in_labels();
    (*in_labels)["hello"] = "world";
    (*in_labels)["toast"] = "italian";
    (*in_labels)["direction"] = "left";

    auto channel = CreateChannel(connection);
    std::unique_ptr<Switchboard::Stub> stub = Switchboard::NewStub(channel);
    ClientContext context;
    ConstrainResponse response;
    Status status = stub->Constrain(&context, request, &response);
    EXPECT_EQ(StatusCode::PERMISSION_DENIED, status.error_code());
    EXPECT_EQ(
        "In-labels [\"toast\"] "
        "are already defined with different values",
        status.error_message());
  }

  // Output labels cannot be overwritten with different values.
  {
    ConstrainRequest request;
    auto out_labels = request.mutable_out_labels();
    (*out_labels)["dog"] = "black";
    (*out_labels)["sheep"] = "black";

    auto channel = CreateChannel(connection);
    std::unique_ptr<Switchboard::Stub> stub = Switchboard::NewStub(channel);
    ClientContext context;
    ConstrainResponse response;
    Status status = stub->Constrain(&context, request, &response);
    EXPECT_EQ(StatusCode::PERMISSION_DENIED, status.error_code());
    EXPECT_EQ(
        "Out-labels [\"dog\",\"sheep\"] "
        "are already defined with different values",
        status.error_message());
  }

  // Sticking to the rules allows us to constrain.
  {
    ConstrainRequest request;
    request.add_rights(Right::CLIENT_CONNECT);
    auto in_labels = request.mutable_in_labels();
    (*in_labels)["hello"] = "world";
    (*in_labels)["pasta"] = "italian";
    auto out_labels = request.mutable_out_labels();
    (*out_labels)["dog"] = "brown";
    (*out_labels)["fish"] = "orange";

    auto channel = CreateChannel(connection);
    std::unique_ptr<Switchboard::Stub> stub = Switchboard::NewStub(channel);
    ClientContext context;
    ConstrainResponse response;
    EXPECT_TRUE(stub->Constrain(&context, request, &response).ok());
    EXPECT_TRUE(response.switchboard());
  }
}

// A server process that does nothing more than accepting connections
// and writing back a textual response of all of the data it received in
// the request.
class LabelEchoingServer final : public Server::Service {
 public:
  Status Connect(ServerContext* context, const ConnectRequest* request,
                 ConnectResponse* response) override {
    std::thread(
        [ fd{request->client()}, labels{request->connection_labels()} ]() {
          std::ostringstream ss;
          ss << "Got incoming connection with labels:" << std::endl;
          for (const auto& label : labels)
            ss << label.first << " " << label.second << std::endl;
          std::string str = ss.str();
          EXPECT_EQ(str.size(), write(fd->get(), str.data(), str.size()));
        })
        .detach();
    return Status::OK;
  }
};

TEST(Handle, ClientServer) {
  Directory directory;
  TargetPicker target_picker;
  WorkerPool worker_pool(10, nullptr);
  Handle handle(&directory, &target_picker, &worker_pool);

  // Start a server that listens on {"host":"banana.apple.com"}.
  std::shared_ptr<FileDescriptor> banana_fd;
  {
    ServerStartRequest request;
    auto in_labels = request.mutable_in_labels();
    (*in_labels)["host"] = "banana.apple.com";

    ServerContext context;
    ServerStartResponse response;
    EXPECT_TRUE(handle.ServerStart(&context, &request, &response).ok());
    banana_fd = response.server();
  }

  // Connecting to {"host":"pear.apple.com"} should fail.
  {
    ClientConnectRequest request;
    auto out_labels = request.mutable_out_labels();
    (*out_labels)["host"] = "pear.apple.com";

    ServerContext context;
    ClientConnectResponse response;
    Status status = handle.ClientConnect(&context, &request, &response);
    EXPECT_EQ(StatusCode::NOT_FOUND, status.error_code());
    EXPECT_EQ("No matching target found", status.error_message());
  }

  // Spawn a thread to start processing connections going to
  // {"host":"banana.apple.com"}.
  std::thread server_thread([banana_fd]() {
    ServerBuilder builder(banana_fd);
    LabelEchoingServer label_echoing_server;
    builder.RegisterService(&label_echoing_server);
    auto server = builder.Build();
    EXPECT_EQ(0, server->HandleRequest());
    EXPECT_EQ(0, server->HandleRequest());
  });

  // Make a connection to
  // {"host":"banana.apple.com","datacenter":"frankfurt"}.
  // This connection should end up on the server created above.
  {
    ClientConnectRequest request;
    auto out_labels = request.mutable_out_labels();
    (*out_labels)["host"] = "banana.apple.com";
    (*out_labels)["datacenter"] = "frankfurt";

    ServerContext context;
    ClientConnectResponse response;
    EXPECT_TRUE(handle.ClientConnect(&context, &request, &response).ok());
    EXPECT_EQ(*out_labels, response.connection_labels());

    char buf[81] = {};
    EXPECT_EQ(sizeof(buf) - 1,
              read(response.server()->get(), buf, sizeof(buf)));
    ASSERT_STREQ(
        "Got incoming connection with labels:\n"
        "datacenter frankfurt\n"
        "host banana.apple.com\n",
        buf);
  }

  // Just connecting to {"datacenter":"frankfurt"} should be sufficient,
  // as none of the labels contradict with {"host":"banana.apple.com"}.
  {
    ClientConnectRequest request;
    auto out_labels = request.mutable_out_labels();
    (*out_labels)["datacenter"] = "frankfurt";

    ServerContext context;
    ClientConnectResponse response;
    EXPECT_TRUE(handle.ClientConnect(&context, &request, &response).ok());
    EXPECT_EQ(
        (LabelMap{{"datacenter", "frankfurt"}, {"host", "banana.apple.com"}}),
        response.connection_labels());

    char buf[81] = {};
    EXPECT_EQ(sizeof(buf) - 1,
              read(response.server()->get(), buf, sizeof(buf)));
    ASSERT_STREQ(
        "Got incoming connection with labels:\n"
        "datacenter frankfurt\n"
        "host banana.apple.com\n",
        buf);
  }

  // Start a second server that listens on {"host":"pear.apple.com"}.
  std::shared_ptr<FileDescriptor> pear_fd;
  {
    ServerStartRequest request;
    auto in_labels = request.mutable_in_labels();
    (*in_labels)["host"] = "pear.apple.com";

    ServerContext context;
    ServerStartResponse response;
    EXPECT_TRUE(handle.ServerStart(&context, &request, &response).ok());
    pear_fd = response.server();
  }

  // Connecting to {"datacenter":"frankfurt"} should now fail, as it
  // becomes ambiguous which server to pick.
  {
    ClientConnectRequest request;
    auto out_labels = request.mutable_out_labels();
    (*out_labels)["datacenter"] = "frankfurt";

    ServerContext context;
    ClientConnectResponse response;
    Status status = handle.ClientConnect(&context, &request, &response);
    EXPECT_EQ(StatusCode::FAILED_PRECONDITION, status.error_code());
    EXPECT_EQ(
        "Labels match multiple targets, "
        "which can be resolved by adding one of the labels [\"host\"]",
        status.error_message());
  }

  // Closing switchboard connections should cause targets to be pruned.
  pear_fd.reset();
  {
    ClientConnectRequest request;
    auto out_labels = request.mutable_out_labels();
    (*out_labels)["host"] = "pear.apple.com";

    ServerContext context;
    ClientConnectResponse response;
    Status status = handle.ClientConnect(&context, &request, &response);
    EXPECT_EQ(StatusCode::NOT_FOUND, status.error_code());
    EXPECT_EQ("No matching target found", status.error_message());
  }

  server_thread.join();
}

// TODO(ed): Add more tests!
