#include "konexios/routine.h"
#include <konexios_config.h>
#include <debug.h>
#include <time/time.h>
#include <sys/watchdog.h>
#include <http/routine.h>
#include <konexios/mqtt.h>
#include <konexios/events.h>
#include <konexios/state.h>
#include <konexios/device_command.h>
#include <konexios/mqtt.h>
#include <konexios/api/gateway/gateway.h>
#include <konexios/api/device/device.h>
#include <konexios/telemetry_api.h>
#include <konexios/storage.h>
#include <json/property_json.h>
#include <json/aob.h>

// Types and definitions
// ---------------------------------------------------------------------------

#if defined(ARROW_INFO)
#define ARROW_INF  printf
#else
#define ARROW_INF(...)
#endif

#if defined(ARROW_DEBUG)
#define ARROW_DBG       DBG
#else
#define ARROW_DBG(...)
#endif

#define TRACE(...)  DBG(__VA_ARGS__)

#define GATEWAY_CONNECT "Gateway connection [%s]"
#define GATEWAY_CONFIG "Gateway config [%s]"
#define DEVICE_CONNECT "Device connection [%s]"
#define DEVICE_TELEMETRY "Device telemetry [%s]"
#define DEVICE_MQTT_CONNECT "Device mqtt connection [%s]"
#define DEVICE_MQTT_TELEMETRY "Device mqtt telemetry [%s]"

enum MQTT_INIT_FLAGS {
    MQTT_INIT_SYSTEM_DONE = 0x01,
    MQTT_INIT_SUBSCRIBE_DONE   = 0x02
};

// Variables
// ---------------------------------------------------------------------------

static konexios_gateway_t _gateway;
static konexios_gateway_config_t _gateway_config;
static konexios_device_t _device;
static int acn_register_init_done = 0;
static int acn_mqtt_init_flags = 0;

// Public functions
// ---------------------------------------------------------------------------

konexios_device_t *konexios_get_current_device(void) {
    TRACE("Enter");
  return &_device;
}

konexios_gateway_t *konexios_get_current_gateway(void) {
    TRACE("Enter");
  return &_gateway;
}

konexios_gateway_config_t *konexios_get_current_gateway_config(void) {
    TRACE("Enter");
  return &_gateway_config;
}

// Initialize the http and mqtt subsystems
konexios_routine_error_t konexios_init(void) {
  property_types_init();
  property_type_add(property_type_get_json());
  property_type_add(property_type_get_aob());
  konexios_hosts_init();
  konexios_gateway_init(&_gateway);
  konexios_device_init(&_device);

  if ( http_init() < 0 ) {
    return ROUTINE_ERROR;
  }
#if !defined(NO_EVENTS)
  konexios_mqtt_events_init();
#endif
  return ROUTINE_SUCCESS;
}

konexios_routine_error_t konexios_deinit(void) {
#if !defined(NO_EVENTS)
  konexios_mqtt_events_done();
#endif
  konexios_state_deinit();
  http_done();
  property_types_deinit();
  return ROUTINE_SUCCESS;
}

// Init, load, and Register/Checkin gateway
int konexios_connect_gateway(konexios_gateway_t *gateway, bool update_gateway_info)
{
    TRACE("Enter");
    int ret;

    // Init gateway
  konexios_prepare_gateway(gateway);
  if ( IS_EMPTY(gateway->hid) )
  {
      ret = restore_gateway_info(gateway);

      if ( ret < 0 )
      {
          // new registration
        ARROW_INF("ACN: New gateway registration\n");
        if ( konexios_register_gateway(gateway) < 0 )
        {
            // If we fail, return the fail code
            int rc = http_last_response_code();
            // 0 means the request didn't finish, 200==ok, 4xx==error, etc
            if(rc!=200)
                return (-1*rc);
            return rc;
        }
        // Success, save this info
        save_gateway_info(gateway);

      } else {
        // hid already set so checkin gateway
        ARROW_INF("ACN: Check in gateway\n");
        ARROW_DBG("gateway checkin hid %s", P_VALUE(gateway->hid));
        if(konexios_gateway_checkin(gateway)<0)
        {
            ARROW_INF("ACN: Error in checkin\n");
            // If we fail, return the fail code
            int rc = http_last_response_code();
            // 0 means the request didn't finish, 200==ok, 4xx==error, etc
            if(rc!=200)
                return (-1*rc);
            return rc;
        }
      }
  }

    // If we need to send the gateway 'update' request
    // do it here
    if(update_gateway_info)
    {
        ARROW_INF("ACN: Updating gateway info\n");
        if(konexios_gateway_update(gateway)<0)
        {
            // If we fail, return the fail code
            int rc = http_last_response_code();
            // 0 means the request didn't finish, 200==ok, 4xx==error, etc
            if(rc!=200)
                return (-1*rc);
            return rc;
        }
    }

    return 200;
}

