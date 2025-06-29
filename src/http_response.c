#include <stdio.h>
#include <string.h>

#include "http_response.h"
#include "http.h"

enum { int_buf_size = 32 };

static int
write_buf(const char *buf, size_t len, char **buf_pos, int *size_left)
{
    if(*size_left <= 0) {
        return 0;
    }

    if(*size_left < len) {
        return 0;
    }

    memcpy(*buf_pos, buf, len);

    *buf_pos += len;
    *size_left -= len;

    return 1;
}

static int write_str(const char *str, char **buf_pos, int *size_left)
{
    return write_buf(str, strlen(str), buf_pos, size_left);
}

static int write_num(int val, char **buf_pos, int *size_left)
{
    char buf_temp[int_buf_size];
    int status;

    status = sprintf(buf_temp, "%d", val);
    if(status <= 0) {
        return 0;
    }

    return write_str(buf_temp, buf_pos, size_left);
}

static int
write_status(const struct http_stat_line *status_line, char **buf_pos,
             int *size_left)
{
    int status;

    status = write_str("HTTP/", buf_pos, size_left);
    if(!status) {
        return 0;
    }

    status = write_num(status_line->ver.major, buf_pos, size_left);
    if(!status) {
        return 0;
    }

    status = write_str(".", buf_pos, size_left);
    if(!status) {
        return 0;
    }

    status = write_num(status_line->ver.minor, buf_pos, size_left);
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

static int
write_hdr(const struct http_hdr *header, char **buf_pos, int *size_left)
{
    int status;

    status = write_str(http_hdr_type_str_get(header->type), buf_pos,
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

int
http_res_write(const struct http_res *response, char *buf, int buf_size)
{
    char *buf_pos;
    int size_left;
    int status;
    struct http_hdr *header;

    buf_pos = buf;
    size_left = buf_size;

    status = write_status(&response->status_line, &buf_pos, &size_left);
    if(!status) {
        return 0;
    }

    header = response->hdrs;
    while(header != NULL) {
        status = write_hdr(header, &buf_pos, &size_left);
        if(!status) {
            return 0;
        }

        header = header->next;
    }

    status = write_str("\r\n", &buf_pos, &size_left);
    if(!status) {
        return 0;
    }

    if(response->content_type == http_cont_buffer) {
        status = write_buf(response->content.buf, response->content_len,
                           &buf_pos, &size_left);
        if(!status) {
            return 0;
        }
    }

    return buf_size - size_left;
}
