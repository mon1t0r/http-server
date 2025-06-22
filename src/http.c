#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "str_utils.h"

enum http_method http_method_parse(const char *str)
{
    static const char *names[] = { LIST_HTTP_METHOD_STRING };

    int index;

    index = find_str(names, sizeof(names) / sizeof(names[0]), str);
    if(index < 0) {
        return http_method_extension;
    }

    return index;
}

enum http_header_type http_header_type_parse(const char *str)
{
    static const char *names[] = { LIST_HTTP_HEADER_TYPE_STRING };

    int index;

    index = find_str(names, sizeof(names) / sizeof(names[0]), str);
    if(index < 0) {
        return http_header_unknown;
    }

    return index;
}

const char *http_header_type_str_get(enum http_header_type header_type)
{
    static const char *names[] = { LIST_HTTP_HEADER_TYPE_STRING };

    return names[header_type];
}

const char *http_status_str_get(enum http_status status)
{
    static const char *names[] = { LIST_HTTP_STATUS_STRING };

    return names[status];
}

int http_status_code_get(enum http_status status)
{
    static int codes[] = { LIST_HTTP_STATUS_CODE };

    return codes[status];
}

void http_add_header(struct http_header_entry **headers,
                     const struct http_header_entry *header_src)
{
    struct http_header_entry *header;
    struct http_header_entry *header_temp;

    header = malloc(sizeof(struct http_header_entry));
    if(!header) {
        return;
    }

    memcpy(header, header_src, sizeof(struct http_header_entry));
    header->next = NULL;

    if(*headers == NULL) {
        *headers = header;
        return;
    }

    header_temp = *headers;
    while(header_temp->next != NULL) {
        header_temp = header_temp->next;
    }

    header_temp->next = header;
}

const struct http_header_entry *http_get_header(
    const struct http_header_entry **headers, enum http_header_type type)
{
    const struct http_header_entry *header;

    header = *headers;
    while(header != NULL) {
        if(header->type == type) {
            return header;
        }
        header = header->next;
    }

    return NULL;
}

void http_remove_headers(struct http_header_entry **headers)
{
    struct http_header_entry *header_temp;

    while(*headers != NULL) {
        header_temp = (*headers)->next;

        free(*headers);

        *headers = header_temp;
    }

    *headers = NULL;
}
