/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_SIGN_H_
#define ARROW_SIGN_H_

char *get_api_key(void);
char *get_secret_key(void);
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
