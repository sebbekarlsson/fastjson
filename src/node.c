#include <fastjson/lex.h>
#include <fastjson/mem.h>
#include <fastjson/node.h>
#include <stdlib.h>

FJNode *init_fj_node(FJNodeType type) {
  FJNode *node = FJ_CALLOC(FJNode, 1);
  node->type = type;

  if (node->type == FJ_NODE_DICT || node->type == FJ_NODE_ARRAY) {
    node->map = NEW_MAP();
  }

  return node;
}

void fj_node_add_child(FJNode *node, FJNode *child) {
  if (!node || !child)
    return;
  node->children_length++;
  node->children = (FJNode **)realloc(node->children,
                                      node->children_length * sizeof(FJNode *));
  node->children[node->children_length - 1] = child;
}

void fj_node_free(FJNode *node) {
  if (!node)
    return;
  if (node->value_str != 0) {
    free(node->value_str);
    node->value_str = 0;
  }

  if (node->children) {
    for (uint32_t i = 0; i < node->children_length; i++) {
      fj_node_free(node->children[i]);
    }

    free(node->children);
    node->children = 0;
  }

  if (node->key) {
    fj_node_free(node->key);
    node->key = 0;
  }

  if (node->value) {
    fj_node_free(node->value);
    node->value = 0;
  }

  if (node->map) {
    map_free(node->map);
    node->map = 0;
  }

  if (node->source != 0)

  {
    free(node->source);
    node->source = 0;
  }

  free(node);
}

void fj_node_assign_dict(FJNode *node, const char *key, FJNode *value) {
  if (!node)
    return;
  if (!node->map) {
    node->map = NEW_MAP();
  }

  map_set(node->map, (char *)key, value);
}

char *fj_node_string(FJNode *node, FJLexer *lexer) {
  char *start = node->str_start;
  char *end = node->str_end;
  if (!start || !end)
    return 0;

  uint32_t length = end - start;

  if (!length)
    return 0;

  char *s = FJ_CALLOC(char, length + 1);

  memcpy(&s[0], start, length);

  return s;
}
