#ifndef HTTP_SERVER_REQUEST_HANDLER_H
#define HTTP_SERVER_REQUEST_HANDLER_H

#include <arpa/inet.h>

#include "http_request.h"
#include "http_response.h"

int handler_handle_request(const struct http_request *request,
                           const struct sockaddr_in *addr,
                           struct http_response *response);

#endif
