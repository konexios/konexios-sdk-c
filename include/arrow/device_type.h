#if !defined(ARROW_DEVICE_TYPE_H_)
#define ARROW_DEVICE_TYPE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/connection.h>

typedef struct _device_type_telemetry {
  int controllable;
  char *description;
  char *name;
  char *type;
  struct _device_type_telemetry *next;
} device_type_telemetry_t;

typedef struct _device_type {
  char *description;
  int enabled;
  char *name;
  device_type_telemetry_t *telemetries;
} device_type_t;

void device_type_init(device_type_t *dev, int enable, const char *name, const char *dec);
void device_type_add_telemetry(device_type_t *dev, int contr, const char *name, const char *type, const char *desc);

void device_type_free(device_type_t *dev);

int arrow_device_type_list();
int arrow_device_type_create(device_type_t *dev_type);
int arrow_device_type_update(arrow_device_t *dev, device_type_t *dev_type);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_DEVICE_TYPE_H_
