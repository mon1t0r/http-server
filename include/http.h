#ifndef HTTP_SERVER_HTTP_H
#define HTTP_SERVER_HTTP_H

#define LIST_HTTP_METHOD_STRING \
    "GET",     \
    "HEAD",    \
    "POST",    \
    "PUT",     \
    "DELETE",  \
    "CONNECT", \
    "OPTIONS", \
    "TRACE"

enum http_method {
    http_get,
    http_head,
    http_post,
    http_put,
    http_delete,
    http_connect,
    http_options,
    http_trace,
    http_extension
};

#define LIST_HTTP_HEADER_TYPE_STRING \
    "Content-Length", \
    "User-Agent"

enum http_header_type {
    http_content_length,
    http_user_agent,
    http_header_unknown
};

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

void http_request_add_header(struct http_request *request,
                             struct http_header_entry *header);

#endif
