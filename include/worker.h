#ifndef HTTP_SERVER_WORKER_H
#define HTTP_SERVER_WORKER_H

#include <netinet/in.h>

int worker_run(int conn_fd);

#endif

