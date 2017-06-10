#include <argdata.h>
#include <program.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "flower.h"

void program_main(const argdata_t* ad) {
  int serverfd = -1;
  int clientfd = -1;

  argdata_map_iterator_t it;
  const argdata_t* key;
  const argdata_t* value;
  argdata_map_iterate(ad, &it);
  while (argdata_map_next(&it, &key, &value)) {
    const char* keystr;
    if (argdata_get_str_c(key, &keystr) != 0) {
      continue;
    }

    if (strcmp(keystr, "stderr") == 0) {
      int fd;
      argdata_get_fd(value, &fd);
      FILE* out = fdopen(fd, "w");
      fswap(stderr, out);
    } else if (strcmp(keystr, "serverfd") == 0) {
      argdata_get_fd(value, &serverfd);
    } else if (strcmp(keystr, "clientfd") == 0) {
      argdata_get_fd(value, &clientfd);
    }
  }

  if (serverfd < 0 || clientfd < 0) {
    fprintf(stderr, "Missing required serverfd / clientfd parameters\n");
    exit(1);
  }

  flower_start(serverfd, clientfd);
  pthread_exit(nullptr);
}
