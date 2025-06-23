#ifndef HTTP_SERVER_RESPONSE_H
#define HTTP_SERVER_RESPONSE_H

#include "http.h"

struct http_status_line {
    struct http_version version;
    enum http_status status;
    char *reason;
};

struct http_response {
    struct http_status_line status_line;
    struct http_header_entry *headers;
    char *content;
};

int http_response_write(const struct http_response *response, char *buf,
                        int buf_size);

#endif
