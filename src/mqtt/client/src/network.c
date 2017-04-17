#include "mqtt/client/network.h"

#include <bsd/socket.h>
#include <debug.h>
#include <bsd/inet.h>
#if defined(__USE_STD__)
# include <errno.h>
#endif
#if defined(__XCC__)
#define WOLFSSL SSL
#define WOLFSSL_CTX SSL_CTX
#define IPPROTO_TCP 0
#define SSL_SUCCESS 0
#define SSL_INBUF_SIZE               6000
#define SSL_OTA_INBUF_SIZE           20000
#define SSL_OUTBUF_SIZE              3500
# define wolfSSL_write qcom_SSL_write
# define wolfSSL_read qcom_SSL_read
#define strerror(...) "xcc error"
#endif

static int _read(Network* n, unsigned char* buffer, int len, int timeout_ms) {
    struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
    if ((int)interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0)) {
        interval.tv_sec = 0;
        interval.tv_usec = 100;
    }

    setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

    int bytes = 0;
    int rc;
    while (bytes < len) {
#if defined(MQTT_CIPHER)
      rc = wolfSSL_read(n->ssl, buffer + bytes, (uint16_t)(len - bytes));
#else
      rc = recv(n->my_socket, (char*) buffer +bytes, (uint16_t)(len - bytes), 0);
#endif
//         if (rc) DBG("mqtt recv %d/%d", rc, len);
        if (rc < 0) {
#if defined(errno)
            DBG("error(%d): %s", rc, strerror(errno));
#endif
            bytes = -1;
            break;
        } else
            bytes += rc;
    }
    return bytes;
}


static int _write(Network* n, unsigned char* buffer, int len, int timeout_ms) {
    struct timeval tv = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};

    setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
    DBG("mqtt send %d", len);
    int rc = 0;
#if defined(MQTT_CIPHER)
    rc = wolfSSL_write(n->ssl, buffer, len);
#else
    rc = send(n->my_socket, (void*)buffer, (size_t)len, 0);
#endif
    return rc;
}


void NetworkInit(Network* n) {
    n->my_socket = -1;
    n->mqttread = _read;
    n->mqttwrite = _write;
#if !defined(__XCC__)
# if defined(MQTT_CIPHER)
    wolfSSL_Init();
//    wolfSSL_Debugging_ON();
    n->method = wolfTLSv1_2_client_method();
# endif
#endif
}

void NetworkDisconnect(Network* n) {
    soc_close(n->my_socket);
#if defined(__XCC__)
# if defined(MQTT_CIPHER)
    if (n->ssl) {
      qcom_SSL_shutdown(n->ssl);
      qcom_SSL_ctx_free(n->ctx);
    }
# endif
#else
# if defined(MQTT_CIPHER)
    if ( n->ctx ) wolfSSL_CTX_free(n->ctx);
    if ( n->ssl ) wolfSSL_free(n->ssl);
# endif
#endif
}

#if !defined(__XCC__) && defined(MQTT_CIPHER)
static int recv_ssl(WOLFSSL *wssl, char* buf, int sz, void* vp) {
  int *tcp = (int *)(vp);
//  DBG("ssl recv %d - %d", *tcp, sz);
  int timeout_ms = 5000;
  struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
  setsockopt(*tcp, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));
  int got = recv(*tcp, buf, sz, 0);
//  DBG("ssl got %d", got);
  if (got == 0)  return -2;  // IO_ERR_WANT_READ;
  return got;
}


static int send_ssl(WOLFSSL *wssl, char* buf, int sz, void* vp) {
  int *tcp = (int *)(vp);
//  DBG("ssl send %d - %d", *tcp, sz);
  int sent = send(*tcp, buf, sz, 0);
//  DBG("ssl sent %d", sent);
  if (sent == 0)
    return -2;  // IO_ERR_WANT_WRITE
  return sent;
}
#endif

int NetworkConnect(Network* n, char* addr, int port) {
    struct sockaddr_in serv;
    struct hostent *serv_resolve;
    DBG("resolve %s", addr);
    serv_resolve = gethostbyname(addr);
    if (serv_resolve == NULL) {
        DBG("MQTT ERROR: no such host %s", addr);
        return -1;
    }
    memset(&serv, 0, sizeof(serv));

    if (serv_resolve->h_addrtype == AF_INET) {
        serv.sin_family = PF_INET;
        bcopy((char *)serv_resolve->h_addr,
                (char *)&serv.sin_addr.s_addr,
                (size_t)serv_resolve->h_length);
        serv.sin_port = htons(port);
    } else
        return -1;
DBG("try socket");
    n->my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( n->my_socket < 0 ) {
        DBG("MQTT connetion fail %d", n->my_socket);
        return n->my_socket;
    }
    if ( connect(n->my_socket, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        soc_close(n->my_socket);
        return -2;
    }


#if defined(MQTT_CIPHER)
#if defined(__XCC__)
    n->ctx = qcom_SSL_ctx_new(SSL_CLIENT, SSL_INBUF_SIZE, SSL_OUTBUF_SIZE, 0);
    if ( n->ctx  == NULL) {
      DBG("unable to get ctx");
      soc_close(n->my_socket);
      return -1;
    }
    n->ssl = qcom_SSL_new(n->ctx);
    if (n->ssl == 0) {
      DBG("oops, bad SSL ptr")
      soc_close(n->my_socket);
      return -4;
    }
    int ret = qcom_SSL_set_fd(n->ssl, n->my_socket);
    if (ret < A_OK) {
      DBG("ERROR: Unable to add socket handle to SSL");
      soc_close(n->my_socket);
      return ret;
    }
    ret = qcom_SSL_connect(n->ssl);
    if (ret < 0) {
      DBG("SSL connect fail %d", ret);
      soc_close(n->my_socket);
      return ret;
    }
#else
    DBG("start cipher");
    n->ctx = wolfSSL_CTX_new(n->method);
    if ( n->ctx  == NULL) {
      DBG("unable to get ctx");
      soc_close(n->my_socket);
      return -1;
    }
    wolfSSL_CTX_set_verify(n->ctx, SSL_VERIFY_NONE, 0);
    wolfSSL_SetIORecv(n->ctx, recv_ssl);
    wolfSSL_SetIOSend(n->ctx, send_ssl);

    n->ssl = wolfSSL_new(n->ctx);
    if (n->ssl == 0) {
      DBG("oops, bad SSL ptr");
      soc_close(n->my_socket);
      return -1;
    }
    wolfSSL_SetIOReadCtx(n->ssl, (void*)&n->my_socket);
    wolfSSL_SetIOWriteCtx(n->ssl, (void*)&n->my_socket);
    DBG("set SSL rdwr context");
//    wolfSSL_Debugging_ON();
    int err = wolfSSL_connect(n->ssl);
    if (err != SSL_SUCCESS) {
      DBG("SSL connect fail %d", err);
      soc_close(n->my_socket);
      return -1;
    } else {
        DBG("SSL connect done");
    }

# endif
#endif

    return n->my_socket;
}
