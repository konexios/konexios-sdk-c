#include "arrow/gateway_payload_sign.h"
#include <arrow/sign.h>
#include <ssl/crypt.h>
#include <arrow/mem.h>
#include <debug.h>
#include <arrow/utf8.h>

#define USE_STATIC

int gateway_payload_sign(char *signature,
                         const char *hid,
                         const char *name,
                         int encrypted,
                         const char *canParString,
                         const char *signatureVersion) {
  // step 1
  static CREATE_CHUNK(canonicalRequest, 256);
  strcpy(canonicalRequest, hid);
  strcat(canonicalRequest, "\n");
  strcat(canonicalRequest, name);
  strcat(canonicalRequest, "\n");
  if ( encrypted ) strcat(canonicalRequest, "true\n");
  else strcat(canonicalRequest, "false\n");
  strcat(canonicalRequest, canParString);
  strcat(canonicalRequest, "\n");
  CREATE_CHUNK(hex_tmp, 66);
  CREATE_CHUNK(tmp, 34);

  sha256(tmp, canonicalRequest, (int)strlen(canonicalRequest));
  hex_encode(hex_tmp, tmp, 32);
  hex_tmp[64] = '\0';
//  DBG("can: %s", hex_hash_canonical_req);

  // step 2
  char *stringtoSign = canonicalRequest;
  strcpy(stringtoSign, hex_tmp);
  strcat(stringtoSign, "\n");
  strcat(stringtoSign, get_api_key());
  strcat(stringtoSign, "\n");
  strcat(stringtoSign, signatureVersion);

//  DBG("strToSign:\r\n[%s]", stringtoSign);
  // step 3

//  DBG("api: %s", get_api_key());
//  DBG("sec: %s", get_secret_key());
  hmac256(tmp, get_api_key(), (int)strlen(get_api_key()), get_secret_key(), (int)strlen(get_secret_key()));
  hex_encode(hex_tmp, tmp, 32);
  hex_tmp[64] = 0x0;
//  DBG("hex1: %s", hex_tmp);
  memset(tmp, 0, 34);
  hmac256(tmp, signatureVersion, (int)strlen(signatureVersion), hex_tmp, (int)strlen(hex_tmp));
  hex_encode(hex_tmp, tmp, 32);
  hex_tmp[64] = 0x0;
//  DBG("hex2: [%d]%s", strlen(hex_tmp), hex_tmp);
  hmac256(tmp, hex_tmp, strlen(hex_tmp), stringtoSign, strlen(stringtoSign));
  hex_encode(signature, tmp, 32);
  signature[64] = 0x0;
//  DBG("sig: [%d]%s", strlen(signature), signature);
  FREE_CHUNK(canonicalRequest);
  FREE_CHUNK(tmp);
  FREE_CHUNK(hex_tmp);
  return 0;
}
