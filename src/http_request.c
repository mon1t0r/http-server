#include <string.h>

#include "http_request.h"
#include "str_utils.h"

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
    request_line->version.major = parse_uint(token);
    if(request_line->version.major < 0) {
        return 0;
    }

    /* 1 */
    token = strtok(NULL, "");
    if(token == NULL) {
        return 0;
    }
    request_line->version.minor = parse_uint(token);
    if(request_line->version.minor < 0) {
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
