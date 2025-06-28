#ifndef HTTP_SERVER_HTTP_H
#define HTTP_SERVER_HTTP_H

#include "enum_def.h"

#define ENUM_HTTP_METHOD(type) \
    ENUM_VALUE2(type, http_get,              "GET"     ), \
    ENUM_VALUE2(type, http_head,             "HEAD"    ), \
    ENUM_VALUE2(type, http_post,             "POST"    ), \
    ENUM_VALUE2(type, http_put,              "PUT"     ), \
    ENUM_VALUE2(type, http_delete,           "DELETE"  ), \
    ENUM_VALUE2(type, http_connect,          "CONNECT" ), \
    ENUM_VALUE2(type, http_options,          "OPTIONS" ), \
    ENUM_VALUE2(type, http_trace,            "TRACE"   ), \
    ENUM_VALUE2(type, http_method_extension, ""        )

#define ENUM_HTTP_STATUS(type) \
    ENUM_VALUE3(type, http_ok,                  200, "OK"                  ), \
    ENUM_VALUE3(type, http_bad_request,         400, "Bad Request"         ), \
    ENUM_VALUE3(type, http_not_found,           404, "Not Found"           ), \
    ENUM_VALUE3(type, http_uri_too_large,       414, "Request-URI Too Large"),\
    ENUM_VALUE3(type, http_internal_error,      500, "Internal Server Error"),\
    ENUM_VALUE3(type, http_not_implemented,     501, "Not Implemented"     ), \
    ENUM_VALUE3(type, http_version_unsupported, 505, \
                "HTTP Version not supported"      ), \
    ENUM_VALUE3(type, http_status_extension,    0,   ""                    )

#define ENUM_HTTP_HEADER_TYPE(type) \
    ENUM_VALUE2(type, http_date,           "Date"           ), \
    ENUM_VALUE2(type, http_server,         "Server"         ), \
    ENUM_VALUE2(type, http_content_length, "Content-Length" ), \
    ENUM_VALUE2(type, http_content_type,   "Content-Type"   ), \
    ENUM_VALUE2(type, http_user_agent,     "User-Agent"     ), \
    ENUM_VALUE2(type, http_header_unknown, ""               )

#define ENUM_HTTP_CONTENT_TYPE(type) \
    ENUM_VALUE2(type, "",     "text/plain"   ), \
    ENUM_VALUE2(type, "html", "text/html"    ), \
    ENUM_VALUE2(type, "png",  "image/png"    ), \
    ENUM_VALUE2(type, "ico",  "image/x-icon" )

enum http_method {
    ENUM_HTTP_METHOD(ENUM_TYPE_VAL1)
};

enum http_status {
    ENUM_HTTP_STATUS(ENUM_TYPE_VAL1)
};

enum http_header_type {
    ENUM_HTTP_HEADER_TYPE(ENUM_TYPE_VAL1)
};

struct http_version {
    int major;
    int minor;
};

struct http_header_entry {
    enum http_header_type type;
    /* Value is not const because some headers assign pointer to a
     * dynamically allocated memory region here, which needs to be freed */
    char *value;
    struct http_header_entry *next;
};

enum http_method http_method_parse(const char *str);

enum http_header_type http_header_type_parse(const char *str);

const char *http_header_type_str_get(enum http_header_type header_type);

const char *http_status_str_get(enum http_status status);

int http_status_code_get(enum http_status status);

const char *http_cont_type_str_get(const char *file_ext);

void http_add_header(struct http_header_entry **headers,
                     const struct http_header_entry *header_src);

const struct http_header_entry *http_get_header(
    const struct http_header_entry **headers, enum http_header_type type);

void http_remove_headers(struct http_header_entry **headers);

#endif
