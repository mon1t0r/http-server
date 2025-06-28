#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "str_utils.h"

enum http_method http_method_parse(const char *str)
{
    static const char *names[] = { ENUM_HTTP_METHOD(ENUM_TYPE_VAL2) };

    int index;

    index = str_arr_find(names, sizeof(names) / sizeof(names[0]), str);
    if(index < 0) {
        return http_method_extension;
    }

    return index;
}

enum http_header_type http_header_type_parse(const char *str)
{
    static const char *names[] = { ENUM_HTTP_HEADER_TYPE(ENUM_TYPE_VAL2) };

    int index;

    index = str_arr_find(names, sizeof(names) / sizeof(names[0]), str);
    if(index < 0) {
        return http_header_unknown;
    }

    return index;
}

const char *http_header_type_str_get(enum http_header_type header_type)
{
    static const char *names[] = { ENUM_HTTP_HEADER_TYPE(ENUM_TYPE_VAL2) };

    return names[header_type];
}

const char *http_status_str_get(enum http_status status)
{
    static const char *names[] = { ENUM_HTTP_STATUS(ENUM_TYPE_VAL3) };

    return names[status];
}

int http_status_code_get(enum http_status status)
{
    static int codes[] = { ENUM_HTTP_STATUS(ENUM_TYPE_VAL2) };

    return codes[status];
}

const char *http_cont_type_str_get(const char *file_ext)
{
    static const char *exts[] = { ENUM_HTTP_CONTENT_TYPE(ENUM_TYPE_VAL1) };
    static const char *types[] = { ENUM_HTTP_CONTENT_TYPE(ENUM_TYPE_VAL2) };

    int index;

    if(file_ext == NULL || file_ext[0] != '.') {
        return types[0];
    }

    /* file_ext + 1 to omit period (.html -> html)*/
    index = str_arr_find(exts, sizeof(exts) / sizeof(exts[0]), file_ext + 1);
    if(index < 0) {
        return types[0];
    }

    return types[index];
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

const struct http_header_entry *
http_get_header(const struct http_header_entry *headers_first,
                enum http_header_type type)
{
    const struct http_header_entry *header;

    header = headers_first;
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
