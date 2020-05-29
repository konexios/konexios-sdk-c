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
#include <konexios_config.h>
#include <sys/mem.h>
#include <arrow/utf8.h>

static const char *METH_str[] = { "GET", "POST", "PUT", "DELETE", "HEAD"};

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

extern int default_set_payload_handler(void *r,
                                       property_t buf);
extern int default_add_payload_handler(void *r,
                                       property_t buf);

#include <arrow/credentials.h>

int http_request_init(http_request_t *req,
                       int meth,
                       property_t *uri) {
  req->is_corrupt = 0;
  property_init(&req->meth);
  property_init(&req->host);
  property_init(&req->uri);
  property_init(&req->payload);
  property_copy(&req->meth, p_const(get_METH(meth)));

  req->header = NULL;
  req->query = NULL;
  req->is_chunked = 0;
  memset(&req->payload, 0x0, sizeof(http_payload_t));
  property_map_init(&req->content_type);
  req->_response_payload_meth._p_set_handler = default_set_payload_handler;
  req->_response_payload_meth._p_add_handler = default_add_payload_handler;

  if ( !arrow_api_host() ) {
      req->is_corrupt = 1;
      return -1;
  }
  req->host = p_const(arrow_api_host()->host);
  req->port = arrow_api_host()->port;
  req->scheme = arrow_api_host()->scheme;
  req->is_cipher = (req->scheme == arrow_scheme_https ? 1 : 0);

  property_move(&req->uri, uri);

#if 1
  DBG("meth: %s", P_VALUE(req->meth) );
  DBG("scheme: %s", (req->scheme?"https":"http"));
  DBG("host: %s", P_VALUE(req->host));
  DBG("port: %d", req->port);
#endif
  if(req->uri.size > 0){
	  DBG("uri: %s", P_VALUE(req->uri));
  }
  return 0;
}

void http_request_close(http_request_t *req) {
  if ( !req ) return;
  property_free(&req->meth);
  property_free(&req->host);
  property_free(&req->uri);
  property_free(&req->payload);
  property_map_clear(&req->header);
  property_map_clear(&req->query);
  property_free(&req->content_type.value);
  property_free(&req->content_type.key);
  memset(req, 0x0, sizeof(http_request_t));
}

void http_request_add_header(http_request_t *req, property_t key, property_t value) {
    property_map_add(&req->header, key, value);
}

void http_request_set_header(http_request_t *req, property_t key, property_t value) {
    property_map_assign(req->header, key, value);
}
int  http_request_find_header(http_request_t *req, property_t key, property_t *value) {
    property_map_t *fm = property_map_find(req->header, key);
    if ( !fm ) return -1;
    if (value) property_copy(value, fm->value);
    return 0;
}

void http_request_add_query(http_request_t *req, property_t key, property_t value) {
  property_map_add(&req->query, key, value);
}

void http_request_set_content_type(http_request_t *req, property_t value) {
  req->content_type.key = p_const(CONTENT_TYPE);
  req->content_type.value = value;
}

property_map_t *http_request_first_header(http_request_t *req) {
    return req->header;
}

void http_request_set_payload(http_request_t *req, property_t payload) {
  property_move(&req->payload, &payload);
  if ( IS_EMPTY(req->payload) ) {
    DBG("[http] set_payload: fail");
  }
}

int http_request_set_findby(http_request_t *req, find_by_t *fb) {
    find_by_t *tmp = NULL;
    find_by_for_each(tmp, fb) {
        http_request_add_query(req,
                find_by_to_property(tmp),
                tmp->value);
    }
    return 0;
}
