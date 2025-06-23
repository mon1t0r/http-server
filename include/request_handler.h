#ifndef HTTP_SERVER_REQUEST_HANDLER_H
#define HTTP_SERVER_REQUEST_HANDLER_H

#include <arpa/inet.h>

#include "http_request.h"
#include "http_response.h"

int handler_response_create(struct http_response *response,
                            const struct http_request *request,
                            const struct sockaddr_in *addr);

void handler_response_free(struct http_response *response);

#endif
