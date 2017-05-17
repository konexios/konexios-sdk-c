#if !defined(ARROW_ERROR_DELAY)
#define ARROW_RETRY_DELAY 3000
#endif

#include "arrow/connection.h"
#include <config.h>
#include <debug.h>
#include <time/time.h>
#include <time/watchdog.h>
#include <arrow/mqtt.h>
#include <arrow/events.h>
#include <arrow/state.h>
#include <arrow/mqtt.h>


static arrow_gateway_t _gateway;
static arrow_gateway_config_t _gateway_config;
static arrow_device_t _device;
static int _init_done = 0;
static int _init_mqtt = 0;

int arrow_initialize_routine() {
  wdt_feed();
  DBG("register gateway via API %p", &_gateway);
  while ( arrow_connect_gateway(&_gateway) < 0 ) {
    DBG("arrow gateway connection fail...");
    msleep(ARROW_RETRY_DELAY);
  }

  wdt_feed();
  while ( arrow_config(&_gateway, &_gateway_config) < 0 ) {
    DBG("arrow gateway config fail...");
    msleep(ARROW_RETRY_DELAY);
  }

  // device registaration
  wdt_feed();
  DBG("register device via API");
  while ( arrow_connect_device(&_gateway, &_device) < 0 ) {
    DBG("arrow: device connection fail...");
    msleep(ARROW_RETRY_DELAY);
  }
  _init_done = 1;
  return 0;
}

int arrow_update_state(const char *name, const char *value) {
  add_state(name, value);
  if ( _init_done ) {
    arrow_post_state_update(&_device);
    return 0;
  }
  return -1;
}

int arrow_send_telemetry_routine(void *data) {
  if ( !_init_done ) return -1;
  wdt_feed();
  while ( arrow_send_telemetry(&_device, data) < 0) {
    DBG("arrow: send telemetry fail...");
    msleep(ARROW_RETRY_DELAY);
  }
  return 0;
}

int arrow_mqtt_connect_routine() {
  if ( !_init_done ) return -1;
  // init MQTT
  DBG("mqtt connect...");
  while ( mqtt_connect(&_gateway, &_device, &_gateway_config) < 0 ) {
    DBG("mqtt connect fail...");
    msleep(ARROW_RETRY_DELAY);
  } //every 3sec try to connect

  arrow_state_mqtt_run(&_device);
  if ( has_cmd_handler() >= 0 )  mqtt_subscribe();
  _init_mqtt = 1;

  return 0;
}

void arrow_mqtt_send_telemetry_routine(get_data_cb data_cb, void *data) {
  if ( !_init_done || !_init_mqtt ) return;
  while (1) {
    if ( has_cmd_handler() < 0 ) {
      msleep(TELEMETRY_DELAY);
    } else {
      mqtt_yield(TELEMETRY_DELAY);
    }
    if ( data_cb(data) < 0 ) continue;
    wdt_feed();
    if ( mqtt_publish(&_device, data) < 0 ) {
      DBG("mqtt publish failure...");
      mqtt_disconnect();
      while (mqtt_connect(&_gateway, &_device, &_gateway_config) < 0) { msleep(ARROW_RETRY_DELAY);}
      if ( has_cmd_handler() >= 0 ) mqtt_subscribe();
    }
  }
}

void arrow_close() {
  if ( _init_mqtt ) {
    mqtt_disconnect();
    _init_mqtt = 0;
  }
  if ( _init_done ) {
    arrow_device_free(&_device);
    arrow_gateway_free(&_gateway);
    _init_done = 0;
  }
}