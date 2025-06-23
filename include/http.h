#ifndef HTTP_SERVER_HTTP_H
#define HTTP_SERVER_HTTP_H

enum http_method {
    http_get,
    http_head,
    http_post,
    http_put,
    http_delete,
    http_connect,
    http_options,
    http_trace,
    http_method_extension
};

#define LIST_HTTP_METHOD_STRING \
    "GET",     \
    "HEAD",    \
    "POST",    \
    "PUT",     \
    "DELETE",  \
    "CONNECT", \
    "OPTIONS", \
    "TRACE"

enum http_status {
    http_ok,
    http_bad_request,
    http_forbidden,
    http_not_found,
    http_service_unavailable,
    http_status_extension
};

#define LIST_HTTP_STATUS_STRING \
    "OK",                 \
    "Bad Request",        \
    "Forbidden",          \
    "Not Found",          \
    "Service Unavailable" \

#define LIST_HTTP_STATUS_CODE \
    200, \
    400, \
    403, \
    404, \
    503

enum http_header_type {
    http_date,
    http_server,
    http_content_length,
    http_content_type,
    http_user_agent,
    http_header_unknown
};

#define LIST_HTTP_HEADER_TYPE_STRING \
    "Date",           \
    "Server",         \
    "Content-Length", \
    "Content-Type",   \
    "User-Agent"

struct http_version {
    int major;
    int minor;
};

struct http_header_entry {
    enum http_header_type type;
    char *value;
    struct http_header_entry *next;
};

enum http_method http_method_parse(const char *str);

enum http_header_type http_header_type_parse(const char *str);

const char *http_header_type_str_get(enum http_header_type header_type);

const char *http_status_str_get(enum http_status status);

int http_status_code_get(enum http_status status);

void http_add_header(struct http_header_entry **headers,
                     const struct http_header_entry *header_src);

const struct http_header_entry *http_get_header(
    const struct http_header_entry **headers, enum http_header_type type);

void http_remove_headers(struct http_header_entry **headers);

#endif