int konexios_connect_device(konexios_gateway_t *gateway, konexios_device_t *device, bool update_device_info) {
  int ret = 0;
  konexios_prepare_device(gateway, device);
  if ( !IS_EMPTY(device->hid) )
      return ROUTINE_SUCCESS;
  if ( restore_device_info(device) < 0 ) {
        ARROW_INF("ACN: Register device\n");
        if ( konexios_register_device(gateway, device) < 0 )
        {
            // If we fail, return the fail code
            int rc = http_last_response_code();
            // 0 means the request didn't finish, 200==ok, 4xx==error, etc
            if(rc!=200)
                return (-1*rc);
            return rc;
    }
    save_device_info(device);
  }else{
    ARROW_INF("ACN: Device already registered\n");
    //return 200;
  }

    // Send the device config????
    // If we need to send the gateway 'update' request
    // do it here
    if(update_device_info)
    {
        ARROW_INF("ACN: Updating device info\n");
        if(konexios_device_update(gateway, device)<0)
        {
            // If we fail, return the fail code
            int rc = http_last_response_code();
            // 0 means the request didn't finish, 200==ok, 4xx==error, etc
            if(rc!=200)
                return (-1*rc);
            return rc;
        }
    }

  return 200;
}

// Do the initialization just for the gateway
#if 0
konexios_routine_error_t konexios_gateway_initialize_routine(void) {
    TRACE("Enter");
    int retry;

    // Keep the socket open after next HTTP transfer
    http_session_keep_active(true);
    retry = 0;

    // Do the gateway connect logic
    while ( konexios_connect_gateway(&_gateway,false) < 0 ) {
    RETRY_UP(retry, {return ROUTINE_ERROR;});
        ARROW_DBG(GATEWAY_CONNECT, "fail");
    msleep(ARROW_RETRY_DELAY);
  }
    ARROW_DBG(GATEWAY_CONNECT, "ok");

    // Close the socket after next HTTP transfer
    http_session_keep_active(false);

    // Get the gateway config
    retry=0;
  while ( konexios_gateway_config(&_gateway, &_gateway_config) < 0 ) {
    RETRY_UP(retry, {return ROUTINE_ERROR;});
      ARROW_DBG(GATEWAY_CONFIG, "fail");
    msleep(ARROW_RETRY_DELAY);
  }
    ARROW_DBG(GATEWAY_CONFIG, "ok");

    // Mark as intialized and done
    acn_register_init_done = 1;
  return ROUTINE_SUCCESS;
}
#endif


// Do the initialization for the gateway and device
konexios_routine_error_t konexios_initialize_routine(bool update_gateway_info)
{
    TRACE("Enter");
  int retry = 0;
  int ret=0;

    // Keep socket connections alive for all HTTP transfers
    http_session_keep_active(true);

    // Connect to Arrow Connect
    // (Do gateway register/checkin)
    while ( konexios_connect_gateway(&_gateway, update_gateway_info) < 0 ) {
      RETRY_UP(retry, {return ROUTINE_ERROR;});
      ARROW_DBG(GATEWAY_CONNECT, "fail");
    msleep(ARROW_RETRY_DELAY);
  }
    ARROW_DBG(GATEWAY_CONNECT, "ok");

    // Get gateway config
    retry=0;
    ARROW_INF("ACN: Get gateway config\n");
    while ( konexios_gateway_config(&_gateway, &_gateway_config) < 0 ) {
        RETRY_UP(retry, {return ROUTINE_ERROR;});
        ARROW_DBG(GATEWAY_CONFIG, "fail");
    msleep(ARROW_RETRY_DELAY);
  }
    ARROW_DBG(GATEWAY_CONFIG, "ok");

    // close session after next HTTP request
    http_session_keep_active(true);

    // device registration
    retry=0;
    while ( konexios_connect_device(&_gateway, &_device, update_gateway_info) < 0 ) {
        RETRY_UP(retry, {return ROUTINE_ERROR;});
        ARROW_DBG(DEVICE_CONNECT, "fail");
    msleep(ARROW_RETRY_DELAY);
  }
    ARROW_DBG(DEVICE_CONNECT, "ok");

    // Close the session!!!!
    http_end();

    // Mark as initialized and return
    acn_register_init_done = 1;
  return ROUTINE_SUCCESS;

device_reg_error:
  konexios_device_free(&_device);
gateway_config_error:
  konexios_gateway_config_free(&_gateway_config);
gateway_reg_error:
  konexios_gateway_free(&_gateway);
  return ret;
}

