# Arrow Connect SDK #

### examples ###

On devices with disabled RTC possible to use NTP time setup:

ntp_set_time_cycle();

related defins in the config.h file:

#define NTP_DEFAULT_SERVER "0.pool.ntp.org"
#define NTP_DEFAULT_PORT 123
#define NTP_DEFAULT_TIMEOUT 4000

function will endlessly try to get current time through the net.

or use parametric function

ntp_set_time_common(const char *server, uint16_t port, int timeout, int try)
where
server - ntp server
port - ntp port
timeout - timeout for time setting
try - attempt to get time setting

### Register Gateway ###

defines for network connection:
#define ARROW_SCH "http"
#define ARROW_ADDR "pegasuskronos01-dev.cloudapp.net"
#define ARROW_PORT 28880

simple example:
arrow_gateway_t gateway;
while ( arrow_connect_gateway(&gateway) < 0) { printf("arrow gateway connection fail\r\n"); }
arrow_gateway_free(&gateway);

related defins in the config.h file:
#define GATEWAY_UID_PREFIX          "aris"
#define GATEWAY_NAME                "aris-gateway-demo"
#define GATEWAY_OS                  "ThreadX"
#define GATEWAY_TYPE                "Local"
#define GATEWAY_SOFTWARE_NAME       "eos"
#define GATEWAY_SOFTWARE_VERSION    "0.1"
This connect function automatically fill the gateway structure according a defines above.
Gateway UID scheme: GATEWAY_UIP_PREFIX-<MAC>
where MAC is device MAC-address

### Checkin Gateway ###

simple example:
arrow_gateway_t gateway;
while ( arrow_connect_gateway(&gateway) < 0) { printf("arrow gateway connection fail\r\n"); }
arrow_checkin(&gateway); // CHECKIN
arrow_gateway_free(&gateway);

### Heartbeat Gateway ###

simple example:
arrow_gateway_t gateway;
while ( arrow_connect_gateway(&gateway) < 0) { printf("arrow gateway connection fail\r\n"); }
arrow_heartbeat(&gateway); // HEARTBEAT
arrow_gateway_free(&gateway);

### Get Gateway Configuration ###
simple example:
arrow_gateway_t gateway;
char config[1024];
while ( arrow_connect_gateway(&gateway) < 0) { printf("arrow gateway connection fail\r\n"); }
arrow_config(&gateway, config);
printf("gateway config: %s\r\n");
arrow_gateway_free(&gateway);

### Register Device ###

simple example:
arrow_gateway_t gateway;
arrow_device_t device;
while ( arrow_connect_gateway(&gateway) < 0) { printf("arrow gateway connection fail\r\n"); }
while ( arrow_connect_device(&gateway, &device) < 0 ) { printf("arrow device connection fail\r\n"); } // Device registration
arrow_device_free(&device);
arrow_gateway_free(&gateway);

related defins in the config.h file:
#define DEVICE_NAME         "aris-device-demo"
#define DEVICE_TYPE         "aris-device"
#define DEVICE_UID_SUFFIX   "board"
This arrow_connect_device function automatically fill the gateway structure according a defines above.
Device UID scheme: GATEWAY_UIP_PREFIX-<MAC>-DEVICE_UID_SUFFIX
where MAC is device MAC-address

### Telemetry API ###

simple example:
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

where telemetry_data_type - device depens data type
for ARIS board it's sensor_data_t
for Nucleo IKS board it's X_NUCLEO_IKS01A1
for Nucleo SensorTile it's SensorTile type
for new data types should implement new telemetry_serialize function.


related defins in the config.h file (sensors depends):
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