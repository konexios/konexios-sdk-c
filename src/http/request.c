/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#define MODULE_NAME "HTTP_Request"

#include "http/request.h"
#include <debug.h>
#include <config.h>
#if defined(__USE_STD__)
# include <string.h>
# include <stdio.h>
# include <stdlib.h>
#endif
#include <arrow/mem.h>
#include <arrow/utf8.h>
#include <arrow/net.h>

static const char *METH_str[] = { "GET", "POST", "PUT", "DELETE", "HEAD"};
static const char *Scheme_str[] = { "http", "https" };

#define CMP_INIT(type) \
static int __attribute__((used)) cmp_##type(const char *str) { \
  int i; \
  for(i=0; i<type##_count; i++) { \
    if ( strcmp(str, type##_str[i]) == 0 ) return i; \
  } \
  return -1; \
} \
static int __attribute__((used)) cmp_n_##type(const char *str, int n) { \
  int i; \
  for(i=0; i<type##_count; i++) { \
    if ( strncmp(str, type##_str[i], n) == 0 ) return i; \
  } \
  return -1; \
} \
static char * __attribute__((used)) get_##type(int i) { \
    if ( i>=0 && i<type##_count ) { \
        return (char*)type##_str[i]; \
    } \
    return NULL; \
}

CMP_INIT(METH)
CMP_INIT(Scheme)

#if defined(_ARIS_)
int cmp_meth(const char *str) { return cmp_METH(str); }
char *get_meth(int i) { return get_METH(i); }
#endif

extern int default_set_payload_handler(void *r,
                                       property_t buf,
                                       int size);
extern int default_add_payload_handler(void *r,
                                       property_t buf,
                                       int size);

void http_request_init(http_request_t *req, int meth, const char *url) {
  req->is_corrupt = 0;
  P_CLEAR(req->meth);
  P_CLEAR(req->scheme);
  P_CLEAR(req->host);
  P_CLEAR(req->uri);
  P_CLEAR(req->payload.buf);
  property_copy(&req->meth, p_const(get_METH(meth)));
  char *sch_end = strstr((char*)url, "://");
  if ( !sch_end ) { req->is_corrupt = 1; return; }
  int sch = cmp_n_Scheme(url, (int)(sch_end - url));
  P_COPY(req->scheme, p_const(get_Scheme(sch)));
  req->is_cipher = sch;
  char *host_start = sch_end + 3; //sch_end + strlen("://");
  char *host_end = strstr(host_start, ":");
  if ( !host_end ) { req->is_corrupt = 1; return; }
  property_n_copy(&req->host, host_start, (host_end - host_start));

  uint16_t port = 0;
  int res = sscanf(host_end+1, "%8hu", &port);
  if ( res!=1 ) { req->is_corrupt = 1; return; }
  req->port = port;

  char *uri_start = strstr(host_end+1, "/");
  P_COPY(req->uri, p_stack(uri_start));

  DBG("meth: %s", P_VALUE(req->meth) );
  DBG("scheme: %s", P_VALUE(req->scheme));
  DBG("host: %s", P_VALUE(req->host));
  DBG("port: %d", req->port);
  DBG("uri: %s", P_VALUE(req->uri));
  DBG("res: %d", res);

  req->header = NULL;
  req->query = NULL;
  req->is_chunked = 0;
  memset(&req->payload, 0x0, sizeof(http_payload_t));
  memset(&req->content_type, 0x0, sizeof(property_map_t));
  req->_response_payload_meth._p_set_handler = default_set_payload_handler;
  req->_response_payload_meth._p_add_handler = default_add_payload_handler;
}

void http_request_close(http_request_t *req) {
  P_FREE(req->meth);
  P_FREE(req->scheme);
  P_FREE(req->host);
  P_FREE(req->uri);
  P_FREE(req->payload.buf);
  req->payload.size = 0;
  property_map_clear(req->header);
  property_map_clear(req->query);
  P_FREE(req->content_type.value);
  P_FREE(req->content_type.key);
}

void http_request_add_header(http_request_t *req, property_t key, property_t value) {
    property_map_add(&req->header, key, value);
}

void http_request_add_query(http_request_t *req, property_t key, property_t value) {
  property_map_add(&req->query, key, value);
}

void http_request_set_content_type(http_request_t *req, property_t value) {
  req->content_type.key = property(CONTENT_TYPE, is_const);
  req->content_type.value = value;
}

property_map_t *http_request_first_header(http_request_t *req) {
    return req->header;
}

property_map_t *http_request_next_header(http_request_t *req, property_map_t *head) {
    SSP_PARAMETER_NOT_USED(req);
    if ( ! head ) return NULL;
    return head->next;
}

void http_request_set_payload(http_request_t *req, property_t payload) {
  req->payload.size = P_SIZE(payload);
  P_COPY(req->payload.buf, payload);
  if ( IS_EMPTY(req->payload.buf) ) {
    DBG("[http] set_payload: fail");
  }
}
