# Arrow Connect SDK #

### examples ###

On devices with disabled RTC possible to use NTP time setup:
```c
ntp_set_time_cycle();
```
related defins in the config.h file:

```c
#define NTP_DEFAULT_SERVER "0.pool.ntp.org"
#define NTP_DEFAULT_PORT 123
#define NTP_DEFAULT_TIMEOUT 4000
```

function will endlessly try to get current time through the net.

or use parametric function

```c
ntp_set_time_common(const char *server, uint16_t port, int timeout, int try)
```
where
server - ntp server
port - ntp port
timeout - timeout for time setting
try - attempt to get time setting

### Register Gateway ###

defines for network connection:
```c
#define ARROW_SCH "http"
#define ARROW_ADDR "pegasuskronos01-dev.cloudapp.net"
#define ARROW_PORT 28880
```
simple example:
```c
arrow_gateway_t gateway;
while ( arrow_connect_gateway(&gateway) < 0) { printf("arrow gateway connection fail\r\n"); }
arrow_gateway_free(&gateway);
```
related defins in the config.h file:

```c
#define GATEWAY_UID_PREFIX          "aris"
#define GATEWAY_NAME                "aris-gateway-demo"
#define GATEWAY_OS                  "ThreadX"
#define GATEWAY_TYPE                "Local"
#define GATEWAY_SOFTWARE_NAME       "eos"
#define GATEWAY_SOFTWARE_VERSION    "0.1"
```

This connect function automatically fill the gateway structure according a defines above.
Gateway UID scheme: GATEWAY_UIP_PREFIX-<MAC>
where MAC is device MAC-address

### Checkin Gateway ###

simple example:

```c
arrow_gateway_t gateway;
while ( arrow_connect_gateway(&gateway) < 0) { printf("arrow gateway connection fail\r\n"); }
arrow_checkin(&gateway); // CHECKIN
arrow_gateway_free(&gateway);
```
### Heartbeat Gateway ###

simple example:

```c
arrow_gateway_t gateway;
while ( arrow_connect_gateway(&gateway) < 0) { printf("arrow gateway connection fail\r\n"); }
arrow_heartbeat(&gateway); // HEARTBEAT
arrow_gateway_free(&gateway);
```

### Get Gateway Configuration ###
simple example:

```c
arrow_gateway_t gateway;
char config[1024];
while ( arrow_connect_gateway(&gateway) < 0) { printf("arrow gateway connection fail\r\n"); }
arrow_config(&gateway, config);
printf("gateway config: %s\r\n");
arrow_gateway_free(&gateway);
```

### Register Device ###

simple example:

```c
arrow_gateway_t gateway;
arrow_device_t device;
while ( arrow_connect_gateway(&gateway) < 0) { printf("arrow gateway connection fail\r\n"); }
while ( arrow_connect_device(&gateway, &device) < 0 ) { printf("arrow device connection fail\r\n"); } // Device registration
arrow_device_free(&device);
arrow_gateway_free(&gateway);
```

related defins in the config.h file:

```c
#define DEVICE_NAME         "aris-device-demo"
#define DEVICE_TYPE         "aris-device"
#define DEVICE_UID_SUFFIX   "board"
```

This arrow_connect_device function automatically fill the gateway structure according a defines above.
Device UID scheme: GATEWAY_UIP_PREFIX-<MAC>-DEVICE_UID_SUFFIX
where MAC is device MAC-address

### Telemetry API ###

simple example:

```c
arrow_gateway_t gateway;
arrow_device_t device;
telemetry_data_type *data;
while ( arrow_connect_gateway(&gateway) < 0) { printf("arrow gateway connection fail\r\n"); }
while ( arrow_connect_device(&gateway, &device) < 0 ) { printf("arrow device connection fail\r\n"); } // Device registration
data = get_telemetry_data_from_sensors();
while ( arrow_send_telemetry(&device, data) < 0 ) { printf("send telemetry fail\r\n") }
arrow_device_free(&device);
arrow_gateway_free(&gateway);
free(data);
```

