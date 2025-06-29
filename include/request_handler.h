#ifndef HTTP_SERVER_REQUEST_HANDLER_H
#define HTTP_SERVER_REQUEST_HANDLER_H

#include <arpa/inet.h>

#include "http_request.h"
#include "http_response.h"

int handler_res_creat(struct http_res *response,
                      const struct http_req *request,
                      const struct sockaddr_in *addr);

void handler_res_free(struct http_res *response);

#endif
