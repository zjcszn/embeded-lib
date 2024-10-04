

#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include <stdint.h>
#include "rootca.h"
#include "core_http_client.h"
#include "transport_interface.h"


#define HTTPCLIENT_MAX_HOST_LENGTH      255

#define HTTPCLIENT_DOWNLOAD_SPEED_TEST  1 


typedef enum httpclient_status {
  HTTPCLIENT_OK,
  HTTPCLIENT_ERROR,
} httpclient_status_t;

typedef struct httpclient {
  char host[HTTPCLIENT_MAX_HOST_LENGTH + 1];
  uint16_t port;
  uint16_t over_tls;
  uint8_t *buffer;
  uint32_t bufsz;
  NetworkContext_t context;
  TransportInterface_t trans;
  HTTPResponse_t response;
  HTTPRequestInfo_t reqinfo;
  HTTPRequestHeaders_t request;
} httpclient_t;

int httpclient_download(const char *url, const char *fname);
int httpclient_get(const char *url, uint8_t *resp_buf, uint32_t resp_bufsz, const uint8_t **resp_body, uint32_t *resp_bodylen);

#endif //__HTTP_CLIENT_H__
