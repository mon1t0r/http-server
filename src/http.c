#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "str_utils.h"

static enum http_method http_method_parse(const char *str)
{
    static const char *names[] = { LIST_HTTP_METHOD_STRING };

    int i;

    for(i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        if(0 == strcmp(str, names[i])) {
            return i;
        }
    }

    return http_extension;
}

static enum http_header_type http_header_type_parse(const char *str)
{
    static const char *names[] = { LIST_HTTP_HEADER_TYPE_STRING };

    int i;

    for(i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        if(0 == strcmp(str, names[i])) {
            return i;
        }
    }

    return http_header_unknown;
}

int http_request_line_parse(struct http_request_line *request_line, char *str)
{
    char *token;

    /* GET */
    token = strtok(str, " ");
    if(token == NULL) {
        return 0;
    }
    request_line->method = http_method_parse(token);

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

int http_header_parse(struct http_header_entry *header, char *str)
{
    char *token;

    /* Content-Length */
    token = strtok(str, ":");
    if(token == NULL) {
        return 0;
    }
    header->type = http_header_type_parse(str);

    /* 128 */
    token = strtok(NULL, "");
    if(token == NULL) {
        return 0;
    }
    header->value = token;

    return 1;
}

void http_request_add_header(struct http_request *request,
                             struct http_header_entry *header)
{
    struct http_header_entry *header_temp;

    header->next = NULL;

    if(request->headers == NULL) {
        request->headers = header;
        return;
    }

    header_temp = request->headers;
    while(header_temp->next != NULL) {
        header_temp = header_temp->next;
    }

    header_temp->next = header;
}
