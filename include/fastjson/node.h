#ifndef FAST_JSON_NODE_H
#define FAST_JSON_NODE_H
#include <fastjson/config.h>
#include <hashmap/map.h>
#include <stdint.h>
#include <fastjson/macros.h>

struct FAST_JSON_LEXER_STRUCT;


#define FJSON_FOR_EACH_NODE_TYPE(E)\
  E(FJ_NODE_DICT)\
  E(FJ_NODE_ID)\
  E(FJ_NODE_STRING)\
  E(FJ_NODE_INT)\
  E(FJ_NODE_UINT32)\
  E(FJ_NODE_UINT64)\
  E(FJ_NODE_INT32)\
  E(FJ_NODE_INT64)\
  E(FJ_NODE_FLOAT)\
  E(FJ_NODE_ARRAY)\
  E(FJ_NODE_TUPLE)\
  E(FJ_NODE_ERROR)\
  E(FJ_NODE_NOOP)\
  E(FJ_NODE_NONE)

typedef enum { FJSON_FOR_EACH_NODE_TYPE(FJSON_GENERATE_ENUM) } FJNodeType;

static const char *const FJSON_NODE_TYPE_STR[] = {
  FJSON_FOR_EACH_NODE_TYPE(FJSON_GENERATE_STRING)
};

typedef struct FAST_JSON_NODE_STRUCT {
  FJNodeType type;
  char *value_str;
  float value_float;
  struct FAST_JSON_NODE_STRUCT **children;
  JSON_LENGTH_INT children_length;
  struct FAST_JSON_NODE_STRUCT *value;
  struct FAST_JSON_NODE_STRUCT *key;
  map_T *map;
  char *source;
  char *str_start;
  char *str_end;
} FJNode;

FJNode *init_fj_node(FJNodeType type);

void fj_node_add_child(FJNode *node, FJNode *child);

void fj_node_free(FJNode *node);

void fj_node_assign_dict(FJNode *node, const char *key, FJNode *value);

char *fj_node_string(FJNode *node, struct FAST_JSON_LEXER_STRUCT *lexer);

#endif
