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

int socket_create(void) {
    int sock_fd;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0) {
        perror("socket()");
    }

    return sock_fd;
}

int socket_set_reuseaddr(int sock_fd) {
    int val;

    val = 1;
    val = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if(val < 0) {
        perror("setsockopt()");
    }

    return val;
}

int socket_bind(int sock_fd) {
    struct sockaddr_in sock_addr;
    int res;

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(listen_port);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    res = bind(sock_fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
    if(res < 0) {
        perror("bind()");
    }

    return res;
}

int socket_listen(int sock_fd) {
    int res;

    res = listen(sock_fd, max_pending_conns);
    if(res < 0) {
        perror("listen()");
    }

    return res;
}

int main(void)
{
    int res;
    int sock_fd;
    int conn_fd;
    struct sockaddr_in conn_addr;
    socklen_t conn_addr_len;
    int worker_pid;

    sock_fd = socket_create();
    if(sock_fd < 0) {
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    res = socket_set_reuseaddr(sock_fd);
    if(res < 0) {
        goto exit;
    }
#endif

    res = socket_bind(sock_fd);
    if(res < 0) {
        goto exit;
    }

    res = socket_listen(sock_fd);
    if(res < 0) {
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

