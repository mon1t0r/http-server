#include <stdio.h>
#include <string.h>

#include "http_response.h"
#include "http.h"

static int write_str(const char *str, char **buf_pos, int *size_left)
{
    int write_cnt;

    if(*size_left <= 0) {
        return 0;
    }

    strncpy(*buf_pos, str, *size_left);

    /* In case string is truncated */
    (*buf_pos)[*size_left - 1] = '\0';

    write_cnt = strlen(*buf_pos);
    if(write_cnt <= 0) {
        return 0;
    }

    *buf_pos += write_cnt;
    *size_left -= write_cnt;

    return 1;
}

static int write_num(int val, char **buf_pos, int *size_left)
{
    char buf_temp[32];
    int status;

    status = sprintf(buf_temp, "%d", val);
    if(status <= 0) {
        return 0;
    }

    return write_str(buf_temp, buf_pos, size_left);
}

static int write_status_line(const struct http_status_line *status_line,
                             char **buf_pos, int *size_left)
{
    int status;

    status = write_str("HTTP/", buf_pos, size_left);
    if(!status) {
        return 0;
    }

    status = write_num(status_line->version.major, buf_pos, size_left);
    if(!status) {
        return 0;
    }

    status = write_str(".", buf_pos, size_left);
    if(!status) {
        return 0;
    }

    status = write_num(status_line->version.minor, buf_pos, size_left);
    if(!status) {
        return 0;
    }

    status = write_str(" ", buf_pos, size_left);
    if(!status) {
        return 0;
    }

    status = write_num(http_status_code_get(status_line->status), buf_pos,
                       size_left);
    if(!status) {
        return 0;
    }

    status = write_str(" ", buf_pos, size_left);
    if(!status) {
        return 0;
    }

    status = write_str(status_line->reason, buf_pos, size_left);
    if(!status) {
        return 0;
    }

    status = write_str("\r\n", buf_pos, size_left);
    if(!status) {
        return 0;
    }

    return 1;
}

static int write_header(const struct http_header_entry *header,
                        char **buf_pos, int *size_left)
{
    int status;

    status = write_str(http_header_type_str_get(header->type), buf_pos,
                       size_left);
    if(!status) {
        return 0;
    }

    status = write_str(": ", buf_pos, size_left);
    if(!status) {
        return 0;
    }

    status = write_str(header->value, buf_pos, size_left);
    if(!status) {
        return 0;
    }

    status = write_str("\r\n", buf_pos, size_left);
    if(!status) {
        return 0;
    }

    return 1;
}

int http_response_write(const struct http_response *response, char *buf,
                        int buf_size)
{
    char *buf_pos;
    int size_left;
    int status;
    struct http_header_entry *header;

    buf_pos = buf;
    size_left = buf_size;

    status = write_status_line(&response->status_line, &buf_pos, &size_left);
    if(!status) {
        return 0;
    }

    header = response->headers;
    while(header != NULL) {
        status = write_header(header, &buf_pos, &size_left);
        if(!status) {
            return 0;
        }

        header = header->next;
    }

    status = write_str("\r\n", &buf_pos, &size_left);
    if(!status) {
        return 0;
    }

    return buf_size - size_left;
}
