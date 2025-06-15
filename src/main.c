#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "worker.h"

enum {
    listen_port       = 50100,
    max_pending_conns = 5
};

int main(void)
{
    int sock_fd, conn_fd;
    struct sockaddr_in sock_addr, conn_addr;
    socklen_t conn_addr_len;
    int worker_pid;

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(listen_port);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock_fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
        perror("bind()");
        goto exit;
    }

    if(listen(sock_fd, max_pending_conns) < 0) {
        perror("listen()");
        goto exit;
    }

    for(;;) {
        conn_addr_len = sizeof(conn_addr);
        conn_fd = accept(sock_fd, (struct sockaddr *) &conn_addr,
                         &conn_addr_len);
        if(conn_fd < 0) {
            perror("accept()");
            goto exit;
        }

        worker_pid = fork();
        if(worker_pid < 0) {
            perror("fork()");
            goto exit;
        }

        if(worker_pid == 0) {
            close(sock_fd);
            return worker_run(conn_fd, &conn_addr);
        }

        close(conn_fd);
    }

exit:
    close(sock_fd);
    return EXIT_FAILURE;
}

