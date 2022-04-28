/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "konexios/sign.h"

#include <konexios_config.h>
#include <konexios/storage.h>
#include <sys/mem.h>
#include <konexios/utf8.h>
#include <ssl/crypt.h>
#include <time/time.h>

#include <data/chunk.h>
#include <debug.h>

#if defined(SIGN_DEBUG)
# define DBG_SIGN DBG
#else
# define DBG_SIGN(...)
#endif

// by default keys
#if defined(DEFAULT_API_KEY) && defined(DEFAULT_SECRET_KEY)
static const char *default_api_key = DEFAULT_API_KEY;
static const char *default_secret_key = DEFAULT_SECRET_KEY;
#else
static const char *default_api_key = NULL;
static const char *default_secret_key = NULL;
#endif

static char api_key[66];
static char secret_key[50];

typedef struct {
  char *key;
} iot_key_t;

static iot_key_t api = {NULL};
static iot_key_t secret = {NULL};

char *get_api_key(void) {
  if (api.key) return api.key;
  if ( restore_key_setting(api_key, NULL) < 0 ) {
    return (char*)default_api_key;
  }
  return api_key;
}

char *get_secret_key(void) {
  if (secret.key) return secret.key;
  if ( restore_key_setting(NULL, secret_key) < 0 ) {
    return (char*)default_secret_key;
  }
  return secret_key;
}

static void set_key(iot_key_t *iot, char *newkey) {
    if ( newkey ) {
        memcpy(iot->key, newkey, strlen(newkey));
        iot->key[strlen(newkey)] = '\0';
    } else {
        iot->key = NULL;
    }
}

void set_api_key(char *newkey) {
  api.key = api_key;
  set_key(&api, newkey);
}
void set_secret_key(char *newkey) {
  secret.key = secret_key;
  set_key(&secret, newkey);
}

void sign(char *signature,
          const char *timestamp,
          property_t *meth,
          const char *uri,
          const char *canQueryString,
          const char *payload,
          const char *apiVersion) {

    DBG_SIGN("api_key: %s", get_api_key());
    DBG_SIGN("secret_key: %s", get_secret_key());
    DBG_SIGN("timestamp: %s", timestamp);

    sha256_init();
    sha256_chunk(P_VALUE(*meth), property_size(meth));
    sha256_chunk("\n", 1);
    sha256_chunk(uri, strlen(uri));
    sha256_chunk("\n", 1);
    if (canQueryString) {
        sha256_chunk(canQueryString, strlen(canQueryString));
    }

    CREATE_CHUNK(hex_hash_payload, 66);
    CREATE_CHUNK(hash_payload, 34);
    if (payload) {
      sha256(hash_payload, (char*)payload, (int)strlen(payload));
      hex_encode(hex_hash_payload, hash_payload, 32);
      hex_hash_payload[64] = '\0';
    } else {
      strcpy(hex_hash_payload, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"); // pre calculated null string hash
    }

    sha256_chunk(hex_hash_payload, 64);
//    DBG_SIGN("<caninical request>%s<end>", canonicalRequest);

    sha256_fin(hash_payload);
    hex_encode(hex_hash_payload, hash_payload, 32);
    hex_hash_payload[64] = '\0';
    DBG_SIGN("hashed canonical request: %s", hex_hash_payload);
//    stringToSign := hashedCanonicalRequest +"\n"+apiKey+"\n"+timestamp+"\n"+apiVersion

    FREE_CHUNK(hash_payload);

    CREATE_CHUNK(signKey, 128);
    strcpy(signKey, get_secret_key());

    CREATE_CHUNK(tmp, 128);
    hmac256(tmp, get_api_key(), (int)strlen(get_api_key()), signKey, (int)strlen(signKey));
    hex_encode(signKey, tmp, 32);
    DBG_SIGN("step 1: %s", signKey);
    hmac256(tmp, timestamp, (int)strlen(timestamp), signKey, 64);
    hex_encode(signKey, tmp, 32);
    DBG_SIGN("step 2: %s", signKey);
    hmac256(tmp, apiVersion, (int)strlen(apiVersion), signKey, 64);
    hex_encode(signKey, tmp, 32);
    DBG_SIGN("step 3: %s", signKey);
    hmac256_init(signKey, 64);
    hmac256_chunk(hex_hash_payload, 64);
    hmac256_chunk("\n", 1);
    hmac256_chunk(get_api_key(), strlen(get_api_key()));
    hmac256_chunk("\n", 1);
    hmac256_chunk(timestamp, strlen(timestamp));
    hmac256_chunk("\n", 1);
    hmac256_chunk(apiVersion, strlen(apiVersion));
//    DBG_SIGN("<string to sign>%s<end>", canonicalRequest);
    hmac256_fin(tmp);
    hex_encode(signature, tmp, 32);
    FREE_CHUNK(hex_hash_payload);
    FREE_CHUNK(tmp);
    DBG_SIGN("sign: %s", signature);
}

static void get_canonical_string(char *buffer, http_request_t *req){
    property_map_t *query = req->query;
    buffer[0] = '\0';
    konexios_linked_list_for_each(query, req->query, property_map_t) {
        strcat(buffer, P_VALUE(query->key));
        strcat(buffer, "=");
        strcat(buffer, P_VALUE(query->value));
        strcat(buffer, "\r\n");
    }
}

void sign_request(http_request_t *req) {
    acn_timestamp_t timest;
    static char ts[25];
    static char signature[70];
    char *canonicalQuery = NULL;
    if ( req->query ) {
      canonicalQuery = (char*)malloc(SIGN_BUFFER_LEN);
      get_canonical_string(canonicalQuery, req);
    }
    timestamp(&timest);
    timestamp_string(&timest, ts);
    if ( http_request_find_header(req, p_const("x-arrow-apikey"), NULL) < 0 ) {
        http_request_add_header(req,
                                p_const("x-arrow-apikey"),
                                p_const(DEFAULT_API_KEY));
    }
    http_request_add_header(req,
                            p_const("x-arrow-date"),
                            p_const(ts));
    http_request_add_header(req,
                            p_const("x-arrow-version"),
                            p_const("1"));

    sign(signature, ts, &req->meth,
         P_VALUE(req->uri), canonicalQuery,
         P_VALUE(req->payload), "1");

    if (canonicalQuery) free(canonicalQuery);

    http_request_add_header(req,
                            p_const("x-arrow-signature"),
                            p_const(signature));
    http_request_set_content_type(req, p_const("application/json"));
    http_request_add_header(req,
                            p_const("Accept"),
                            p_const("application/json"));
    http_request_add_header(req,
                            p_const("Connection"),
                            p_const("Keep-Alive"));
    http_request_add_header(req,
                            p_const("Accept-Encoding"),
                            p_const("gzip, deflate"));
    http_request_add_header(req,
                            p_const("User-Agent"),
                            p_const("Eos"));
}
