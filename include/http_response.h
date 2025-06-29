#ifndef HTTP_SERVER_RESPONSE_H
#define HTTP_SERVER_RESPONSE_H

#include "http.h"

struct http_stat_line {
    struct http_ver ver;
    enum http_status status;
    /* Value is not const because a pointer to a dynamically allocated memory
     * region can be assigned here, which needs to be freed */
    char *reason;
};

enum http_res_cont_t {
    http_cont_none = 0,
    http_cont_buffer,
    http_cont_file
};

struct http_res {
    struct http_stat_line status_line;
    struct http_hdr *hdrs;
    union {
        char *buf;
        int fd;
    } content;
    enum http_res_cont_t content_type;
    int content_len;
};

int
http_res_write(const struct http_res *response, char *buf, int buf_size);

#endif
