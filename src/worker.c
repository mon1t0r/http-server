#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "worker.h"
#include "http_request.h"
#include "http_response.h"
#include "request_handler.h"
#include "str_utils.h"

enum { recv_buf_size = 8192 };

enum recv_status {
    recv_continue,
    recv_stop_handle,
    recv_stop_error
};

enum send_status {
    send_ok,
    send_non_critical,
    send_fatal_error
};

static int hdr_parse(struct http_request *request, char *str)
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

static enum recv_status
data_recv(struct http_request *request, char *buf, int buf_len, int *buf_pos)
{
    int line_len;
    int parse_res;
    char *line;

    /* TODO: Receive request content */

    for(;;) {
        line_len = str_find_crlf(buf + *buf_pos, buf_len);
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
                return recv_stop_error;
            }
            continue;
        }

        /* If CRLFCRLF sequence found - end of the message header */
        if(line_len == 0) {
            return recv_stop_handle;
        }

        parse_res = hdr_parse(request, line);
        if(!parse_res) {
            return recv_stop_error;
        }
    }

    /* More data required */
    return recv_continue;
}

static enum send_status
res_send(int conn_fd, const struct http_response *response, char *buf,
         int buf_size)
{
    size_t status;

    status = http_response_write(response, buf, buf_size);
    if(status <= 0) {
        return send_non_critical;
    }

    status = send(conn_fd, buf, status, 0);
    if(status >= 0) {
        return send_ok;
    }

    perror("send()");
    return send_fatal_error;
}

int worker_run(int conn_fd, const struct sockaddr_in *addr)
{
    char *buf;
    int exit_code;

    struct http_request request;
    struct http_response response;

    int buf_pos;
    int buf_len;
    ssize_t data_len;
    enum recv_status recv_status;
    enum send_status send_status;
    int handle_status;

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

        recv_status = data_recv(&request, buf, buf_len, &buf_pos);
        if(recv_status == recv_stop_error) {
            goto exit;
        }

        /*
         * Continue receiving only when there is space in buffer left
         */
        if(recv_status == recv_continue && buf_len < recv_buf_size) {
            continue;
        }

        handle_status = handler_response_create(&response, &request, addr);
        send_status = res_send(conn_fd, &response, buf, recv_buf_size);
        if(!handle_status || send_status == send_fatal_error) {
            goto exit;
        }

        buf_len = buf_pos = 0;

        /* Reset request */
        http_remove_headers(&request.headers);
        memset(&request, 0, sizeof(request));

        /* Free response */
        handler_response_free(&response);
    }

exit:
    free(buf);
    close(conn_fd);
    return exit_code;
}