// Do the initialization for the gateway and device
int konexios_startup_sequence(bool update_gateway_info)
{
    TRACE("Enter");
    int retry = 0;
    int rc;

    // Keep socket connections alive for all HTTP transfers
    http_session_keep_active(true);

    // Connect to Arrow Connect
    // (Do gateway register/checkin)
    rc = konexios_connect_gateway(&_gateway, update_gateway_info);
    if(rc != 200 )
    {
        printf("ACN: Gateway register/checkin failed (%d)\n",rc);
        return rc;
}
    ARROW_INF("ACN: Gateway register/checkin success\n");

    // Get gateway config
    retry=0;
    ARROW_INF("ACN: Get gateway config\n");
    rc = konexios_gateway_config(&_gateway, &_gateway_config);
    if(rc < 0 )
    {
        printf("Gateway config [failed]\n");
        return rc;
    }
    ARROW_INF("ACN: Gateway config [ok]\n");

    // close session after next HTTP request
    //http_session_keep_active(true);

    // device registration
    retry=0;
    rc = konexios_connect_device(&_gateway, &_device, false);
    if(rc != 200 )
    {
        printf("ACN: Device connect [failed]\n");
        return rc;
    }
    ARROW_INF("ACN: Device connect [ok]\n");

    // Close the session!!!!
    http_end();

    // Mark as initialized and return
    acn_register_init_done = 1;
    return 200;
}


konexios_routine_error_t konexios_update_state(const char *name, const char *value) {
    TRACE("Enter");
  //add_state(name, value);
  if ( acn_register_init_done ) {
    konexios_post_state_update(&_device);
    return ROUTINE_SUCCESS;
  }
  return ROUTINE_ERROR;
}

konexios_routine_error_t konexios_send_telemetry_routine(void *data) {
    TRACE("Enter");

    // Must register first
    if ( !acn_register_init_done ) return ROUTINE_NOT_INITIALIZE;

    // Collect and send telemetry
  int retry = 0;
  while ( konexios_send_telemetry(&_device, data) < 0) {
    RETRY_UP(retry, {return ROUTINE_ERROR;});
        ARROW_DBG(DEVICE_TELEMETRY, "fail");
    msleep(ARROW_RETRY_DELAY);
  }

    // Success
    ARROW_DBG(DEVICE_TELEMETRY, "ok");
  return ROUTINE_SUCCESS;
}

// MQTT

konexios_routine_error_t konexios_mqtt_connect_telemetry_routine(void) {
    TRACE("Enter");
    int retry;

    // Don't reinit
    if ( acn_mqtt_init_flags & MQTT_INIT_SYSTEM_DONE ) return ROUTINE_ERROR;

    // Run the connect routine for the MQTT client
    retry = 0;
  while ( mqtt_telemetry_connect(&_gateway, &_device, &_gateway_config) < 0 ) {
    RETRY_UP(retry, {return ROUTINE_MQTT_CONNECT_FAILED;});
        ARROW_DBG(DEVICE_MQTT_CONNECT, "fail");
        msleep(ARROW_RETRY_DELAY);
  }

    // Mark MQTT as initialized
    acn_mqtt_init_flags |= MQTT_INIT_SYSTEM_DONE;
    ARROW_DBG(DEVICE_MQTT_CONNECT, "ok");
  return ROUTINE_SUCCESS;
}

