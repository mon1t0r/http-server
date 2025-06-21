#include "http.h"
#include "str_utils.h"

enum http_method http_method_parse(const char *str)
{
    static const char *names[] = { LIST_HTTP_METHOD_STRING };

    int index;

    index = find_str(names, sizeof(names) / sizeof(names[0]), str);
    if(index < 0) {
        return http_extension;
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

