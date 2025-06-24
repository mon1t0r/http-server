#ifndef HTTP_SERVER_STR_UTILS_H
#define HTTP_SERVER_STR_UTILS_H

int str_find_crlf(const char *buf, int len);

int str_find_ddot(const char *buf, int len);

int parse_uint(const char *str);

int str_arr_find(const char * const *arr, int arr_len, const char *str);

#endif
