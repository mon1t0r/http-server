#include <stdio.h>
#include <string.h>

#include "request_handler.h"
#include "http_request.h"
#include "http_response.h"

static int handle_get(const struct http_request *request,
                      struct http_response *response)
{
    response->status_line.version = request->request_line.version;
    response->status_line.status = http_ok;
    response->status_line.reason =
        http_status_str_get(response->status_line.status);
    return 1;
}

int handler_handle_request(const struct http_request *request,
                           const struct sockaddr_in *addr,
                           struct http_response *response)
{
    struct http_header_entry *header;

    printf("Received HTTP request: %d %s %d.%d\n",
           request->request_line.method,
           request->request_line.request_uri,
           request->request_line.version.major,
           request->request_line.version.minor);

    printf("Headers:\n");

    header = request->headers;
    while(header != NULL) {
        printf("%d: %s\n", header->type, header->value);
        header = header->next;
    }

    switch(request->request_line.method) {
        case http_get:
            return handle_get(request, response);
            break;
        default:
            return 0;
    }
}