where telemetry_data_type - device depens data type
for ARIS board it's sensor_data_t
for Nucleo IKS board it's X_NUCLEO_IKS01A1
for Nucleo SensorTile it's SensorTile type
for new data types should implement new telemetry_serialize function.


related defines in the config.h file (sensors depends):

```c
#define TELEMETRY_DEVICE_HID        "_|deviceHid"
#define TELEMETRY_TEMPERATURE       "f|temperature"
#define TELEMETRY_HUMIDITY          "f|humidity"
#define TELEMETRY_BAROMETER         "f|barometer"
#define TELEMETRY_ACCELEROMETER_X   "f|accelerometerX"
#define TELEMETRY_ACCELEROMETER_Y   "f|accelerometerY"
#define TELEMETRY_ACCELEROMETER_Z   "f|accelerometerZ"
#define TELEMETRY_GYROMETER_X       "f|gyrometerX"
#define TELEMETRY_GYROMETER_Y       "f|gyrometerY"
#define TELEMETRY_GYROMETER_Z       "f|gyrometerZ"
#define TELEMETRY_MAGNETOMETER_X    "f|magnetometerX"
#define TELEMETRY_MAGNETOMETER_Y    "f|magnetometerY"
#define TELEMETRY_MAGNETOMETER_Z    "f|magnetometerZ"
#define TELEMETRY_DELAY             5000


### Test Suite ###

For a test suite creation you need to know the testProcedureHid.
Example:

CREATE_TEST_SUITE(gate_test, "384f9d6832ce2272f18f3ee8597e0f108f6a8109");
arrow_test_gateway(current_gateway(), &gate_test);

    sleep(60);

CREATE_TEST_SUITE(p, "a53f0aa3e8bf7806ff5b8770ad4d9d3477d534c9");
arrow_test_device(current_device(), &p);
printf("test device result hid {%s}\r\n", P_VALUE(p.result_hid));

arrow_test_begin(&p);
// start test procedure
arrow_test_step_begin(&p, 1);
//test temperature
get_telemetry_data(&data);
if ( sizeof(data) > 1 ) {
  arrow_test_step_success(&p, 1);
} else {
  arrow_test_step_fail(&p, 1, "no temp sensor");
}

#if !defined(SKIP_LED)
// where is no LED, skiping...
arrow_test_step_skip(&p, 2);
#else
arrow_test_step_begin(&p, 2);
arrow_test_step_fail(&p, 2, "no LED");
#endif

arrow_test_step_begin(&p, 3);
// check ARM
#if defined(__arm__)
arrow_test_step_success(&p, 3);
#else
arrow_test_step_fail(&p, 3, "not ARM");
#endif
// end test
arrow_test_end(&p);


### OTA firmware upgrade ###

There is a capability to update the firmware via an SDK. It's sufficiantly to implement the int arrow_gateway_software_update(const char *url)  function into your application. Where url it is URL address passed through HTTP software update request. The content of this function is platform depend.
For example for the linux:
#include <stdio.h>
#include <curl/curl.h>

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}
int arrow_gateway_software_update(const char *url) {
  CURL *curl;
  static const char *pagefilename = "update.file";
  FILE *pagefile;

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  pagefile = fopen(pagefilename, "wb");
  if(pagefile) {
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);
    curl_easy_perform(curl);
    fclose(pagefile);
  }
  curl_easy_cleanup(curl);
  return 0;
}


This function for the QCA4010 board is more complicated. You can find it into acn-embedded  xtensa directory.
The JSON message for a QCA update should be like this:
{
  "url": "http://192.168.83.129:80/ota_image_AR401X_REV6_IOT_MP1_hostless_unidev_singleband_iot_arrow.bin"
}
or
{
  "url": "tftp://192.168.83.129/ota_image_AR401X_REV6_IOT_MP1_hostless_unidev_singleband_iot_arrow.bin"
}



