#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include "flower.h"

static void set_sockaddr_un_path(sockaddr_un& s, const char* p) {
  s.sun_len = strlen(p);
  s.sun_family = AF_UNIX;
  strncpy(s.sun_path, p, sizeof(s.sun_path));
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s serversock clientsock\n", argv[0]);
    return 1;
  }

  int serverfd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (serverfd < 0) {
    perror("server socket()");
  }

  sockaddr_un server_sock;
  set_sockaddr_un_path(server_sock, argv[1]);

  if (bind(serverfd, (sockaddr*)&server_sock, sizeof(server_sock)) < 0) {
    perror("server bind()");
  }
  if (listen(serverfd, SOMAXCONN) < 0) {
    perror("server listen()");
  }

  int clientfd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (clientfd < 0) {
    perror("client socket()");
  }

  sockaddr_un client_sock;
  set_sockaddr_un_path(client_sock, argv[2]);

  if (bind(clientfd, (sockaddr*)&client_sock, sizeof(client_sock)) < 0) {
    perror("client bind()");
  }
  if (listen(clientfd, SOMAXCONN) < 0) {
    perror("client listen()");
  }

  return flower_start(serverfd, clientfd);
}
