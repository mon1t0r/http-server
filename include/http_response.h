#ifndef HTTP_SERVER_RESPONSE_H
#define HTTP_SERVER_RESPONSE_H

#include "http.h"

struct http_status_line {
    struct http_version version;
    enum http_status status;
    /* Value is not const because a pointer to a dynamically allocated memory
     * region can be assigned here, which needs to be freed */
    char *reason;
};

struct http_response {
    struct http_status_line status_line;
    struct http_header_entry *headers;
    /* Value is not const because a pointer to a dynamically allocated memory
     * region can be assigned here, which needs to be freed */
    char *content;
    int content_len;
};

int http_response_write(const struct http_response *response, char *buf,
                        int buf_size);

#endif
