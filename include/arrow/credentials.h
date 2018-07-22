#if !defined(WIFI_CREDENTIALS_H_)
#define WIFI_CREDENTIALS_H_

#include <sys/type.h>

typedef struct _wifi_credentials {
    char *ssid;
    char *pass;
    uint32_t sec;
} wifi_credentials_t;

uint32_t credentials_qnt();
wifi_credentials_t *credentials_get(int index);
wifi_credentials_t *credentials_next();

typedef struct arrow_host_ {
    uint16_t scheme;
    uint16_t port;
    const char *host;
} arrow_host_t;

void arrow_hosts_init();

arrow_host_t *arrow_api_host(void);
arrow_host_t *arrow_mqtt_host(void);

#endif  // WIFI_CREDENTIALS_H_
