#ifndef HTTP_SERVER_SERVER_CONFIG_H
#define HTTP_SERVER_SERVER_CONFIG_H

enum {
    /* Listen port */
    conf_listen_port        = 50100,

    /* Maximum pending connections (for listen() syscall) */
    conf_max_pending_conns  = 5,

    /* HTTP Keep-Alive timeout, if client supports keep alive, in seconds */
    conf_keep_alive_timeout = 5,

    /* Send/receive buffer size, in bytes */
    conf_buf_size           = 8192
};

/* Value for Server HTTP header */
#define SERVER_STRING "TestServer/1.0"

/* Path to the served content (request URI will be relative to this path) */
#define CONTENT_PATH "./html"

#endif