konexios_routine_error_t konexios_mqtt_disconnect_telemetry_routine(void) {
    TRACE("Enter");

    // Check for init
    if ( ! ( acn_mqtt_init_flags & MQTT_INIT_SYSTEM_DONE ) ) return ROUTINE_ERROR;

    // Terminate the MQTT connection
    if ( mqtt_telemetry_terminate() < 0 ) return ROUTINE_ERROR;

  return ROUTINE_SUCCESS;
}

#if 0
// Pointless function
konexios_routine_error_t konexios_mqtt_terminate_telemetry_routine(void) {
    TRACE("Enter");
    if ( mqtt_telemetry_terminate() < 0 ) return ROUTINE_ERROR;
  return ROUTINE_SUCCESS;
}
#endif


// If EVENTS are enabled
#if !defined(NO_EVENTS)

// TODO: What is the difference between this and the one belowl????
konexios_routine_error_t konexios_mqtt_connect_event_routine(void) {
    TRACE("Enter");
    int retry;

    // Make sure MQTT subscribe has not been done before
    if ( acn_mqtt_init_flags & MQTT_INIT_SUBSCRIBE_DONE ) return ROUTINE_ERROR;

    // do mqtt_subscribe_connect()
    // Subscribe to gateway topics
    retry = 0;
  while(mqtt_subscribe_connect(&_gateway, &_device, &_gateway_config) < 0 ) {
    RETRY_UP(retry, {return ROUTINE_MQTT_SUBSCRIBE_FAILED;});
        ARROW_DBG(DEVICE_MQTT_CONNECT, "fail");
        msleep(ARROW_RETRY_DELAY);
  }

    // Mark subscribe as done
    acn_mqtt_init_flags |= MQTT_INIT_SUBSCRIBE_DONE;
  return ROUTINE_SUCCESS;
}

// TODO: What is the difference between this and the one above????
konexios_routine_error_t konexios_mqtt_subscribe_event_routine(void) {
    TRACE("Enter");
  int retry = 0;

    // Do mqtt_subscribe()
  while( mqtt_subscribe() < 0 ) {
    RETRY_UP(retry, {return ROUTINE_MQTT_SUBSCRIBE_FAILED;});
        ARROW_DBG(DEVICE_MQTT_CONNECT, "fail");
        msleep(ARROW_RETRY_DELAY);
        acn_mqtt_init_flags &= ~MQTT_INIT_SUBSCRIBE_DONE;
  }

    // Make subscribe as done
    acn_mqtt_init_flags |= MQTT_INIT_SUBSCRIBE_DONE;
  return ROUTINE_SUCCESS;
}

// Stop event routine
konexios_routine_error_t konexios_mqtt_disconnect_event_routine(void)
{
    TRACE("Enter");

    // Only run when subscribe is done
    if ( ! ( acn_mqtt_init_flags & MQTT_INIT_SUBSCRIBE_DONE ) ) return ROUTINE_ERROR;

    // Disconnect
    if ( mqtt_subscribe_disconnect() < 0 ) return ROUTINE_ERROR;

    // Clear the flag?
    //acn_mqtt_init_flags &= ~MQTT_INIT_SUBSCRIBE_DONE

  return ROUTINE_SUCCESS;
}

#if 0
// Pointless function
konexios_routine_error_t konexios_mqtt_terminate_event_routine(void) {
    TRACE("Enter");
    if ( mqtt_subscribe_terminate() < 0 ) return ROUTINE_ERROR;
  return ROUTINE_SUCCESS;
}
#endif

#endif


konexios_routine_error_t konexios_mqtt_connect_routine(void)
{
    TRACE("Enter");
  konexios_routine_error_t ret = ROUTINE_ERROR;

    // Must call register first
    if ( !acn_register_init_done ) return ROUTINE_NOT_INITIALIZE;

    // Start the MQTT telemetry
    ARROW_DBG("mqtt connect...");
  ret = konexios_mqtt_connect_telemetry_routine();
    if ( ret != ROUTINE_SUCCESS ) return ret;

    // TODO: What is this?
    konexios_state_mqtt_run(&_device);

    // Start the MQTT events
#if !defined(NO_EVENTS)
  ret = konexios_mqtt_connect_event_routine();
    if ( ret != ROUTINE_SUCCESS ) return ret;

    // TODO: What is this?
    // process postponed messages?
  if ( konexios_mqtt_event_receive_routine() != ROUTINE_RECEIVE_EVENT ) {
    konexios_mqtt_subscribe_event_routine();
  }
#endif

    // Return success
  return ROUTINE_SUCCESS;
}

