/* signal.h */
#define _POSIX_C_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "server_config.h"
#include "worker.h"

static void sigchld_handle(int sig)
{
    int save_errno = errno;
    waitpid(-1, NULL, WNOHANG);
    errno = save_errno;
}

static int set_sigaction(void)
{
    struct sigaction sa;
    int res;

    sa.sa_handler = &sigchld_handle;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    res = sigaction(SIGCHLD, &sa, NULL);
    if(res < 0) {
        perror("sigaction()");
    }

    return res;
}

static int sock_creat(void)
{
    int sock_fd;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0) {
        perror("socket()");
    }

    return sock_fd;
}

static int sock_set_opt(int sock_fd)
{
    int val;

    val = 1;
    val = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if(val < 0) {
        perror("setsockopt()");
    }

    return val;
}

static int sock_bind(int sock_fd) {
    struct sockaddr_in sock_addr;
    int res;

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(conf_listen_port);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    res = bind(sock_fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
    if(res < 0) {
        perror("bind()");
    }

    return res;
}

static int sock_listen(int sock_fd)
{
    int res;

    res = listen(sock_fd, conf_max_pending_conns);
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
    int worker_pid;

    sock_fd = sock_creat();
    if(sock_fd < 0) {
        exit(EXIT_FAILURE);
    }

    res = sock_set_opt(sock_fd);
    if(res < 0) {
        goto exit;
    }

    res = sock_bind(sock_fd);
    if(res < 0) {
        goto exit;
    }

    res = sock_listen(sock_fd);
    if(res < 0) {
        goto exit;
    }

    res = set_sigaction();
    if(res < 0) {
        goto exit;
    }

    for(;;) {
        conn_fd = accept(sock_fd, NULL, NULL);
        if(conn_fd < 0) {
            /* Interrupted by a signal */
            if(errno == EINTR) {
                continue;
            }
            perror("accept()");
            goto exit;
        }

        worker_pid = fork();
        if(worker_pid < 0) {
            perror("fork()");
            goto exit;
        }

        /* If in child process */
        if(worker_pid == 0) {
            close(sock_fd);
            return worker_run(conn_fd);
        }

        close(conn_fd);
    }

exit:
    close(sock_fd);
    return EXIT_FAILURE;
}

