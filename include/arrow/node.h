#if !defined(ARROW_NODE_H_)
#define ARROW_NODE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/connection.h>

typedef struct _arrow_node {
  char *description;
  int enabled;
  char *name;
  char *nodeTypeHid;
  char *parentNodeHid;
  char *hid;
} arrow_node_t;

int arrow_node_list();
int arrow_node_create(arrow_node_t *node);
int arrow_node_update(arrow_node_t *node);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_NODE_H_
