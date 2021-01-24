#if !defined(KONEXIOS_DEVICE_ACTION_H_)
#define KONEXIOS_DEVICE_ACTION_H_

#include <konexios/device.h>

typedef struct _dev_action_model {
  char *criteria;
  char *description;
  int enabled;
  int expiration;
  int index;
  char *systemName;
} dev_action_model_t;

// create a new device action for a specific device type
int konexios_create_device_action(konexios_device_t *dev, dev_action_model_t *model);
// delete a device action from a specific device type
int konexios_delete_device_action(konexios_device_t *dev, dev_action_model_t *model);
// list existing device actions for a device
int konexios_list_device_action(konexios_device_t *dev);
// list available action types
int konexios_list_action_type(void);
// update an existing device action for a specific device type
int konexios_update_device_action(konexios_device_t *dev, dev_action_model_t *model);

#endif  // KONEXIOS_DEVICE_ACTION_H_
