#if !defined(_KONEXIOS_GATEWAY_PAYLOAD_SIGN_H_)
#define _KONEXIOS_GATEWAY_PAYLOAD_SIGN_H_

#include <konexios_config.h>
#include <json/json.h>

int gateway_payload_sign(char *signature,
                         const char *hid,
                         const char *name,
                         int encrypted,
                         const char *canParString,
                         const char *signatureVersion);

int konexios_event_sign(char *signature,
                  property_t ghid,
                  const char *name,
                  int encrypted,
                  JsonNode *_parameters);

#endif // _KONEXIOS_GATEWAY_PAYLOAD_SIGN_H_
