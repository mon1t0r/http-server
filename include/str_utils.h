#ifndef HTTP_SERVER_STR_UTILS_H
#define HTTP_SERVER_STR_UTILS_H

int find_crlf(const char *str, int len);

int parse_uint(const char *str);

int find_str(const char * const *arr, int arr_len, const char *str);

#endif
