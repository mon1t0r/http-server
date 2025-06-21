#include <stdio.h>

#include "request_handler.h"

void handler_handle_request(const struct http_request *request,
                            const struct sockaddr_in *addr)
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
