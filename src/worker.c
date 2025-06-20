#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "worker.h"
#include "http.h"
#include "str_utils.h"

enum {
    recv_buf_size = 8192
};

static int handle_data(struct http_request *request, char *buf,
                       int buf_len, int *buf_pos)
{
    int line_len;

    while((line_len = find_crlf(buf + *buf_pos, buf_len)) >= 0) {
        buf[*buf_pos + line_len] = '\0';

        if(*buf_pos == 0) {
            if(!fill_http_request_line(&request->request_line, buf)) {
                /* Request line parsing failed - error */
                return -1;
            }
            *buf_pos += line_len + 2;
            continue;
        }

        /* If CRLFCRLF sequence found - end of the header */
        if(line_len == 0) {
            return 1;
        }

        /* TODO: Parse header fields */

        *buf_pos += line_len + 2;
    }

    /* More data required */
    return 0;
}

static void handle_request(const struct http_request *request)
{
    printf("Received HTTP request: %d %s %d.%d\n", request->request_line.method,
           request->request_line.request_uri,
           request->request_line.version_major,
           request->request_line.version_minor);
}

int worker_run(int conn_fd, const struct sockaddr_in *addr)
{
    char *buf;
    int buf_len;

    struct http_request request;

    int buf_pos;
    int data_len;
    int handle_res;

    buf = malloc(recv_buf_size);
    buf_len = buf_pos = 0;
    memset(&request, 0, sizeof(request));

    for(;;) {
        data_len = recv(conn_fd, buf + buf_len, recv_buf_size - buf_len, 0);
        if(data_len < 0) {
            perror("recv()");
            goto exit;
        }

        buf_len += data_len;

        if(data_len != 0) {
            handle_res = handle_data(&request, buf, buf_len, &buf_pos);
        } else {
            handle_res = 1;
        }

        /*
         * handle_res == 0  : continue receiving data
         * handle_res == 1  : stop receiving and attempt to handle request
         * handle_res == -1 : stop receiving wihtout attempting to handle
         */
        if(handle_res == 0) {
            continue;
        }

        if(handle_res > 0) {
            handle_request(&request);
        }

        buf_len = buf_pos = 0;
        memset(&request, 0, sizeof(request));
    }


exit:
    free(buf);
    close(conn_fd);
    return EXIT_FAILURE;
}
