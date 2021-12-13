#ifndef FAST_JSON_NODE_H
#define FAST_JSON_NODE_H
#include <hashmap/map.h>
#include <stdint.h>

typedef enum {
  FJ_NODE_DICT,
  FJ_NODE_ID,
  FJ_NODE_STRING,
  FJ_NODE_NUMBER,
  FJ_NODE_ARRAY,
  FJ_NODE_TUPLE,
  FJ_NODE_ERROR,
  FJ_NODE_NOOP,
  FJ_NODE_NONE
} FJNodeType;

typedef struct FAST_JSON_NODE_STRUCT {
  FJNodeType type;
  char *value_str;
  float value_num;
  struct FAST_JSON_NODE_STRUCT **children;
  uint32_t children_length;
  struct FAST_JSON_NODE_STRUCT *value;
  struct FAST_JSON_NODE_STRUCT *key;
  map_T *map;
} FJNode;

FJNode *init_fj_node(FJNodeType type);

void fj_node_add_child(FJNode *node, FJNode *child);

void fj_node_free(FJNode *node);

void fj_node_assign_dict(FJNode *node, const char *key, FJNode *value);
#endif
