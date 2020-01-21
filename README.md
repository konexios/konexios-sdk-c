# Konexios SDK #

### private data ###

All information concerned platform should be collect into private.h file in the SDK root directory.
It's strongly recommended define all environment variable to build correct firmware.
Example:

```c
#ifndef ACN_SDK_C_PRIVATE_H_
#define ACN_SDK_C_PRIVATE_H_

#define DEFAULT_API_KEY             "abc"
#define DEFAULT_SECRET_KEY          "xyz"
//#define DEV_ENV
//#define HTTP_DEBUG

#define DEFAULT_WIFI_SSID           "yourSSID"
#define DEFAULT_WIFI_PASS           "password"
#define DEFAULT_WIFI_SEC            0x00040003

/* gateway */
#define GATEWAY_UID_PREFIX          "QCA"
#define GATEWAY_NAME                "QCA-gateway-demo"
#define GATEWAY_OS                  "ThreadX"

/* gateway firmware */
#define GATEWAY_SOFTWARE_NAME       "SX-ULPGN-EVK-TEST-FW"
#define GATEWAY_SOFTWARE_VERSION    "1.2.4"

/* device */
#define DEVICE_NAME                 "ULPGN"
#define DEVICE_TYPE                 "SX_ULPGN"
#define DEVICE_UID_SUFFIX           "devkit"
```

### Defines ###
You can define this options in the private.h file or use "-Dxxx" compiler flag

define NO_EVENTS            to switch off the event handlers for a mqtt connection

define NO_RELEASE_UPDATE    turn off the firmware update capability (based on a arrow_software_release_dowload_set_cb functions)

define NO_SOFTWARE_UPDATE   turn off the software update capability (based on a arrow_gateway_software_update_set_cb function)

define ARCH_MEM             use the platform specific header with memory functions (bzero, bcopy, strcpy, strncpy etc) in a ${SDK_IMPL}/sys/arch/mem.h (if your platform need the implementation this one)

define ARCH_TYPE            use the platform specific header with common types (uint8_t, uint16_t etc) in a ${SDK_IMPL}/sys/arch/type.h

define ARCH_SOCK            use the platform specific header with socket layer additional headers and definitions (if needed) in a ${SDK_IMPL}/sys/arch/socket.h

define ARCH_SSL             use the wolfSSL settings file ${SDK_IMPL}/sys/arch/ssl.h (if you don't reimplement ssl functions and use default ssl_connect, ssl_recv etc)

define ARCH_TIME            use the platform specific headers or define needed types for common time functions (struct tm etc)

### examples ###

On devices with disabled RTC possible to use NTP time setup:
```c
ntp_set_time_cycle();
```
related defins in the config/ntp.h file:

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

### Find Gateway ###
```c
gateway_info_t *list = NULL;
int r = arrow_gateway_find_by(&list, 2, find_by(osNames, "mbed"), find_by(f_size, "100"));
if ( r == 0 ) {
  gateway_info_t *tmp;
  for_each_node_hard(tmp, list, gateway_info_t) {
  std::cout<<"hid:\t\t"<<P_VALUE(tmp->hid)<<std::endl
           <<"createdBy:\t"<<P_VALUE(tmp->createdBy)<<std::endl
           <<"lastModifiedBy:\t"<<P_VALUE(tmp->lastModifiedBy)<<std::endl
           <<"uid:\t\t"<<P_VALUE(tmp->uid)<<std::endl
           <<"name:\t\t"<<P_VALUE(tmp->name)<<std::endl
           <<"type:\t\t"<<P_VALUE(tmp->type)<<std::endl
           <<"deviceType:\t"<<P_VALUE(tmp->deviceType)<<std::endl
           <<"osName:\t\t"<<P_VALUE(tmp->osName)<<std::endl
           <<"softwareName:\t"<<P_VALUE(tmp->softwareName)<<std::endl
           <<"softwareVersion:\t"<<P_VALUE(tmp->softwareVersion)<<std::endl;
  gateway_info_free(tmp);
  free(tmp);
  }
}
```

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
related defins in the private.h file:

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

### Default Initialization ###
The Default Initialization include establishing a Gateway Connection and Configuration and Device Connection if it's necessary. This function implemeted all logic to initialize your device in a cloud. 

```c
arrow_initialize_routine();
```

To access the Gateway and Device objects you can use following function:
arrow_gateway_t *current_gateway();
arrow_device_t *current_device();

### Checkin Gateway ###

It's not necessary to use this functions if you already initialize your device by the arrow_initialize_routine function.

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

It's not necessary to use this functions if you already initialize your device by the arrow_initialize_routine function.

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

It's not necessary to use this functions if you already initialize your device by the arrow_initialize_routine function.

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

### Commands ###

You can to set your own command handler. At first you need to declare the handler function: int (*)(const char *str), where str - string with a JSON body. If the command processing are succeeded this handler shoud return 0.
After this you should register the command handler before the gateway initializing.

Example:
```c
static int test_cmd_proc(const char *str) {
  printf("test: [%s]", str);
  return 0;
}
// ... in the main function:
add_cmd_handler("test", &test_cmd_proc);
```

### Telemetry API ###

Simple example:

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
```

Or you can to replace this code by:
```c
arrow_send_telemetry_routine(&data);
```

Also you may initialize the MQTT processor and run the telemetry loop.
For MQTT initializing:
```c
arrow_mqtt_connect_routine();
```
After that you can run loop:
```c
arrow_mqtt_send_telemetry_routine(get_telemetry_data, &data);
```
where get_telemetry_data - function that fill a telemetry data object;
data - argument for this function

And if the command handler was installed loop will wait the mqtt commands between telemetry sending activities.

### Test Suite ###

For a test suite creation you need to know the testProcedureHid.

The gateway example:

CREATE_TEST_SUITE(gate_test, "384f9d6832ce2272f18f3ee8597e0f108f6a8109");
arrow_test_gateway(current_gateway(), &gate_test);

arrow_test_begin(&gate_test);
// start test procedure
arrow_test_step_begin(&gate_test, 1);
//test temperature
if ( 1 ) {
  arrow_test_step_success(&gate_test, 1);
} else {
  arrow_test_step_fail(&gate_test, 1, "no temp sensor");
}
// end test
arrow_test_end(&gate_test);


The device example: 

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

### Software Update ###

You can set the callback for a Software Update ability.
Just implement this function somewhere:

```c
int arrow_software_update(const char *url,
                          const char *checksum,
                          const char *from,
                          const char *to)
```
where url - is an address of the new firmware
checksum is the md5sum of the new firmware
from - old firmware version
to - new firmware version

This function should to return 0 if the update process succeeded.

And in the main function you chould to install this callback this way:
```c
arrow_software_release_set_cb(&arrow_software_update);
```

### OTA firmware upgrade ###

There is a capability to update the firmware via an SDK. It's sufficiantly to implement the callback function for an upgrade procedure and set this one:

// somewere in the main function:
#include <arrow/software_update.h>

int qca_gateway_software_update(const char *url) {
  // upgrade procedure
  return 0;
}

// ...

int main() {
  // ...
  arrow_gateway_software_update_set_cb(qca_gateway_software_update);
  // ...
}

Or reimplement the arrow_gateway_software_update function:

int arrow_gateway_software_update(const char *url)  function into your application. Where url it is URL address passed through HTTP software update request. The content of this function is platform depend.
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


