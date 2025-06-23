#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "request_handler.h"
#include "http.h"
#include "http_request.h"
#include "http_response.h"

#define SERVER_STRING "TestServer/1.0"

enum { int_buf_size = 32 };

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

static void response_setup(struct http_response *response,
                           enum http_status status)
{
    struct http_header_entry header;

    response->status_line.version.major = 1;
    response->status_line.version.minor = 1;
    response->status_line.status = status;
    response->status_line.reason = (char *) http_status_str_get(status);

    header.type = http_date;
    header.value = time_get_str();
    http_add_header(&response->headers, &header);

    header.type = http_server;
    header.value = SERVER_STRING;
    http_add_header(&response->headers, &header);
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

static void response_header_length_free(struct http_header_entry *header)
{
    free(header->value);
}

static int response_create_get(struct http_response *response,
                               const struct http_request *request)
{
    struct http_header_entry header;

    response_setup(response, http_ok);

    response->content = "Hello from server!";

    header.type = http_content_type;
    header.value = "text/plain; charset=us-ascii";
    http_add_header(&response->headers, &header);

    response_header_length_create(&header, response->content);
    http_add_header(&response->headers, &header);

    return 1;
}

int handler_response_create(struct http_response *response,
                            const struct http_request *request,
                            const struct sockaddr_in *addr)
{
    struct http_header_entry *header;

    printf("Received HTTP request: %d %s HTTP/%d.%d\n",
           request->request_line.method,
           request->request_line.request_uri,
           request->request_line.version.major,
           request->request_line.version.minor);

    puts("Recognized headers:");

    header = request->headers;
    while(header != NULL) {
        printf("%s: %s\n", http_header_type_str_get(header->type),
               header->value);
        header = header->next;
    }

    puts("");

    switch(request->request_line.method) {
        case http_get:
            return response_create_get(response, request);
            break;
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
            response_header_length_free(header);
        }
        header = header->next;
    }

    http_remove_headers(&response->headers);
}
