#ifndef HTTP_SERVER_HTTP_H
#define HTTP_SERVER_HTTP_H

enum http_method {
    http_get = 0,
    http_head,
    http_post,
    http_put,
    http_delete,
    http_connect,
    http_options,
    http_trace,
    http_extension
};

struct http_request_line {
    enum http_method method;
    char *request_uri;
    int version_major;
    int version_minor;
};

struct http_request {
    struct http_request_line request_line;
    /* TODO: Header fields */
    char *content;
};

int fill_http_request_line(struct http_request_line *request_line, char *str);

#endif
