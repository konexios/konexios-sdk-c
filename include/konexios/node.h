#if !defined(ARROW_NODE_H_)
#define ARROW_NODE_H_

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _konexios_node {
  char *description;
  int enabled;
  char *name;
  char *nodeTypeHid;
  char *parentNodeHid;
  char *hid;
} konexios_node_t;

// list existing nodes
int konexios_node_list(void);
// create new node
int konexios_node_create(konexios_node_t *node);
// update existing node
int konexios_node_update(konexios_node_t *node);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_NODE_H_
