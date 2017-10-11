/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "ssl/md5sum.h"
#include <wolfssl/wolfcrypt/md5.h>

int __attribute__((weak)) md5sum(char *hash, const char *data, int len) {
  Md5 md5;
  wc_InitMd5(&md5);
  wc_Md5Update(&md5, (const byte*)data, (word32)len);
  wc_Md5Final(&md5, (byte*) hash);
  return 0;
}
#include <debug.h>
#include <arrow/utf8.h>
static Md5 md5;
static int count = 0;
void __attribute__((weak)) md5_chunk_init() {
    DBG("md5sum init");
    count = 0;
  wc_InitMd5(&md5);
}

void __attribute__((weak)) md5_chunk(const char *data, int len) {
    count += len;
    char tmp[34];
    hex_encode(tmp, data, 16);
    DBG("add %d %s", count, tmp);
  wc_Md5Update(&md5, (const byte*)data, (word32)len);

}

int md5_chunk_hash(char *hash) {
    DBG("md5sum chunk %d", count);
  wc_Md5Final(&md5, (byte*) hash);
  return MD5_DIGEST_SIZE;
}
