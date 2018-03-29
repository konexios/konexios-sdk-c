#include "arrow/gateway_payload_sign.h"
#include <arrow/sign.h>
#include <ssl/crypt.h>
#include <sys/mem.h>
#include <debug.h>
#include <arrow/utf8.h>

#define USE_HEAP
#include <data/chunk.h>

int gateway_payload_sign(char *signature,
                         const char *hid,
                         const char *name,
                         int encrypted,
                         const char *canParString,
                         const char *signatureVersion) {
  int ret = -1;
  // step 1
  int total_size = strlen(hid);
  total_size += strlen(name);
  total_size += strlen(canParString);
  total_size += strlen(signatureVersion);
  total_size += 50;
  CREATE_CHUNK(canonicalRequest, total_size);
  CHECK_CHUNK(canonicalRequest, return ret);
  strcpy(canonicalRequest, hid);
  strcat(canonicalRequest, "\n");
  strcat(canonicalRequest, name);
  strcat(canonicalRequest, "\n");
  if ( encrypted ) strcat(canonicalRequest, "true\n");
  else strcat(canonicalRequest, "false\n");
  strcat(canonicalRequest, canParString);
  strcat(canonicalRequest, "\n");
  CREATE_CHUNK(hex_tmp, 66);
  CHECK_CHUNK(hex_tmp, goto hex_tmp_error);

  CREATE_CHUNK(tmp, 34);
  CHECK_CHUNK(tmp, goto tmp_error);

  sha256(tmp, canonicalRequest, (int)strlen(canonicalRequest));
  hex_encode(hex_tmp, tmp, 32);
  hex_tmp[64] = '\0';
#if defined(DEBUG_GATEWAY_PAYLOAD_SIGN)
  DBG("can: %s", hex_hash_canonical_req);
#endif

  // step 2
  char *stringtoSign = canonicalRequest;
  strcpy(stringtoSign, hex_tmp);
  strcat(stringtoSign, "\n");
  strcat(stringtoSign, get_api_key());
  strcat(stringtoSign, "\n");
  strcat(stringtoSign, signatureVersion);

#if defined(DEBUG_GATEWAY_PAYLOAD_SIGN)
  DBG("strToSign:\r\n[%s]", stringtoSign);
#endif
  // step 3

#if defined(DEBUG_GATEWAY_PAYLOAD_SIGN)
  DBG("api: %s", get_api_key());
  DBG("sec: %s", get_secret_key());
#endif
  hmac256(tmp, get_api_key(), (int)strlen(get_api_key()), get_secret_key(), (int)strlen(get_secret_key()));
  hex_encode(hex_tmp, tmp, 32);
  hex_tmp[64] = 0x0;
#if defined(DEBUG_GATEWAY_PAYLOAD_SIGN)
  DBG("hex1: %s", hex_tmp);
#endif
  memset(tmp, 0, 34);
  hmac256(tmp, signatureVersion, (int)strlen(signatureVersion), hex_tmp, (int)strlen(hex_tmp));
  hex_encode(hex_tmp, tmp, 32);
  hex_tmp[64] = 0x0;
#if defined(DEBUG_GATEWAY_PAYLOAD_SIGN)
  DBG("hex2: [%d]%s", strlen(hex_tmp), hex_tmp);
#endif
  hmac256(tmp, hex_tmp, strlen(hex_tmp), stringtoSign, strlen(stringtoSign));
  hex_encode(signature, tmp, 32);
  signature[64] = 0x0;
#if defined(DEBUG_GATEWAY_PAYLOAD_SIGN)
  DBG("sig: [%d]%s", strlen(signature), signature);
#endif
  ret  = 0;
  FREE_CHUNK(tmp);
tmp_error:
  FREE_CHUNK(hex_tmp);
hex_tmp_error:
  FREE_CHUNK(canonicalRequest);
  return ret;
}
