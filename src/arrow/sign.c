/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include <config.h>
#include <arrow/sign.h>
#include <arrow/mem.h>
#if defined(__USE_STD__)
# include <inttypes.h>
# include <stdbool.h>
# include <string.h>
# include <stdio.h>
# include <config.h>
#endif
#include <debug.h>
#include <crypt/crypt.h>

// by default keys
static const char *default_api_key = DEFAULT_API_KEY;
static const char *default_secret_key = DEFAULT_SECRET_KEY;

static char api_key[100];
static char secret_key[100];

typedef struct {
  char *key;
} iot_key_t;

static iot_key_t api = {NULL};
static iot_key_t secret = {NULL};

char *get_api_key() {
  if (api.key) return api.key;
  return (char*)default_api_key;
}

char *get_secret_key() {
  if (secret.key) return secret.key;
  return (char*)default_secret_key;
}

static void set_key(iot_key_t *iot, char *newkey) {
  memcpy(iot->key, newkey, strlen(newkey));
  iot->key[strlen(newkey)] = '\0';
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
          const char *meth,
          const char *uri,
          const char *canQueryString,
          const char *payload,
          const char *apiVersion) {
    int i;
    static char canonicalRequest[512];
    static char signKey[128];
    static char tmp[128];

    strcpy(canonicalRequest, meth);
    strcat(canonicalRequest, "\n");
    strcat(canonicalRequest, uri);
    strcat(canonicalRequest, "\n");
    if (canQueryString) strcat(canonicalRequest, canQueryString);
    char hex_hash_payload[66];
    char hash_payload[34];
    if (payload) {
      sha256(hash_payload, (char*)payload, (int)strlen(payload));
      for (i=0; i<32; i++) sprintf(hex_hash_payload+i*2, "%02x", (unsigned char)hash_payload[i]);
      hex_hash_payload[64] = '\0';
    } else {
      strcpy(hex_hash_payload, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"); // pre calculated null string hash
    }
    strncat(canonicalRequest, hex_hash_payload, 64);
//    DBG("<caninical request>%s<end>", canonicalRequest);
    sha256(hash_payload, canonicalRequest, (int)strlen(canonicalRequest));
    for (i=0; i<32; i++) sprintf(hex_hash_payload+i*2, "%02x", (unsigned char)(hash_payload[i]));
    hex_hash_payload[64] = '\0';
//    DBG("hashed canonical request: %s", hex_hash_payload);
//    stringToSign := hashedCanonicalRequest +"\n"+apiKey+"\n"+timestamp+"\n"+apiVersion

    strncpy(canonicalRequest, hex_hash_payload, 64);
    canonicalRequest[64] = 0x0;
    strcat(canonicalRequest, "\n");
    strcat(canonicalRequest, get_api_key());
    strcat(canonicalRequest, "\n");
    strcat(canonicalRequest, timestamp);
    strcat(canonicalRequest, "\n");
    strcat(canonicalRequest, apiVersion);
//    DBG("<string to sign>%s<end>", canonicalRequest);

    strcpy(signKey, get_secret_key());

    hmac256(tmp, get_api_key(), (int)strlen(get_api_key()), signKey, (int)strlen(signKey));
    for (i=0; i<32; i++) sprintf(signKey+i*2, "%02x", (unsigned char)(tmp[i]));
//    DBG("step 1: %s", signKey);
    hmac256(tmp, timestamp, (int)strlen(timestamp), signKey, 64);
    for (i=0; i<32; i++) sprintf(signKey+i*2, "%02x", (unsigned char)(tmp[i]));
//    DBG("step 2: %s", signKey);
    hmac256(tmp, apiVersion, (int)strlen(apiVersion), signKey, 64);
    for (i=0; i<32; i++) sprintf(signKey+i*2, "%02x", (unsigned char)(tmp[i]));
//    DBG("step 3: %s", signKey);
    hmac256(tmp, signKey, 64, canonicalRequest, (int)strlen(canonicalRequest));
    for (i=0; i<32; i++) sprintf(signature+i*2, "%02x", (unsigned char)(tmp[i]));
    signature[64] = '\0';
//    DBG("sign: %s", signature);
}