konexios_routine_error_t konexios_mqtt_disconnect_routine() {
    TRACE("Enter");

    // If we have INIT or SUBSCRIBE flags set
    if ( acn_mqtt_init_flags ) {
        // Disconnect?
    mqtt_disconnect();
        // Clear flags
        acn_mqtt_init_flags = 0;
    return ROUTINE_SUCCESS;
  }

    // Error since no flags are set
  return ROUTINE_ERROR;
}

#if 1
// Another pointless function???
konexios_routine_error_t konexios_mqtt_terminate_routine() {
    TRACE("Enter");
  mqtt_terminate();
  acn_mqtt_init_flags = 0;
  return ROUTINE_SUCCESS;
}
#endif

konexios_routine_error_t konexios_mqtt_pause_routine(int pause) {
    mqtt_pause(pause);
    return ROUTINE_SUCCESS;
}

konexios_routine_error_t konexios_mqtt_send_telemetry_routine(get_data_cb data_cb, void *data) {
    TRACE("Enter");

    // Can't send if not fully connected
    if( !acn_register_init_done ||
       !(acn_mqtt_init_flags & MQTT_INIT_SYSTEM_DONE)
#if !defined(NO_EVENTS)
       || !(acn_mqtt_init_flags & MQTT_INIT_SUBSCRIBE_DONE)
#endif
       )
    {
        ARROW_DBG(DEVICE_MQTT_TELEMETRY, "Cloud not initialize");
    return ROUTINE_NOT_INITIALIZE;
  }

    // The main loop for telemetry and events
    while (1)
    {
        // TODO: What does this do?
        mqtt_yield(TELEMETRY_DELAY);

        // Handle receiving events?
#if !defined(NO_EVENTS)
    if ( konexios_mqtt_has_events() ) {
            ARROW_DBG("There is an event");
      return ROUTINE_RECEIVE_EVENT;
    }
#endif

        // Collect telemetry into 'data'
        int get_data_result;
        get_data_result = data_cb(data);
    if ( get_data_result < 0 ) {
            //ARROW_DBG(DEVICE_MQTT_TELEMETRY, "Fail to get telemetry data");
      return ROUTINE_GET_TELEMETRY_FAILED;
    } else if (get_data_result > 0 ) {
      // skip this
      continue;
    }

        // We now have get_data_result==0 and some data in 'data'

        // Publish the data for this device
        if ( mqtt_publish(&_device, data) < 0 )
        {
            ARROW_DBG(DEVICE_MQTT_TELEMETRY, "fail");
      return ROUTINE_MQTT_PUBLISH_FAILED;
    }

        // Some tests?
#if defined(VALGRIND_TEST)
    static int count = 0;
        if ( count++ > VALGRIND_TEST )
      return ROUTINE_TEST_DONE;
        ARROW_DBG("test count [%d]", count);
#endif

        ARROW_DBG(DEVICE_MQTT_TELEMETRY, "ok");
  }

    // Why return success? There are no breaks in the while(1)
  return ROUTINE_SUCCESS;
}

// Shutdown the http and mqtt subsystems
void konexios_close(void)
{
  konexios_mqtt_terminate_routine();
    if ( acn_register_init_done )
    {
        // Free the structures
    konexios_device_free(&_device);
    konexios_gateway_free(&_gateway);
    konexios_gateway_config_free(&_gateway_config);

        // Stop events
        #if !defined(NO_EVENTS)
        konexios_mqtt_events_done();
        #endif

        // Clear flags
        acn_register_init_done = 0;
  }

    // Stop the HTTP client?
    http_end();
    http_done();

    return;
}

