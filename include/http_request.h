#ifndef HTTP_SERVER_HTTP_REQUEST_H
#define HTTP_SERVER_HTTP_REQUEST_H

#include "http.h"

struct http_req_line {
    enum http_method method;
    char *uri;
    struct http_ver ver;
};

struct http_req {
    struct http_req_line req_line;
    struct http_hdr *hdrs;
    /* The content field may be here */
};

int http_req_line_parse(struct http_req_line *request_line, char *str);

int http_hdr_parse(struct http_hdr *header, char *str);

#endif

