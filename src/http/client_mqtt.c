/* Copyright (c) 2018 Arrow Electronics, Inc.
* All rights reserved. This program and the accompanying materials
* are made available under the terms of the Apache License 2.0
* which accompanies this distribution, and is available at
* http://apache.org/licenses/LICENSE-2.0
* Contributors: Arrow Electronics, Inc.
*/

#include "http/client_mqtt.h"
#if defined(HTTP_VIA_MQTT)
#include <arrow/mqtt.h>
#include <arrow/sign.h>
#include <arrow/events.h>
#include <arrow/routine.h>
#include <debug.h>
#include <arrow/gateway_payload_sign.h>

static int mqtt_connection_error() {
    arrow_mqtt_disconnect_routine();
    arrow_routine_error_t connect_error = ROUTINE_ERROR;
    int retry = 0;
    while ( connect_error != ROUTINE_SUCCESS &&
            connect_error != ROUTINE_RECEIVE_EVENT ) {
        RETRY_UP(retry, {
                     return -1;
                 });
        connect_error = arrow_mqtt_connect_routine();
    }
    return 0;
}

static int headbyname( property_map_t *h, const char *name ) {
    if ( strcmp(P_VALUE(h->key), name) == 0 ) return 0;
    return -1;
}

int http_mqtt_client_open(http_client_t *cli, http_request_t *req) {
    cli->response_code = 0;
    cli->request = req;
    if ( cli->protocol != api_via_mqtt ) return -1;
    return 0;
}

static int mqtt_error = 0;
int http_mqtt_client_close(http_client_t *cli) {
    SSP_PARAMETER_NOT_USED(cli);
    if ( cli->flags._close && mqtt_error ) {
        mqtt_connection_error();
        mqtt_error = 0;
    }
    return 0;
}

int http_mqtt_client_do(http_client_t *cli, http_response_t *res) {
    int ret = 0;
    http_request_t *req = cli->request;
    if ( !req ) return -1;
    http_response_init(res, &req->_response_payload_meth);

    JsonNode *_node = json_mkobject();
    if ( !_node ) {
        http_session_force_http(1);
        return -1;
    }
    JsonNode *_parameters = json_mkobject();
    if ( !_parameters ) {
        http_session_force_http(1);
        goto http_mqtt_error;
    }

    property_map_t *tmp = NULL;
    char reqhid[50];
    strcpy(reqhid, "GS-");
    get_time(reqhid+3);
    ret = json_append_member(_node,
                             p_const("requestId"),
                             json_mkstring(reqhid));
    if ( ret < 0 ) {
        http_session_force_http(1);
        goto http_mqtt_error;
    }
    ret = json_append_member(_node,
                             p_const("eventName"),
                             json_mkstring("GatewayToServer_ApiRequest"));
    if ( ret < 0 ) {
        http_session_force_http(1);
        goto http_mqtt_error;
    }
    ret = json_append_member(_node,
                             p_const("encrypted"),
                             json_mkbool(false));
    if ( ret < 0 ) {
        http_session_force_http(1);
        goto http_mqtt_error;
    }
    //add parameter
    ret = json_append_member(_parameters,
                             p_const("uri"),
                             json_mkstring(P_VALUE(req->uri)));
    if ( ret < 0 ) {
        http_session_force_http(1);
        goto http_mqtt_error;
    }
    ret = json_append_member(_parameters,
                       p_const("method"),
                       json_mkstring(P_VALUE(req->meth)));
    if ( ret < 0 ) {
        http_session_force_http(1);
        goto http_mqtt_error;
    }

    linked_list_find_node(tmp, req->header, property_map_t, headbyname, "x-arrow-apikey");
    if ( tmp ) {
        ret = json_append_member(_parameters,
                           p_const("apiKey"),
                           json_mkstring(P_VALUE(tmp->value)));
        if ( ret < 0 ) {
            http_session_force_http(1);
            goto http_mqtt_error;
        }
    }
    if ( !IS_EMPTY(req->payload) ) {
        ret = json_append_member(_parameters,
                           p_const("body"),
                           json_mkstring(P_VALUE(req->payload)));
        if ( ret < 0 ) {
            http_session_force_http(1);
            goto http_mqtt_error;
        }
    }
    linked_list_find_node(tmp, req->header, property_map_t, headbyname, "x-arrow-signature");
    if ( tmp ) {
        ret = json_append_member(_parameters,
                           p_const("apiRequestSignature"),
                           json_mkstring(P_VALUE(tmp->value)));
        if ( ret < 0 ) {
            http_session_force_http(1);
            goto http_mqtt_error;
        }
    }

    linked_list_find_node(tmp, req->header, property_map_t, headbyname, "x-arrow-version");
    if ( tmp ) {
        ret = json_append_member(_parameters,
                           p_const("apiRequestSignatureVersion"),
                           json_mkstring(P_VALUE(tmp->value)));
        if ( ret < 0 ) {
            http_session_force_http(1);
            goto http_mqtt_error;
        }
    }

    linked_list_find_node(tmp, req->header, property_map_t, headbyname, "x-arrow-date");
    if ( tmp ) {
        ret  =json_append_member(_parameters,
                           p_const("timestamp"),
                           json_mkstring(P_VALUE(tmp->value)));
        if ( ret < 0 ) {
            http_session_force_http(1);
            goto http_mqtt_error;
        }
    }
    char sig[65] = {0};
    ret = json_append_member(_node, p_const("parameters"), _parameters);
    if ( ret < 0 ) {
        http_session_force_http(1);
        goto http_mqtt_error;
    }
    if ( arrow_event_sign(sig,
                  p_stack(reqhid),
                  "GatewayToServer_ApiRequest",
                  0,
                  _parameters ) < 0 ) {
        _parameters = NULL;
        goto http_mqtt_error;
    }
    _parameters = NULL;

    ret = json_append_member(_node,
                       p_const("signature"),
                       json_mkstring(sig));
    if ( ret < 0 ) goto http_mqtt_error;
    ret = json_append_member(_node,
                       p_const("signatureVersion"),
                       json_mkstring("1"));
    if ( ret < 0 ) goto http_mqtt_error;

    arrow_mqtt_api_wait(1);
    ret = mqtt_api_publish(_node);

    DBG("publish %d", ret);
    if ( ret < 0 ) {
        goto http_mqtt_error;
    }

    TimerInterval timer;
    TimerInit(&timer);
    TimerCountdownMS(&timer, (unsigned int) 2*cli->timeout);
    while ( !arrow_mqtt_api_has_events() && !TimerIsExpired(&timer) ) {
        ret = mqtt_yield(TimerLeftMS(&timer));
        DBG("yield %d", ret);
    }

    if ( arrow_mqtt_api_has_events() <= 0 ) {
        ret = -1;
        goto http_mqtt_error;
    }

    while ( (ret = arrow_mqtt_api_event_proc(res)) > 0)
        ;
http_mqtt_error:
    if ( _node ) json_delete(_node);
    if ( _parameters ) json_delete(_parameters);
    arrow_mqtt_api_wait(0);
    if ( ret < 0 ) {
        mqtt_error = 1;
        DBG("%s %d", __PRETTY_FUNCTION__, ret);
    }
    return ret;
}

#else
typedef void __dummy;
#endif
