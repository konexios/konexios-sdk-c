#if !defined(ARROW_NODE_TYPE_H_)
#define ARROW_NODE_TYPE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/connection.h>

typedef struct _arrow_node_type {
  char *description;
  int enabled;
  char *name;
  char *hid;
} arrow_node_type_t;

int arrow_node_type_list();
int arrow_node_type_create(arrow_node_type_t *node);
int arrow_node_type_update(arrow_node_type_t *node);


#if defined(__cplusplus)
}
#endif

#endif  // ARROW_NODE_TYPE_H_
