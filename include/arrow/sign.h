/*
 * sign.h
 *
 *  Created on: 13 окт. 2016 г.
 *      Author: ddemidov
 */

#ifndef ARROW_SIGN_H_
#define ARROW_SIGN_H_

//void sha256(char *shasum, char *buf, int size);
//void hmac256(char *hmacdig, const char *key, int key_size, const char *buf, int buf_size);
char *get_api_key();
void set_api_key(char *newkey);
void set_secret_key(char *newkey);
void sign(char *signature,
          const char* timestamp,
          const char *meth,
          const char *uri,
          const char *canQueryString,
          const char *payload,
          const char *apiVersion);

#endif /* ARROW_SIGN_H_ */
