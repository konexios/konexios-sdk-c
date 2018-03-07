#include "arrow/routine.h"
#include <config.h>
#include <debug.h>
#include <time/time.h>
#include <time/watchdog.h>
#include <http/routine.h>
#include <arrow/mqtt.h>
#include <arrow/events.h>
#include <arrow/state.h>
#include <arrow/device_command.h>
#include <arrow/mqtt.h>
#include <arrow/api/gateway/gateway.h>
#include <arrow/api/device/device.h>
#include <arrow/telemetry_api.h>
#include <arrow/storage.h>

#define GATEWAY_CONNECT "Gateway connection [%s]"
#define GATEWAY_CONFIG "Gateway config [%s]"
#define DEVICE_CONNECT "Device connection [%s]"
#define DEVICE_TELEMETRY "Device telemetry [%s]"
#define DEVICE_MQTT_CONNECT "Device mqtt connection [%s]"
#define DEVICE_MQTT_TELEMETRY "Device mqtt telemetry [%s]"

static arrow_gateway_t _gateway;
static arrow_gateway_config_t _gateway_config;
static arrow_device_t _device;
static int _init_done = 0;
static int _init_mqtt = 0;

arrow_device_t *current_device(void) {
  return &_device;
}

arrow_gateway_t *current_gateway(void) {
  return &_gateway;
}

int arrow_connect_gateway(arrow_gateway_t *gateway){
  arrow_prepare_gateway(gateway);
  int ret = restore_gateway_info(gateway);
  if ( ret < 0 ) {
    // new registration
    if ( arrow_register_gateway(gateway) < 0 ) {
      return -1;
    }
    save_gateway_info(gateway);
  } else {
    // hid already set
    DBG("gateway checkin hid %s", P_VALUE(gateway->hid));
    return arrow_gateway_checkin(gateway);
  }
  return 0;
}

int arrow_connect_device(arrow_gateway_t *gateway, arrow_device_t *device) {
  arrow_prepare_device(gateway, device);
  if ( restore_device_info(device) < 0 ) {
    if ( arrow_register_device(gateway, device) < 0 ) return -1;
    save_device_info(device);
  } else {
      device_info_t list;
      if ( arrow_device_find_by_hid(&list, P_VALUE(device->hid)) < 0 ) {
          return -1;
      } else {
          if ( list.enabled ) {
              DBG("device: %s", P_VALUE(list.name));
          }
          device_info_free(&list);
      }
  }
  return 0;
}

arrow_routine_error_t arrow_initialize_routine(void) {
  wdt_feed();
  int retry = 0;
  http_session_close_set(current_client(), false);
  DBG("register gateway via API");
  while ( arrow_connect_gateway(&_gateway) < 0 ) {
      RETRY_UP(retry, {return ROUTINE_ERROR;});
      DBG(GATEWAY_CONNECT, "fail");
      msleep(ARROW_RETRY_DELAY);
  }
  DBG(GATEWAY_CONNECT, "ok");

  wdt_feed();
  RETRY_CR(retry);
  while ( arrow_gateway_config(&_gateway, &_gateway_config) < 0 ) {
    RETRY_UP(retry, {return ROUTINE_ERROR;});
    DBG(GATEWAY_CONFIG, "fail");
    msleep(ARROW_RETRY_DELAY);
  }
  DBG(GATEWAY_CONFIG, "ok");

  // device registaration
  wdt_feed();
  RETRY_CR(retry);
  DBG("register device via API");
  // close session after next request
  http_session_close_set(current_client(), true);
  while ( arrow_connect_device(&_gateway, &_device) < 0 ) {
    RETRY_UP(retry, {return ROUTINE_ERROR;});
    DBG(DEVICE_CONNECT, "fail");
    msleep(ARROW_RETRY_DELAY);
  }
  DBG(DEVICE_CONNECT, "ok");
  _init_done = 1;
  return ROUTINE_SUCCESS;
}

arrow_routine_error_t arrow_update_state(const char *name, const char *value) {
  add_state(name, value);
  if ( _init_done ) {
    arrow_post_state_update(&_device);
    return ROUTINE_SUCCESS;
  }
  return ROUTINE_ERROR;
}

