#ifndef HTTP_SERVER_HTTP_REQUEST_H
#define HTTP_SERVER_HTTP_REQUEST_H

#include "http.h"

struct http_request_line {
    enum http_method method;
    char *request_uri;
    struct http_version version;
};

struct http_request {
    struct http_request_line request_line;
    struct http_header_entry *headers;
    char *content;
};

int http_request_line_parse(struct http_request_line *request_line, char *str);

int http_header_parse(struct http_header_entry *header, char *str);

#endif
