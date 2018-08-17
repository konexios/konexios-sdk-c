#include "http_cb.h"
#include <string.h>

typedef struct __test_http_resp_t {
    const char *text;
    int http_size;
} test_http_resp_t;

static test_http_resp_t http_r[10];
static int resp_index = 0;
static int http_resp_cout = 0;

void set_http_cb(char *buf, int size) {
  http_r[0].text = buf;
  http_r[0].http_size = size;
  resp_index = 0;
  http_resp_cout = 0;
}

void add_http_cb(const char *buf, int size) {
    int i = 0;
    for ( i =0; i< 10 ; i++ ) {
        if ( !http_r[i].text && !http_r[i].http_size  ) {
            http_r[i].text = buf;
            http_r[i].http_size = size;
            break;
        }
    }
}

static char send_buffer[1024] = {0};
static int send_index = 0;

ssize_t send_cb(int sockfd, const void *buf, size_t len, int flags, int count) {
    (void)(sockfd);
    (void)(buf);
    (void)(flags);
    if ( len < sizeof(send_buffer) - send_index ) {
        memcpy(send_buffer + send_index, buf, len);
    } else {
        int chunk = sizeof(send_buffer) - send_index;
        memcpy(send_buffer + send_index, buf, chunk);
        memcpy(send_buffer, buf + chunk, len - chunk);
        send_index = len - chunk;
    }

//    printf("s[%s]\r\n", buf);
//    if ( strstr(send_buffer, "\r\n\r\n") == 0 ) {
//        memset(send_buffer, 0x0, sizeof(send_buffer));
//    }
    return (int)len;
}

ssize_t recv_cb(int sockfd, void *buf, size_t len, int flags, int count) {
    (void)(sockfd);
    (void)(buf);
    (void)(flags);
//    printf("--- %d (req %d) --- \r\n", http_resp_cout, len);
    if ( http_r[http_resp_cout].http_size <= 0 ) {
        if (http_r[http_resp_cout].http_size < 0 && !http_r[http_resp_cout].text) {
            http_resp_cout++;
        }
        return -1;
    }
    int size = http_r[http_resp_cout].http_size - resp_index;
    if ( size > len ) {
        size = len;
        memcpy(buf, http_r[http_resp_cout].text + resp_index, size);
        resp_index += size;
//        printf("r{%s} %d\r\n", buf, size);
    } else {
        if ( size <= 0 ) return -1;
        memcpy(buf, http_r[http_resp_cout].text + resp_index, size);
//         printf("r[%s] %d\r\n", buf, size);
        resp_index += size;
//        printf("---- last %d %d\r\n", resp_index, http_r[http_resp_cout].http_size);
        if ( resp_index == http_r[http_resp_cout].http_size &&
             http_resp_cout < 10 ) {
            // next expected message
            http_resp_cout++;
            resp_index = 0;
        }
    }
    return size;
}

ssize_t sendto_cb(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen, int count) {
    return send_cb(sockfd, buf, len, flags, count);
}

ssize_t recvfrom_cb(int sock, void *buf, size_t size, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen, int count) {
    return recv_cb(sock, buf, size, flags, count);
}
