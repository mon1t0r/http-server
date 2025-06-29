#include <string.h>

#include "http_request.h"
#include "str_utils.h"

int http_req_line_parse(struct http_req_line *request_line, char *str)
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
    request_line->uri = token;

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
    request_line->ver.major = parse_uint(token);
    if(request_line->ver.major < 0) {
        return 0;
    }

    /* 1 */
    token = strtok(NULL, "");
    if(token == NULL) {
        return 0;
    }
    request_line->ver.minor = parse_uint(token);
    if(request_line->ver.minor < 0) {
        return 0;
    }

    return 1;
}

int http_hdr_parse(struct http_hdr *header, char *str)
{
    char *token;

    /* Content-Length */
    token = strtok(str, ":");
    if(token == NULL) {
        return 0;
    }
    header->type = http_hdr_type_parse(str);

    /* 128 */
    token = strtok(NULL, "");
    if(token == NULL) {
        return 0;
    }
    header->value = token;
    str_ltrim(header->value);

    return 1;
}
