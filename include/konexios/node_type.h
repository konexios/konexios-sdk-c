#if !defined(KONEXIOS_NODE_TYPE_H_)
#define KONEXIOS_NODE_TYPE_H_

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _konexios_node_type {
  char *description;
  int enabled;
  char *name;
  char *hid;
} konexios_node_type_t;

// list existing node types
int konexios_node_type_list(void);
// create new node type
int konexios_node_type_create(konexios_node_type_t *node);
// update existing node type
int konexios_node_type_update(konexios_node_type_t *node);


#if defined(__cplusplus)
}
#endif

#endif  // KONEXIOS_NODE_TYPE_H_
