#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "worker.h"

int worker_run(int conn_fd, const struct sockaddr_in *addr)
{
    char buf[INET_ADDRSTRLEN];

    if(inet_ntop(AF_INET, &addr->sin_addr, buf, sizeof(buf)) == NULL) {
        perror("inet_ntop()");
        goto exit;
    }

    printf("Incoming connection from %s\n", buf);

exit:
    close(conn_fd);
    return EXIT_FAILURE;
}
