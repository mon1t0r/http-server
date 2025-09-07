#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "worker.h"
#include "http_request.h"
#include "http_response.h"
#include "request_handler.h"
#include "server_config.h"
#include "str_utils.h"

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

static int hdr_parse(struct http_req *request, char *str)
{
    struct http_hdr header;
    int parse_res;

    parse_res = http_hdr_parse(&header, str);
    if(!parse_res) {
        return 0;
    }

    /* Unknown header type - ignore */
    if(header.type == http_header_unknown) {
        return 1;
    }

    http_hdr_add(&request->hdrs, &header);

    return 1;
}

static enum recv_status
data_recv(struct http_req *request, char *buf, int buf_len, int *buf_pos)
{
    int line_len;
    int parse_res;
    char *line;

    /* Request content may be received here */

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
            parse_res = http_req_line_parse(&request->req_line, line);
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
res_send(int conn_fd, const struct http_res *response, char *buf, int buf_size)
{
    int bytes_left, bytes_sent, bytes_sent_now;

    bytes_left = http_res_write(response, buf, buf_size);
    if(bytes_left <= 0) {
        return send_non_critical;
    }

    bytes_sent = 0;
    while(bytes_left > 0) {
        bytes_sent_now = send(conn_fd, buf + bytes_sent, bytes_left, 0);
        if(bytes_sent_now < 0) {
            perror("send()");
            return send_fatal_error;
        }

        bytes_sent += bytes_sent_now;
        bytes_left -= bytes_sent_now;
    }

    if(response->content_type != http_cont_file) {
        return send_ok;
    }

    while((bytes_left = read(response->content.fd, buf, buf_size)) > 0) {
        bytes_sent_now = send(conn_fd, buf, bytes_left, 0);
        if(bytes_sent_now < 0) {
            perror("send()");
            return send_fatal_error;
        }
    }

    if(bytes_left < 0) {
        perror("read()");
        return send_fatal_error;
    }

    return send_ok;
}

static int sock_init(int sock_fd)
{
    struct timeval tv;
    int res;

    tv.tv_sec = conf_keep_alive_timeout;
    tv.tv_usec = 0;

    res = setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if(res < 0) {
        perror("setsockopt()");
        return 0;
    }

    return 1;
}

int worker_run(int conn_fd)
{
    char *buf;
    int exit_code;

    struct http_req request;
    struct http_res response;

    int buf_pos;
    int buf_len;
    int data_len;
    enum recv_status recv_status;
    enum send_status send_status;
    int handle_status;

    buf_len = buf_pos = 0;
    memset(&request, 0, sizeof(request));
    memset(&response, 0, sizeof(response));

    exit_code = EXIT_FAILURE;

    buf = malloc(conf_buf_size);
    if(!buf) {
        perror("malloc()");
        goto exit;
    }

    handle_status = sock_init(conn_fd);
    if(!handle_status) {
        goto exit;
    }

    for(;;) {
        data_len = recv(conn_fd, buf + buf_len, conf_buf_size - buf_len, 0);
        if(data_len < 0) {
            /* If errno == EWOULDBLOCK, timeout expired */
            if(errno != EWOULDBLOCK) {
                perror("recv()");
            }
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

        /* Continue receiving only when there is space in the buffer left */
        if(recv_status == recv_continue && buf_len < conf_buf_size) {
            continue;
        }

        handle_status = handler_res_creat(&response, &request);
        send_status = res_send(conn_fd, &response, buf, conf_buf_size);

        /* Close connection if handler returned 0, or send error occured */
        if(!handle_status || send_status == send_fatal_error) {
            goto exit;
        }

        buf_len = buf_pos = 0;

        /* Free request headers & reset request */
        http_hdrs_remove(&request.hdrs);
        memset(&request, 0, sizeof(request));

        /* Free response */
        handler_res_free(&response);
    }

exit:
    /* Free request & response */
    http_hdrs_remove(&request.hdrs);
    handler_res_free(&response);

    free(buf);
    close(conn_fd);
    return exit_code;
}

