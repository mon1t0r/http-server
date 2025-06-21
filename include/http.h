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

enum http_method http_method_parse(const char *str);

enum http_header_type http_header_type_parse(const char *str);

#endif
