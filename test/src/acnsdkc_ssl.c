#include <ssl/ssl.h>
#include <bsd/socket.h>

int ssl_connect(int sock) {
    (void)(sock);
    return 0;
}

int ssl_recv(int sock, char *data, int len) {
    return recv(sock, data, len, 0);
}

int ssl_send(int sock, char* data, int length) {
    return send(sock, data, length, 0);
}

int ssl_close(int sock) {
    (void)(sock);
    return 0;
}
