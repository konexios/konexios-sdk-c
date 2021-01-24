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

typedef struct konexios_host_ {
    uint16_t scheme;
    uint16_t port;
    const char *host;
} konexios_host_t;

void konexios_hosts_init();

konexios_host_t *konexios_api_host(void);
konexios_host_t *konexios_mqtt_host(void);

#endif  // WIFI_CREDENTIALS_H_
