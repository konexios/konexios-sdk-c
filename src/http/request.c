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

#define CONTENT_TYPE "Content-Type"

static const char *METH_str[] = { "GET", "POST", "PUT", "DELETE", "HEAD"};
static const char *Scheme_str[] = { "http", "https" };

#define CMP_INIT(type) \
int cmp_##type(const char *str) { \
  int i; \
  for(i=0; i<type##_count; i++) { \
    if ( strcmp(str, type##_str[i]) == 0 ) return i; \
  } \
  return -1; \
} \
int cmp_n_##type(const char *str, int n) { \
  int i; \
  for(i=0; i<type##_count; i++) { \
    if ( strncmp(str, type##_str[i], n) == 0 ) return i; \
  } \
  return -1; \
} \
char *get_##type(int i) { \
    if ( i>=0 && i<type##_count ) { \
        return (char*)type##_str[i]; \
    } \
    return NULL; \
}

CMP_INIT(METH)
CMP_INIT(Scheme)

P_ADD(http_header, key)
P_ADD(http_header, value)

P_ADD(http_request, meth)
P_ADD(http_request, scheme)
P_ADD(http_request, host)
P_ADD(http_request, uri)

void http_request_init(http_request_t *req, int meth, const char *url) {
  req->is_corrupt = 0;
  http_request_set_meth(req, get_METH(meth));
  char *sch_end = strstr((char*)url, "://");
  if ( !sch_end ) { req->is_corrupt = 1; return; }
  int sch = cmp_n_Scheme(url, (int)(sch_end - url));
  http_request_set_scheme(req, get_Scheme(sch));
  req->is_cipher = sch;

  char *host_start = sch_end + 3; //sch_end + strlen("://");
  char *host_end = strstr(host_start, ":");
  if ( !host_end ) { req->is_corrupt = 1; return; }
  http_request_set_host_copy(req, host_start, (host_end - host_start));

  unsigned long port = 0;
  int res = sscanf(host_end+1, "%8hu", &port);
  if ( res!=1 ) { req->is_corrupt = 1; return; }
  req->port = port;

  char *uri_start = strstr(host_end+1, "/");
  http_request_set_uri_dup(req, uri_start);

  DBG("meth: %s", P_VALUE(req->meth) );
  DBG("scheme: %s", P_VALUE(req->scheme));
  DBG("host: %s", P_VALUE(req->host));
  DBG("port: %d", req->port);
  DBG("uri: %s", req->uri);
  DBG("res: %d", res);

  req->header = NULL;
  req->query = NULL;
  req->is_chunked = 0;
  memset(&req->payload, 0x0, sizeof(http_payload_t));
  memset(&req->content_type, 0x0, sizeof(http_header_t));
}

void http_request_close(http_request_t *req) {
  P_FREE(req->meth);
  P_FREE(req->scheme);
  P_FREE(req->host);
  P_FREE(req->uri);

    if (req->payload.buf) free(req->payload.buf);
    req->payload.size = 0;
    http_header_t *head = req->header;
    http_header_t *head_next = NULL;
    do {
        if (head) {
            head_next = head->next;
            P_FREE(head->key);
            P_FREE(head->value);
            free(head);
        }
        head = head_next;
    } while(head);

    http_query_t *query = req->query;
    http_query_t *query_next = NULL;
    do {
        if (query) {
            query_next = query->next;
            P_FREE(query->key);
            P_FREE(query->value);
            free(query);
        }
        query = query_next;
    } while(query);

    P_FREE(req->content_type.value);
    P_FREE(req->content_type.key);
}

void http_response_free(http_response_t *res) {
    if (res->payload.buf) free(res->payload.buf);
    res->payload.size = 0;
    http_header_t *head = res->header;
    http_header_t *head_next = NULL;
    do {
        if (head) {
            head_next = head->next;
            P_FREE(head->key);
            P_FREE(head->value);
            free(head);
        }
        head = head_next;
    } while(head);
    P_FREE(res->content_type.value);
    P_FREE(res->content_type.key);

}

void http_request_add_header(http_request_t *req, property_t key, property_t value) {
    http_header_t *head = req->header;
    while( head && head->next ) {
//        if ( head ) {
//            DBG("--:{%s}", head->value);
//        }
        head = head->next;
    }
    http_header_t *head_new = (http_header_t *)malloc(sizeof(http_header_t));
    head_new->key = key;
    head_new->value = value;
    head_new->next = NULL;
    if ( head ) head->next = head_new;
    else req->header = head_new;
}

void http_request_add_query(http_request_t *req, property_t key, property_t value) {
  http_query_t *head = req->query;
  while ( head && head->next ) head = head->next;
  http_query_t *head_new = (http_query_t *)malloc(sizeof(http_query_t));
  head_new->key = key;
  head_new->value = value;
  head_new->next = NULL;
  if ( head ) head->next = head_new;
  else req->query = head_new;
}

void http_request_set_content_type(http_request_t *req, property_t value) {
  req->content_type.key = property(CONTENT_TYPE, is_const);
  req->content_type.value = value;
}

http_header_t *http_request_first_header(http_request_t *req) {
    return req->header;
}

http_header_t *http_request_next_header(http_request_t *req, http_header_t *head) {
    SSP_PARAMETER_NOT_USED(req);
    if ( ! head ) return NULL;
    return head->next;
}

static int set_payload(http_payload_t *pay, const char *buf, uint32_t size) {
    pay->size = size;
    if ( pay->buf ) {
        pay->buf = (uint8_t*)realloc(pay->buf, size+1);
    } else {
        pay->buf = (uint8_t*)malloc(size+1);
    }
    if ( !pay->buf ) {
      DBG("[http] set_payload: fail");
      return -1;
    }
    memcpy(pay->buf, buf, size);
    pay->buf[pay->size] = '\0';
    return 0;
}

static int set_payload_ptr(http_payload_t *pay, char *buf, uint32_t size) {
    pay->size = size;
    if ( pay->buf ) {
        free(pay->buf);
    }
    pay->buf = (uint8_t*)buf;
    return 0;
}

void http_request_set_payload(http_request_t *req, char *payload) {
    set_payload(&req->payload, payload, strlen(payload));
}

void http_request_set_payload_ptr(http_request_t *req, char *payload) {
    set_payload_ptr(&req->payload, payload, strlen(payload));
}

void http_response_add_header(http_response_t *req, property_t key, property_t value) {
    http_header_t *head = req->header;
    while( head && head->next ) head = head->next;
    http_header_t *head_new = (http_header_t *)malloc(sizeof(http_header_t));
    P_COPY(head_new->key, key);
    P_COPY(head_new->value, value);
    head_new->next = NULL;
    if ( head ) head->next = head_new;
    else req->header = head_new;
}

void http_response_set_content_type(http_response_t *res, property_t value) {
  res->content_type.key = property(CONTENT_TYPE, is_const);
  P_COPY(res->content_type.value, value);
}

void http_response_set_payload(http_response_t *res, char *payload, uint32_t size) {
    if ( !size ) size = strlen(payload);
    set_payload(&res->payload, payload, size);
}

void http_response_add_payload(http_response_t *res, char *payload, uint32_t size) {
    if ( !size ) size = strlen(payload);
    if ( !res->payload.buf ) {
        set_payload(&res->payload, payload, size);
        return;
    } else {
        res->payload.size += size;
        uint8_t *old_buf = res->payload.buf;
        res->payload.buf = (uint8_t*)malloc(res->payload.size+1);
        memcpy(res->payload.buf, old_buf, res->payload.size - size);
        free(old_buf);
        memcpy(res->payload.buf + res->payload.size - size, payload, size);
        res->payload.buf[res->payload.size] = '\0';
    }
}
