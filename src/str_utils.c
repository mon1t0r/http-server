#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "str_utils.h"

static int str_find_dchar(const char *haystack, int len, const char needle[2])
{
    int i;

    for(i = 1; i < len - 1; i += 2) {
        if(haystack[i] == needle[0] && haystack[i + 1] == needle[1]) {
            return i;
        }

        if(haystack[i] == needle[1] && haystack[i - 1] == needle[0]) {
            return i - 1;
        }
    }

    return -1;
}

int str_find_crlf(const char *buf, int len)
{
    return str_find_dchar(buf, len, "\r\n");
}

int str_find_ddot(const char *buf, int len)
{
    return str_find_dchar(buf, len, "..");
}

int parse_uint(const char *str)
{
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

int str_arr_find(const char * const *arr, int arr_len, const char *str)
{
    int i;

    for(i = 0; i < arr_len; i++) {
        if(0 == strcmp(str, arr[i])) {
            return i;
        }
    }

    return -1;
}

int str_cmp_case(const char *str1, const char *str2)
{
    do {
        if(tolower(*str1) != tolower(*str2)) {
            return 1;
        }

        str1++;
        str2++;
    } while(*str1 != '\0' && *str2 != '\0');

    return *str1 != '\0' || *str2 != '\0';
}

void str_ltrim(char *str)
{
    int i;
    size_t len;

    i = 0;
    len = strlen(str);

    while(i < len && isspace(str[i])) {
        i++;
    }

    memmove(str, str + i, len - i + 1);
}
