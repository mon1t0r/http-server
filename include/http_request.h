#ifndef HTTP_SERVER_HTTP_REQUEST_H
#define HTTP_SERVER_HTTP_REQUEST_H

#include "http.h"

struct http_request_line {
    enum http_method method;
    char *request_uri;
    int version_major;
    int version_minor;
};

struct http_header_entry {
    enum http_header_type type;
    char *value;
    struct http_header_entry *next;
};

struct http_request {
    struct http_request_line request_line;
    struct http_header_entry *headers;
    char *content;
};

int http_request_line_parse(struct http_request_line *request_line, char *str);

int http_header_parse(struct http_header_entry *header, char *str);

void http_request_empty(struct http_request *request);

void http_request_add_header(struct http_request *request,
                             const struct http_header_entry *header_src);

void http_request_remove_headers(struct http_request *request);

#endif
