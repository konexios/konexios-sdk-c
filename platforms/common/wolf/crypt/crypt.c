#include "crypt/crypt.h"

#include "wolfssl/wolfcrypt/sha256.h"
#include "wolfssl/wolfcrypt/hmac.h"

void sha256(char *shasum, char *buf, int size) {
  Sha256 sh;
  wc_InitSha256(&sh);
  wc_Sha256Update(&sh, (byte*)buf, (word32)size);
  wc_Sha256Final(&sh, (byte*)shasum);
}

void hmac256(char *hmacdig, const char *key, int key_size, const char *buf, int buf_size) {
  Hmac hmac;
  wc_HmacSetKey(&hmac, SHA256, (const byte*)key, (word32)key_size);
  wc_HmacUpdate(&hmac, (const byte*)buf, (word32)buf_size);
  wc_HmacFinal(&hmac, (byte*)hmacdig);
}
