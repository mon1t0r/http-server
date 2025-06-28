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
#include "http_request.h"
#include "http_response.h"
#include "str_utils.h"

#define SERVER_STRING "TestServer/1.0"
#define CONTENT_PATH "./html"

enum {
    int_buf_size  = 32,
    path_buf_size = 150,
    res_ver_major = 1,
    res_ver_minor = 1
};

static char *time_get_str(void)
{
    time_t time_cur;
    char *str;
    size_t len;

    time_cur = time(NULL);

    /* We must not use this date format according to RFC2616, */
    /* but implementations must understand it, so we use it for simplicity */
    /* See RFC 2616 section 3.3.1 */

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
res_cont_creat(char **buf_ptr, int *buf_len_ptr, const char *uri)
{
    char path_buf[path_buf_size];
    size_t path_len;
    size_t uri_len;

    struct stat s;
    int stat_res;

    char *buf;
    int fd;
    off_t file_len;
    ssize_t read_len;

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

    file_len = file_get_len(fd);
    if(file_len < 0) {
        return http_internal_error;
    }

    /* +1 byte for terminator */
    buf = malloc(file_len + 1);

    read_len = read(fd, buf, file_len);
    if(read_len != file_len) {
        free(buf);
        close(fd);
        return http_internal_error;
    }

    close(fd);

    *buf_ptr = buf;
    *buf_len_ptr = read_len;
    return http_ok;
}

static void res_cont_free(char **buf_ptr)
{
    if(*buf_ptr != NULL) {
        free(*buf_ptr);
        *buf_ptr = NULL;
    }
}

static void res_setup(struct http_response *response)
{
    struct http_header_entry header;

    memset(response, 0, sizeof(*response));

    response->status_line.version.major = res_ver_major;
    response->status_line.version.minor = res_ver_minor;

    header.type = http_date;
    header.value = time_get_str();
    http_add_header(&response->headers, &header);

    header.type = http_server;
    header.value = SERVER_STRING;
    http_add_header(&response->headers, &header);
}

static void res_set_status(struct http_response *response,
                           enum http_status status)
{
    response->status_line.status = status;
    response->status_line.reason = (char *) http_status_str_get(status);
}

static void
res_hdr_cont_len_creat(struct http_header_entry *header, int content_len)
{
    char *buf;

    buf = malloc(int_buf_size);

    sprintf(buf, "%d", content_len);

    header->type = http_content_length;
    header->value = buf;
}

static enum http_status
res_hdr_cont_t_creat(struct http_header_entry *header, const char *uri)
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

static int res_creat_get(struct http_response *response,
                         const struct http_request *request)
{
    struct http_header_entry header;
    enum http_status status;

    /* Setup */
    res_setup(response);

    /* Create content */
    status = res_cont_creat(&response->content,
                            &response->content_len,
                            request->request_line.request_uri);
    if(status != http_ok) {
        goto exit;
    }

    /* Create Content-Type header */
    status = res_hdr_cont_t_creat(&header, request->request_line.request_uri);
    if(status != http_ok) {
        res_cont_free(&response->content);
        goto exit;
    }
    http_add_header(&response->headers, &header);

    status = http_ok;

exit:
    /* Set response status */
    res_set_status(response, status);

    /* Create Content-Length header */
    res_hdr_cont_len_creat(&header, response->content_len);
    http_add_header(&response->headers, &header);

    return 1;
}

static int res_creat_head(struct http_response *response,
                          const struct http_request *request)
{
    int status;

    status = res_creat_get(response, request);

    res_cont_free(&response->content);

    return status;
}

static int res_creat_default(struct http_response *response)
{
    res_setup(response);

    res_set_status(response, http_not_implemented);

    return 1;
}

int handler_response_create(struct http_response *response,
                            const struct http_request *request,
                            const struct sockaddr_in *addr)
{
    /* Only HTTP/1.1 is supported */
    if(request->request_line.version.major != 1 ||
        request->request_line.version.minor != 1) {
        res_setup(response);
        res_set_status(response, http_version_unsupported);
        return 1;
    }

    switch(request->request_line.method) {
        case http_get:
            return res_creat_get(response, request);
        case http_head:
            return res_creat_head(response, request);
        default:
            return res_creat_default(response);
    }
}

void handler_response_free(struct http_response *response)
{
    struct http_header_entry *header;

    header = response->headers;
    while(header != NULL) {
        if(header->type == http_content_length) {
            free(header->value);
        }
        header = header->next;
    }

    http_remove_headers(&response->headers);

    res_cont_free(&response->content);
}
