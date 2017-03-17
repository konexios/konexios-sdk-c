/*
 * request.c
 *
 *  Created on: 13 окт. 2016 г.
 *      Author: ddemidov
 */

#include "arrow/request.h"
#include <time/time.h>
#include <arrow/mem.h>
#if !defined(__XCC__)
#include <string.h>
#endif

void get_canonical_string(char *buffer, http_request_t *req){
    http_query_t *query = req->query;
    buffer[0] = '\0';
    while (query) {
        strcat(buffer, (char*)query->key);
        strcat(buffer, "=");
        strcat(buffer, (char*)query->value);
        strcat(buffer, "\r\n");
        query = query->next;
    }
}

void sign_request(http_request_t *req) {
    char ts[25];
    char signature[70];
    char *canonicalQuery = NULL;
    if ( req->query ) {
      canonicalQuery = (char*)malloc(CANONICAL_QUERY_LEN);
      get_canonical_string(canonicalQuery, req);
    }
    get_time(ts);
    http_request_add_header(req, "x-arrow-apikey", get_api_key());
    http_request_add_header(req, "x-arrow-date", ts);
    http_request_add_header(req, "x-arrow-version", "1");
    sign(signature, ts, (char*)req->meth,
         (char*)req->uri, canonicalQuery,
         (char*)req->payload.buf, "1");
    if (canonicalQuery) free(canonicalQuery);
    http_request_add_header(req, "x-arrow-signature", signature);
    http_request_set_content_type(req, "application/json");
    http_request_add_header(req, "Accept", "application/json");
    http_request_add_header(req, "Connection", "Keep-Alive");
    http_request_add_header(req, "Accept-Encoding", "gzip, deflate");
    http_request_add_header(req, "User-Agent", "Go 1.1 package http");
}
