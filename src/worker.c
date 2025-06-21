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

static int header_create_add(struct http_request *request, char *str)
{
    struct http_header_entry *header;
    int parse_res;

    header = malloc(sizeof(struct http_header_entry));

    parse_res = http_header_parse(header, str);
    if(!parse_res) {
        free(header);
        return 0;
    }

    /* Unknown header type - ignore */
    if(header->type == http_header_unknown) {
        free(header);
        return 1;
    }

    http_request_add_header(request, header);

    return 1;
}

static void header_free_all(struct http_request *request) {
    struct http_header_entry *header_temp;

    while(request->headers != NULL) {
        header_temp = request->headers->next;

        free(request->headers);

        request->headers = header_temp;
    }
}

static int handle_data(struct http_request *request, char *buf,
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

        parse_res = header_create_add(request, line);
        if(!parse_res) {
            return -1;
        }
    }

    /* More data required */
    return 0;
}

static void handle_request(const struct http_request *request)
{
    struct http_header_entry *header;

    printf("Received HTTP request: %d %s %d.%d\n",
           request->request_line.method,
           request->request_line.request_uri,
           request->request_line.version_major,
           request->request_line.version_minor);

    printf("Headers:\n");

    header = request->headers;
    while(header != NULL) {
        printf("%d: %s\n", header->type, header->value);
        header = header->next;
    }
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

    buf = malloc(recv_buf_size);
    buf_len = buf_pos = 0;
    memset(&request, 0, sizeof(request));

    for(;;) {
        data_len = recv(conn_fd, buf + buf_len, recv_buf_size - buf_len, 0);
        if(data_len < 0) {
            perror("recv()");
            exit_code = EXIT_FAILURE;
            goto exit;
        }
        if(data_len == 0) {
            exit_code = EXIT_SUCCESS;
            goto exit;
        }

        buf_len += data_len;

        handle_res = handle_data(&request, buf, buf_len, &buf_pos);

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
        header_free_all(&request);
        memset(&request, 0, sizeof(request));
    }

exit:
    free(buf);
    close(conn_fd);
    return exit_code;
}
