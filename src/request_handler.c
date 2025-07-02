#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "request_handler.h"
#include "http.h"
#include "http_response.h"
#include "server_config.h"
#include "str_utils.h"

#define HEADER_KEEP_ALIVE "Keep-Alive"

enum {
    hdr_ka_buf_size    = 70,
    int_buf_size       = 32,
    path_buf_size      = 150,
    ver_major          = 1,
    ver_minor          = 1
};

static char *time_get_str(void)
{
    time_t time_cur;
    char *str;
    size_t len;

    time_cur = time(NULL);

    /* We must not use this date format according to RFC9110,
     * but implementations must understand it, so we use it for simplicity
     * See RFC 9110 section 5.6.7 */

    /* (!) Static buffer is used here */
    str = ctime(&time_cur);

    len = strlen(str);
    /* Replace \n with \0 */
    str[len - 1] = '\0';

    return str;
}

static off_t file_get_len(int fd)
{
    off_t length, res;

    length = lseek(fd, 0, SEEK_END);
    if(length < 0) {
        return -1;
    }

    res = lseek(fd, 0, SEEK_SET);
    if(res < 0) {
        return -1;
    }

    return length;
}

static enum http_status
res_cont_creat(struct http_res *response, const char *uri)
{
    char path_buf[path_buf_size];
    size_t path_len;
    size_t uri_len;

    struct stat s;
    int stat_res;

    int fd;

    path_len = strlen(CONTENT_PATH);
    uri_len = strlen(uri);

    if(path_len + uri_len >= path_buf_size) {
        return http_uri_too_large;
    }

    /* Do not allow to go up in file tree */
    if(str_find_ddot(uri, uri_len) >= 0) {
        return http_bad_request;
    }

    sprintf(path_buf, "%s%s", CONTENT_PATH, uri);

    stat_res = stat(path_buf, &s);
    if(stat_res < 0) {
        if(errno == ENAMETOOLONG) {
            return http_uri_too_large;
        }
        if(errno == EACCES || errno == ENOENT || errno == ENOTDIR) {
            return http_not_found;
        }
        return http_internal_error;
    }
    if(S_ISDIR(s.st_mode)) {
        return http_not_found;
    }

    fd = open(path_buf, O_RDONLY);
    if(fd < 0) {
        if(errno == ENAMETOOLONG) {
            return http_uri_too_large;
        }
        if(errno == EACCES || errno == ENOENT) {
            return http_not_found;
        }
        return http_internal_error;
    }

    response->content.fd = fd;
    response->content_type = http_cont_file;
    response->content_len = file_get_len(fd);

    return http_ok;
}

static void res_cont_free(struct http_res *response)
{
    switch(response->content_type) {
        case http_cont_buffer:
            free(response->content.buf);
            break;

        case http_cont_file:
            close(response->content.fd);
            break;

        case http_cont_none:
            return;
    }

    response->content_type = http_cont_none;
}

static void res_setup(struct http_res *response)
{
    struct http_hdr header;

    memset(response, 0, sizeof(*response));

    response->status_line.ver.major = ver_major;
    response->status_line.ver.minor = ver_minor;

    header.type = http_date;
    header.value = time_get_str();
    http_hdr_add(&response->hdrs, &header);

    header.type = http_server;
    header.value = SERVER_STRING;
    http_hdr_add(&response->hdrs, &header);
}

static void res_set_status(struct http_res *response,
                           enum http_status status)
{
    response->status_line.status = status;
    response->status_line.reason = (char *) http_status_str_get(status);
}

static void
res_hdr_keep_alive_creat(struct http_hdr *header)
{
    char *buf;

    buf = malloc(hdr_ka_buf_size);

    sprintf(buf, "timeout=%d", conf_keep_alive_timeout);

    header->type = http_keep_alive;
    header->value = buf;
}

static void
res_hdr_cont_len_creat(struct http_hdr *header, int content_len)
{
    char *buf;

    buf = malloc(int_buf_size);

    sprintf(buf, "%d", content_len);

    header->type = http_content_length;
    header->value = buf;
}

static enum http_status
res_hdr_cont_t_creat(struct http_hdr *header, const char *uri)
{
    char path_buf[path_buf_size];
    size_t uri_len;
    const char *filename;
    const char *file_ext;

    header->type = http_content_type;

    uri_len = strlen(uri);
    if(uri_len >= path_buf_size) {
        return http_uri_too_large;
    }

    strcpy(path_buf, uri);

    filename = basename(path_buf);
    file_ext = strrchr(filename, '.');

    /* header->value will not be modified for this header */
    header->value = (char *) http_cont_type_str_get(file_ext);

    return http_ok;
}

static void
res_creat_get(struct http_res *response, const struct http_req *request)
{
    struct http_hdr header;
    enum http_status status;

    /* Create content */
    status = res_cont_creat(response, request->req_line.uri);
    if(status != http_ok) {
        goto exit;
    }

    /* Create Content-Type header */
    status = res_hdr_cont_t_creat(&header, request->req_line.uri);
    if(status != http_ok) {
        res_cont_free(response);
        goto exit;
    }
    http_hdr_add(&response->hdrs, &header);

    status = http_ok;

exit:
    /* Create Content-Length header */
    res_hdr_cont_len_creat(&header, response->content_len);
    http_hdr_add(&response->hdrs, &header);

    /* Set response status */
    res_set_status(response, status);
}

static void
res_creat_head(struct http_res *response, const struct http_req *request)
{
    res_creat_get(response, request);
    res_cont_free(response);
}

static enum http_status req_check(const struct http_req *request)
{
    const struct http_hdr *header;

    if(request->req_line.ver.major != ver_major ||
        request->req_line.ver.minor != ver_minor) {
        return http_version_unsupported;
    }

    /* If no User-Agent specified, return 403 Forbidden */
    header = http_hdr_get(request->hdrs, http_user_agent);
    if(header == NULL) {
        return http_forbidden;
    }

    return http_ok;
}

static int req_con_keep_alive(const struct http_req *request)
{
    const struct http_hdr *header;

    header = http_hdr_get(request->hdrs, http_connection);
    if(header == NULL) {
        return 1;
    }

    return 0 == str_cmp_case(header->value, HEADER_KEEP_ALIVE);
}

int handler_res_creat(struct http_res *response,
                      const struct http_req *request)
{
    int conn_keep_alive;
    enum http_status status;
    struct http_hdr header;

    /* Check if connection should be kept alive */
    conn_keep_alive = req_con_keep_alive(request);

    /* Setup response */
    res_setup(response);

    /* Check request */
    status = req_check(request);
    if(status != http_ok) {
        res_set_status(response, status);
        goto exit;
    }

    switch(request->req_line.method) {
        case http_get:
            res_creat_get(response, request);
            break;
        case http_head:
            res_creat_head(response, request);
            break;
        default:
            res_set_status(response, http_not_implemented);
            break;
    }

    if(conn_keep_alive) {
        /* Create Connection header */
        header.type = http_connection;
        header.value = HEADER_KEEP_ALIVE;
        http_hdr_add(&response->hdrs, &header);

        /* Create Keep-Alive header */
        res_hdr_keep_alive_creat(&header);
        http_hdr_add(&response->hdrs, &header);
    }

exit:
    return conn_keep_alive;
}

void handler_res_free(struct http_res *response)
{
    struct http_hdr *header;

    header = response->hdrs;
    while(header != NULL) {
        if(header->type == http_content_length ||
            header->type == http_keep_alive) {
            free(header->value);
        }
        header = header->next;
    }

    http_hdrs_remove(&response->hdrs);

    res_cont_free(response);
}