arrow_routine_error_t arrow_send_telemetry_routine(void *data) {
    if ( !_init_done ) return ROUTINE_NOT_INITIALIZE;
    wdt_feed();
    int retry = 0;
    while ( arrow_send_telemetry(&_device, data) < 0) {
        RETRY_UP(retry, {return ROUTINE_ERROR;});
        DBG(DEVICE_TELEMETRY, "fail");
        msleep(ARROW_RETRY_DELAY);
    }
    DBG(DEVICE_TELEMETRY, "ok");
    return ROUTINE_SUCCESS;
}

arrow_routine_error_t arrow_mqtt_connect_routine(void) {
  if ( !_init_done ) return ROUTINE_NOT_INITIALIZE;
  // init MQTT
  DBG("mqtt connect...");
  int retry = 0;
  while ( mqtt_telemetry_connect(&_gateway, &_device, &_gateway_config) < 0 ) {
      RETRY_UP(retry, {return ROUTINE_MQTT_CONNECT_FAILED;});
      DBG(DEVICE_MQTT_CONNECT, "fail");
      msleep(ARROW_RETRY_DELAY);
  }
  DBG(DEVICE_MQTT_CONNECT, "ok");

  arrow_state_mqtt_run(&_device);
  RETRY_CR(retry);
#if !defined(NO_EVENTS)
  while(mqtt_subscribe_connect(&_gateway, &_device, &_gateway_config) < 0 ) {
      RETRY_UP(retry, {return ROUTINE_MQTT_SUBSCRIBE_FAILED;});
      DBG(DEVICE_MQTT_CONNECT, "fail");
      msleep(ARROW_RETRY_DELAY);
  }
  RETRY_CR(retry);
  while( mqtt_subscribe() < 0 ) {
      RETRY_UP(retry, {return ROUTINE_MQTT_SUBSCRIBE_FAILED;});
      DBG(DEVICE_MQTT_CONNECT, "fail");
      msleep(ARROW_RETRY_DELAY);
  }
#endif
  _init_mqtt = 1;
  return ROUTINE_SUCCESS;
}

arrow_routine_error_t arrow_mqtt_disconnect_routine() {
    if ( _init_mqtt ) {
        if ( mqtt_is_connect() )
            mqtt_disconnect();
        _init_mqtt = 0;
        return ROUTINE_SUCCESS;
    }
    return ROUTINE_ERROR;
}

arrow_routine_error_t arrow_mqtt_send_telemetry_routine(get_data_cb data_cb, void *data) {
  if ( !_init_done || !_init_mqtt ) {
      DBG(DEVICE_MQTT_TELEMETRY, "Cloud not initialize");
      return ROUTINE_NOT_INITIALIZE;
  }
  wdt_feed();
  while (1) {
#if defined(NO_EVENTS)
      msleep(TELEMETRY_DELAY);
#else
      mqtt_yield(TELEMETRY_DELAY);
#endif
      int get_data_result = data_cb(data);
      if ( get_data_result < 0 ) {
          DBG(DEVICE_MQTT_TELEMETRY, "Fail to get telemetry data");
          return ROUTINE_GET_TELEMETRY_FAILED;
      } else if (get_data_result > 0 ) {
          // skip this
          continue;
      }
      wdt_feed();
      if ( mqtt_publish(&_device, data) < 0 ) {
          DBG(DEVICE_MQTT_TELEMETRY, "fail");
          return ROUTINE_MQTT_PUBLISH_FAILED;
      }
#if defined(VALGRIND_TEST)
      static int count = 0;
      if ( count++ > VALGRIND_TEST ) break;
#endif
#if defined(DEBUG)
    else {
      DBG(DEVICE_MQTT_TELEMETRY, "ok");
    }
#endif
  }
  return ROUTINE_SUCCESS;
}

void arrow_close(void) {
  arrow_mqtt_disconnect_routine();
  if ( _init_done ) {
    arrow_device_free(&_device);
    arrow_gateway_free(&_gateway);
    arrow_gateway_config_free(&_gateway_config);
    _init_done = 0;
  }
}

