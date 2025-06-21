#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "str_utils.h"
#include "worker.h"
#include "http.h"
#include "request_handler.h"

enum {
    recv_buf_size = 8192
};

static int header_parse(struct http_request *request, char *str)
{
    struct http_header_entry header;
    int parse_res;

    parse_res = http_header_parse(&header, str);
    if(!parse_res) {
        return 0;
    }

    /* Unknown header type - ignore */
    if(header.type == http_header_unknown) {
        return 1;
    }

    http_add_header(&request->headers, &header);

    return 1;
}

static int receive_data(struct http_request *request, char *buf,
                       int buf_len, int *buf_pos)
{
    int line_len;
    int parse_res;
    char *line;

    for(;;) {
        line_len = find_crlf(buf + *buf_pos, buf_len);
        if(line_len < 0) {
            break;
        }

        line = buf + *buf_pos;
        *buf_pos += line_len + 2;

        line[line_len] = '\0';

        /* *buf_pos == 0, beginning of the data */
        if(buf == line) {
            parse_res = http_request_line_parse(&request->request_line, line);
            if(!parse_res) {
                /* Request line parsing failed - error */
                return -1;
            }
            continue;
        }

        /* If CRLFCRLF sequence found - end of the header */
        if(line_len == 0) {
            return 1;
        }

        parse_res = header_parse(request, line);
        if(!parse_res) {
            return -1;
        }
    }

    /* More data required */
    return 0;
}


int worker_run(int conn_fd, const struct sockaddr_in *addr)
{
    char *buf;
    int buf_len;
    int exit_code;

    struct http_request request;

    int buf_pos;
    int data_len;
    int handle_res;

    exit_code = EXIT_FAILURE;

    buf = malloc(recv_buf_size);
    if(!buf) {
        perror("malloc()");
        goto exit;
    }

    buf_len = buf_pos = 0;
    memset(&request, 0, sizeof(request));

    for(;;) {
        data_len = recv(conn_fd, buf + buf_len, recv_buf_size - buf_len, 0);
        if(data_len < 0) {
            perror("recv()");
            goto exit;
        }
        if(data_len == 0) {
            exit_code = EXIT_SUCCESS;
            goto exit;
        }

        buf_len += data_len;

        handle_res = receive_data(&request, buf, buf_len, &buf_pos);

        /*
         * handle_res == 0  : continue receiving data
         * handle_res == 1  : stop receiving and attempt to handle request
         * handle_res == -1 : stop receiving wihtout attempting to handle
         *
         * Continue receiving only when there is space in buffer left
         */
        if(handle_res == 0 && buf_len < recv_buf_size) {
            continue;
        }

        if(handle_res > 0) {
            handler_handle_request(&request, addr);
        }

        buf_len = buf_pos = 0;
        http_remove_headers(&request.headers);
        memset(&request, 0, sizeof(request));
    }

exit:
    free(buf);
    close(conn_fd);
    return exit_code;
}
