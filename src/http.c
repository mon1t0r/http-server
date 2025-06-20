#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "str_utils.h"

static enum http_method parse_http_method(const char *str) {
    /* Should be in the same order as enum http_method */
    static const char *method_names[] = {
        "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE"
    };

    int i;

    for(i = 0; i < sizeof(method_names) / sizeof(method_names[0]); i++) {
        if(0 == strcmp(str, method_names[i])) {
            return i;
        }
    }

    return http_extension;
}

int fill_http_request_line(struct http_request_line *request_line, char *str)
{
    char *token;

    /* GET */
    token = strtok(str, " ");
    if(token == NULL) {
        return 0;
    }
    request_line->method = parse_http_method(token);

    /* /index.html */
    token = strtok(NULL, " ");
    if(token == NULL) {
        return 0;
    }
    request_line->request_uri = token;

    /* HTTP */
    token = strtok(NULL, "/");
    if(token == NULL) {
        return 0;
    }

    /* 1 */
    token = strtok(NULL, ".");
    if(token == NULL) {
        return 0;
    }
    request_line->version_major = parse_uint(token);
    if(request_line->version_major < 0) {
        return 0;
    }

    /* 1 */
    token = strtok(NULL, "");
    if(token == NULL) {
        return 0;
    }
    request_line->version_minor = parse_uint(token);
    if(request_line->version_minor < 0) {
        return 0;
    }

    return 1;
}
