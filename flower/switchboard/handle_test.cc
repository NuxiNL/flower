// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#include <memory>

#include <arpc++/arpc++.h>
#include <gtest/gtest.h>

#include <flower/protocol/switchboard.ad.h>
#include <flower/switchboard/directory.h>
#include <flower/switchboard/handle.h>

using arpc::ClientContext;
using arpc::CreateChannel;
using arpc::FileDescriptor;
using arpc::ServerContext;
using arpc::Status;
using arpc::StatusCode;
using flower::protocol::switchboard::ConstrainRequest;
using flower::protocol::switchboard::ConstrainResponse;
using flower::protocol::switchboard::Right;
using flower::protocol::switchboard::Switchboard::NewStub;
using flower::protocol::switchboard::Switchboard::Stub;
using flower::switchboard::Handle;

TEST(Handle, Constrain) {
  // A handle starts out without any constraints configured. Create a
  // second handle that has some basic constraints set up.
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
    EXPECT_TRUE(Handle(nullptr).Constrain(&context, &request, &response).ok());
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
    std::unique_ptr<Stub> stub = NewStub(channel);
    ClientContext context;
    ConstrainResponse response;
    Status status = stub->Constrain(&context, request, &response);
    EXPECT_EQ(StatusCode::PERMISSION_DENIED, status.error_code());
    EXPECT_EQ(
        "Rights { EGRESS_START, INGRESS_CONNECT } "
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
    std::unique_ptr<Stub> stub = NewStub(channel);
    ClientContext context;
    ConstrainResponse response;
    Status status = stub->Constrain(&context, request, &response);
    EXPECT_EQ(StatusCode::PERMISSION_DENIED, status.error_code());
    EXPECT_EQ(
        "In-labels { toast } "
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
    std::unique_ptr<Stub> stub = NewStub(channel);
    ClientContext context;
    ConstrainResponse response;
    Status status = stub->Constrain(&context, request, &response);
    EXPECT_EQ(StatusCode::PERMISSION_DENIED, status.error_code());
    EXPECT_EQ(
        "Out-labels { dog, sheep } "
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
    std::unique_ptr<Stub> stub = NewStub(channel);
    ClientContext context;
    ConstrainResponse response;
    EXPECT_TRUE(stub->Constrain(&context, request, &response).ok());
    EXPECT_TRUE(response.switchboard());
  }
}

// TODO(ed): Add tests for other operations.
