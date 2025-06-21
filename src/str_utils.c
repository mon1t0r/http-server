#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "str_utils.h"

int find_crlf(const char *str, int len)
{
    int i;

    for(i = 1; i < len - 1; i += 2) {
        if(str[i] == '\r' && str[i + 1] == '\n') {
            return i;
        }

        if(str[i] == '\n' && str[i - 1] == '\r') {
            return i - 1;
        }
    }

    return -1;
}


int parse_uint(const char *str) {
    long res;

    if(str[0] == '\0') {
        return -1;
    }

    errno = 0;
    res = strtol(str, NULL, 10);
    if(errno != 0) {
        return -1;
    }

    if(res > INT_MAX || res < 0) {
        return -1;
    }

    return res;
}

int find_str(const char * const *arr, int arr_len, const char *str)
{
    int i;

    for(i = 0; i < arr_len; i++) {
        if(0 == strcmp(str, arr[i])) {
            return i;
        }
    }

    return -1;
}
