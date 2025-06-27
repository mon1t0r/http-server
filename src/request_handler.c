#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
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
    int_buf_size = 32,
    path_buf_size = 150,
    response_version_major = 1,
    response_version_minor = 1
};

static char *time_get_str(void)
{
    time_t time_cur;
    char *str;
    int len;

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

static off_t file_get_length(int fd)
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

static void response_setup(struct http_response *response)
{
    struct http_header_entry header;

    memset(response, 0, sizeof(*response));

    response->status_line.version.major = response_version_major;
    response->status_line.version.minor = response_version_minor;

    header.type = http_date;
    header.value = time_get_str();
    http_add_header(&response->headers, &header);

    header.type = http_server;
    header.value = SERVER_STRING;
    http_add_header(&response->headers, &header);
}

static void response_set_status(struct http_response *response,
                           enum http_status status)
{
    response->status_line.status = status;
    response->status_line.reason = (char *) http_status_str_get(status);
}

static void response_header_length_create(struct http_header_entry *header,
                                          const char *content)
{
    char *buf;
    int len;

    buf = malloc(int_buf_size);

    len = strlen(content);

    sprintf(buf, "%d", len);

    header->type = http_content_length;
    header->value = buf;
}

static enum http_status response_content_create(
    const struct http_request *request, char **buf_ptr)
{
    char path_buf[path_buf_size];
    int path_len;
    int uri_len;

    struct stat s;
    int stat_res;

    char *buf;
    int fd;
    off_t file_len;
    ssize_t read_len;

    path_len = strlen(CONTENT_PATH);
    uri_len = strlen(request->request_line.request_uri);

    if(path_len + uri_len >= path_buf_size) {
        return http_internal_error;
    }

    /* Do not allow to go up in file tree */
    if(str_find_ddot(request->request_line.request_uri, uri_len) >= 0) {
        return http_bad_request;
    }

    sprintf(path_buf, "%s%s", CONTENT_PATH, request->request_line.request_uri);

    stat_res = stat(path_buf, &s);
    if(stat_res < 0) {
        return http_internal_error;
    }
    if(S_ISDIR(s.st_mode)) {
        return http_not_found;
    }

    fd = open(path_buf, O_RDONLY);
    if(fd < 0) {
        if(errno == EACCES) {
            return http_not_found;
        }
        return http_internal_error;
    }

    file_len = file_get_length(fd);
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
    buf[file_len] = '\0';

    close(fd);

    *buf_ptr = buf;
    return http_ok;
}

static int response_create_get(struct http_response *response,
                               const struct http_request *request)
{
    struct http_header_entry header;
    enum http_status status;

    response_setup(response);

    status = response_content_create(request, &response->content);
    if(status != http_ok) {
        response_set_status(response, status);
        return 1;
    }

    response_set_status(response, http_ok);

    header.type = http_content_type;
    header.value = "text/html";
    http_add_header(&response->headers, &header);

    response_header_length_create(&header, response->content);
    http_add_header(&response->headers, &header);

    return 1;
}

static int response_create_head(struct http_response *response,
                                const struct http_request *request)
{
    int status;

    status = response_create_get(response, request);

    if(response->content != NULL) {
        free(response->content);
        response->content = NULL;
    }

    return status;
}

int handler_response_create(struct http_response *response,
                            const struct http_request *request,
                            const struct sockaddr_in *addr)
{
    switch(request->request_line.method) {
        case http_get:
            return response_create_get(response, request);
        case http_head:
            return response_create_head(response, request);
        default:
            return 0;
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

    if(response->content != NULL) {
        free(response->content);
    }
}
