#ifndef _ARROW_INCLUDE_CRYPT_SHA256_H_
#define _ARROW_INCLUDE_CRYPT_SHA256_H_

void sha256(char *shasum, char *buf, int size);
void hmac256(char *hmacdig, const char *key, int key_size, const char *buf, int buf_size);

#endif // _ARROW_INCLUDE_CRYPT_SHA256_H_
