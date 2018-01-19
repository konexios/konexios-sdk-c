#include "http_cb.h"
#include <string.h>

static char *http_r = NULL;
static int http_size = 0;

void set_http_cb(char *buf, int size) {
  http_r = buf;
  http_size = size;
}

ssize_t send_cb(int sockfd, const void *buf, size_t len, int flags, int count) {
    (void)(sockfd);
    (void)(buf);
    (void)(flags);
    return (int)len;
}

ssize_t recv_cb(int sockfd, void *buf, size_t len, int flags, int count) {
    (void)(sockfd);
    (void)(buf);
    (void)(flags);
    static int resp_count = 0;
//    int size = sizeof(http_resp_text) - resp_count;
    int size = http_size - resp_count;
    if ( size > len ) {
        size = len;
        memcpy(buf, http_r + resp_count, size);
        resp_count += size;
    } else {
        if ( size <= 0 ) return -1;
        memcpy(buf, http_r + resp_count, size);
        resp_count += size;
    }
    return size;
}