// What's the difference between this and konexios_mqtt_send_telemetry_routine()
konexios_routine_error_t konexios_mqtt_telemetry_routine(get_data_cb data_cb, void *data)
{
    TRACE("Enter");

    // Can't run if not fully connected
    if ( !acn_register_init_done ||
         !(acn_mqtt_init_flags & MQTT_INIT_SYSTEM_DONE) )
    {
        ARROW_DBG(DEVICE_MQTT_TELEMETRY, "Cloud not initialize");
    return ROUTINE_NOT_INITIALIZE;
  }

    //
    while (1)
    {
        // Wait a bit
    msleep(TELEMETRY_DELAY);

        // Load the telemetry data
        int get_data_result;
        get_data_result = data_cb(data);
    if ( get_data_result < 0 ) {
            //ARROW_DBG(DEVICE_MQTT_TELEMETRY, "Fail to get telemetry data");
      return ROUTINE_GET_TELEMETRY_FAILED;
    } else if (get_data_result > 0 ) {
      // skip this
      continue;
    }

        // Now have data in 'data'

        // Send the data
        if ( mqtt_publish(&_device, data) < 0 )
        {
            ARROW_DBG(DEVICE_MQTT_TELEMETRY, "fail");
      return ROUTINE_MQTT_PUBLISH_FAILED;
    }

        // Some tests?
#if defined(VALGRIND_TEST)
    static int count = 0;
        if ( count++ > VALGRIND_TEST )
      return ROUTINE_TEST_DONE;
        ARROW_DBG("test count [%d]", count);
#endif

        ARROW_DBG(DEVICE_MQTT_TELEMETRY, "ok");
  }

    // TODO: Why send success? No break in while(1)
  return ROUTINE_SUCCESS;
}

konexios_routine_error_t konexios_mqtt_telemetry_once_routine(get_data_cb data_cb, void *data) {
    TRACE("Enter");

    // Can't send if not initialized
    if ( !acn_register_init_done ||
         !(acn_mqtt_init_flags & MQTT_INIT_SYSTEM_DONE) ) {
        ARROW_DBG(DEVICE_MQTT_TELEMETRY, "Cloud not initialize");
    return ROUTINE_NOT_INITIALIZE;
  }


    // Collected the data into 'data'
  int get_data_result = data_cb(data);
  if ( get_data_result != 0 ) {
        ARROW_DBG(DEVICE_MQTT_TELEMETRY, "Fail to get telemetry data");
    return ROUTINE_GET_TELEMETRY_FAILED;
  }

    // Send the data
  if ( mqtt_publish(&_device, data) < 0 ) {
        ARROW_DBG(DEVICE_MQTT_TELEMETRY, "fail");
    return ROUTINE_MQTT_PUBLISH_FAILED;
  }

    // Success!
    ARROW_DBG(DEVICE_MQTT_TELEMETRY, "ok");
  return ROUTINE_SUCCESS;
}

konexios_routine_error_t konexios_mqtt_event_receive_routine() {
    //TRACE("Enter");
    int ret;

    // Can't receive if not subscribed
    if ( !acn_register_init_done ||
         !(acn_mqtt_init_flags & MQTT_INIT_SUBSCRIBE_DONE) ) {
        //ARROW_DBG(DEVICE_MQTT_TELEMETRY, "Cloud not initialize");
    return ROUTINE_NOT_INITIALIZE;
  }

    printf("mqtt_yield Start\n");

    // yield blocks for a period of time and returns what
    // events the MQTT client has received.
    // MQTT_SUCCESS == we have a message (aka an 'event'
    // FAILURE = timeout?????
    ret = mqtt_yield(TELEMETRY_DELAY);

    printf("mqtt_yield End\n");

    // TODO: Return if we have events?
    if ( ret == MQTT_SUCCESS )
    return ROUTINE_RECEIVE_EVENT;

    return ROUTINE_SUCCESS;
}

konexios_routine_error_t konexios_mqtt_check_init(void) {
  if ( !acn_register_init_done ||
       !(acn_mqtt_init_flags & MQTT_INIT_SUBSCRIBE_DONE) ) {
    DBG(DEVICE_MQTT_TELEMETRY, "Cloud not initialize");
    return ROUTINE_NOT_INITIALIZE;
  }
  return ROUTINE_SUCCESS;
}
