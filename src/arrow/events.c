/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/events.h"
#include <debug.h>
#include <http/client.h>

static char *form_evetns_url(const char *hid, event_t ev) {
    char *uri = (char *)malloc(strlen(ARROW_API_EVENTS_ENDPOINT) + strlen(hid) + 15);
    strcpy(uri, ARROW_API_GATEWAY_ENDPOINT);
    strcat(uri, "/");
    strcat(uri, hid);
    switch(ev) {
        case failed:    strcat(uri, "/failed"); break;
        case received:  strcat(uri, "/received"); break;
        case succeeded: strcat(uri, "/succeeded"); break;
    }
    return uri;
}

int arrow_send_event_ans(const char *hid, event_t ev, const char *payload) {
    http_client_t cli;
    http_response_t response;
    http_request_t request;
    
    char *uri = form_evetns_url(hid, ev);
    http_client_init( &cli );
    http_request_init(&request, PUT, uri);
    if ( payload ) {
        http_request_set_payload(&request, payload);
    }    
    sign_request(&request);
    http_client_do(&cli, &request, &response);
    http_request_close(&request);
    DBG("response %d", response.m_httpResponseCode);
    
    http_client_free(&cli);
    free(uri);
    if ( response.m_httpResponseCode != 200 ) {
        http_response_free(&response);
        return -1;
    }
    http_response_free(&response);
    return 0;
}
