#if !defined(ARROW_DEVICE_API_H_)
#define ARROW_DEVICE_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/connection.h>

enum FindBy {
  f_userHid,
  f_uid,
  f_type,
  f_gatewayHid,
  f_createdBefore,
  f_createdAfter,
  f_updatedBefore,
  f_updatedAfter,
  f_enabled,
  f_page,
  f_size
};

typedef struct _find_by {
  int key;
  const char *value;
  struct _find_by *next;
#if defined(__cplusplus)
  _find_by(int k, const char *val) : key(k), value(val), next(NULL) {}
#endif
} find_by_t;

#if defined(__cplusplus)
#define find_by(x, y) find_by_t(x, (const char*)y)
#else
#define find_by(x, y) (find_by_t){ .key=x, .name=y, .next=NULL }
#endif

int arrow_register_device(arrow_gateway_t *gateway, arrow_device_t *device);
int arrow_device_find_by(int n, ...);
int arrow_device_find_by_hid(const char *hid);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_DEVICE_API_H_
