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
  Handle handle(nullptr);

  std::shared_ptr<FileDescriptor> connection;
  {
    ConstrainRequest request;
    request.add_rights(Right::CLIENT_CONNECT);
    request.add_rights(Right::SERVER_START);
    auto in_labels = request.mutable_additional_in_labels();
    (*in_labels)["hello"] = "world";
    (*in_labels)["toast"] = "french";
    auto out_labels = request.mutable_additional_out_labels();
    (*out_labels)["dog"] = "brown";
    (*out_labels)["sheep"] = "white";

    ServerContext context;
    ConstrainResponse response;
    EXPECT_TRUE(handle.Constrain(&context, &request, &response).ok());
    connection = response.switchboard();
  }

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

  // TODO(ed): Add more tests!
}
