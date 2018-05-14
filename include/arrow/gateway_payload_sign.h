#if !defined(_ARROW_GATEWAY_PAYLOAD_SIGN_H_)
#define _ARROW_GATEWAY_PAYLOAD_SIGN_H_

#include <config.h>

int gateway_payload_sign(char *signature,
                         const char *hid,
                         const char *name,
                         int encrypted,
                         const char *canParString,
                         const char *signatureVersion);

#endif // _ARROW_GATEWAY_PAYLOAD_SIGN_H_
