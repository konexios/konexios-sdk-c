#ifndef __MQTT_OUT_MSG_H__
#define __MQTT_OUT_MSG_H__

#define MQTT_ACK_LENGTH 5


// types of outbound messages
typedef enum {
    OUT_ACK,
    OUT_MQTT_ACK,
} out_msg_type_enum_t;

typedef struct {
    unsigned char resp[MQTT_ACK_LENGTH];
    int len;
} mqtt_ack_t;

typedef struct{
    out_msg_type_enum_t type;
    uint16_t            packetId;
    uint16_t            retryCount;
    union {
        mqtt_ack_t     mqtt_ack;
    };
}out_msg_t;


#endif

// EOF
