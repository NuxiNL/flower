#include <sys/socket.h>
#include <sys/un.h>

#include <iostream>
#include <thread>

#include <arpc++/arpc++.h>

#include <flower/switchboard/directory.h>
#include <flower/switchboard/handle.h>
#include <flower/util/sockaddr.h>
#include <flower/util/socket.h>

using arpc::FileDescriptor;
using arpc::Server;
using arpc::ServerBuilder;
using arpc::Status;
using flower::switchboard::Directory;
using flower::switchboard::Handle;
using flower::util::AcceptSocketConnection;
using flower::util::CreateSocket;
using flower::util::InitializeSockaddrUn;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: flower_switchboard socket_path" << std::endl;
    return 1;
  }

  // Bind to a UNIX socket.
  std::unique_ptr<FileDescriptor> s;
  if (Status status = CreateSocket(AF_UNIX, &s); !status.ok()) {
    // TODO(ed): Report error.
    return 1;
  }
  union {
    sockaddr sa;
    sockaddr_un sun;
  } path = {};
  if (Status status = InitializeSockaddrUn(argv[1], &path.sun); !status.ok()) {
    // TODO(ed): Report error.
    return 1;
  }
  std::remove(argv[1]);
  if (bind(s->get(), &path.sa, sizeof(path.sun)) != 0) {
    std::cerr << "Failed to bind socket: " << std::strerror(errno) << std::endl;
    return 1;
  }
  if (listen(s->get(), 0) != 0) {
    std::cerr << "Failed to listen: " << std::strerror(errno) << std::endl;
    return 1;
  }

  // Start processing incoming requests.
  Directory directory;
  for (;;) {
    std::unique_ptr<FileDescriptor> connection;
    if (Status status =
            AcceptSocketConnection(*s, &connection, nullptr, nullptr);
        !status.ok()) {
      // TODO(ed): Report error.
      return 1;
    }
    // TODO(ed): Deal with thread creation errors!
    std::thread([ connection{std::move(connection)}, &directory ]() mutable {
      ServerBuilder builder(std::move(connection));
      Handle handle(&directory);
      builder.RegisterService(&handle);
      std::shared_ptr<Server> server = builder.Build();
      while (server->HandleRequest() == 0) {
      }
      // TODO(ed): Log error.
    }).detach();
  }
}
