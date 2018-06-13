#include "arrow/gateway_payload_sign.h"
#include <arrow/sign.h>
#include <ssl/crypt.h>
#include <sys/mem.h>
#include <debug.h>
#include <arrow/utf8.h>

#include <data/chunk.h>

int gateway_payload_sign(char *signature,
                         const char *hid,
                         const char *name,
                         int encrypted,
                         const char *canParString,
                         const char *signatureVersion) {
  int ret = -1;
  // step 1
  sha256_init();
  sha256_chunk(hid, strlen(hid));
  sha256_chunk("\n", 1);
  sha256_chunk(name, strlen(name));
  sha256_chunk("\n", 1);
  if ( encrypted ) {
      sha256_chunk("true\n", 5);
  } else {
      sha256_chunk("false\n", 6);
  }
  sha256_chunk(canParString, strlen(canParString));
  sha256_chunk("\n", 1);

  CREATE_CHUNK(hex_canreq, 66);
  CHECK_CHUNK(hex_canreq, goto hex_tmp_error);

  CREATE_CHUNK(tmp, 34);
  CHECK_CHUNK(tmp, goto tmp_error);

  sha256_fin(tmp);
  hex_encode(hex_canreq, tmp, 32);
  hex_canreq[64] = '\0';
#if defined(DEBUG_GATEWAY_PAYLOAD_SIGN)
  DBG("can: %s", hex_canreq);
#endif

  CREATE_CHUNK(hex_tmp, 66);
  CHECK_CHUNK(hex_tmp, goto hex_tmp_error);

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
  hmac256_init(hex_tmp, strlen(hex_tmp));
  hmac256_chunk(hex_canreq, strlen(hex_canreq));
  hmac256_chunk("\n", 1);
  hmac256_chunk(get_api_key(), strlen(get_api_key()));
  hmac256_chunk("\n", 1);
  hmac256_chunk(signatureVersion, strlen(signatureVersion));
  hmac256_fin(tmp);

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
  FREE_CHUNK(hex_canreq);
  return ret;
}
