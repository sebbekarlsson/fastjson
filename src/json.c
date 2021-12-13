#include <fastjson/io.h>
#include <fastjson/json.h>
#include <fastjson/mem.h>
#include <fastjson/node.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

JSON *json_parse(const char *contents) {
  FJLexer lexer = FJ_LEXER(contents);
  FJParser parser = (FJParser){&lexer, 0};
  JSON *root = parse(&parser);

  return root;
}
JSON *json_parse_file(const char *filepath) {
  char *contents = fj_read_file(filepath);
  if (!contents) {
    fprintf(stderr, "json: Failed to parse file `%s`\n", filepath);
    return 0;
  }

  JSON *root = json_parse(contents);

  free(contents);

  return root;
}

void json_free(JSON *node) { fj_node_free(node); }

unsigned int json_is_array(JSON *json) {
  if (!json)
    return 0;
  return json->type == FJ_NODE_ARRAY;
}

JSONIterator json_iterate(JSON *node) {
  JSONIterator iterator = {};
  if (!json_is_array(node)) {
    fprintf(stderr,
            "json warning: trying to iterate over %p which is not an array.\n",
            node);
    return iterator;
  }
  if (!node->children) {
    return iterator;
  }

  iterator.index = 0;
  iterator.length = node->children_length;
  iterator.items = node->children;
  iterator.current = iterator.items[iterator.index];

  return iterator;
}

JSONIterator json_iterate_kv(JSON *node) {
  JSONIterator iterator = {};
  if (!json_is_dict(node)) {
    fprintf(stderr,
            "json warning: trying to iterate over %p which is not a dict.\n",
            node);
    return iterator;
  }
  if (!node->children) {
    return iterator;
  }

  iterator.index = 0;
  iterator.length = node->children_length;
  iterator.items = node->children;
  iterator.current = iterator.items[iterator.index];

  return iterator;
}
JSON *json_iterator_next(JSONIterator *iterator) {
  iterator->index++;

  if (iterator->index >= iterator->length) {
    return 0;
  }

  iterator->current = iterator->items[iterator->index];
  return iterator->current;
}

JSON *json_get(JSON *node, const char *key) {
  if (!node)
    return 0;
  if (!key)
    return 0;

  if (node->type == FJ_NODE_TUPLE && node->value) {
    return node->value;
  }

  if (!node->map)
    return 0;

  return (JSON *)map_get_value(node->map, (char *)key);
}

float json_get_float(JSON *node, const char *key) {
  JSON *value = json_get(node, key);
  if (!value)
    return 0;
  return value->value_num;
}
int json_get_int(JSON *node, const char *key) {
  JSON *value = json_get(node, key);
  if (!value)
    return 0;
  return (int)value->value_num;
}
float json_get_number(JSON *node, const char *key) {
  return json_get_float(node, key);
}
char *json_get_string(JSON *node, const char *key) {
  JSON *value = json_get(node, key);
  if (!value)
    return 0;
  return value->value_str;
}

JSONIterator json_get_array(JSON *node, const char *key) {
  JSONIterator it = {};
  JSON *value = json_get(node, key);
  if (!value)
    return it;

  return json_iterate(value);
}

void json_keys(JSON *node, char ***keys, uint32_t *len) {
  if (!node)
    return;
  if (!node->map)
    return;

  map_get_keys(node->map, keys, len);
}

void json_values(JSON *node, JSON **values, uint32_t *len) {
  if (!node)
    return;
  char **keys = 0;
  uint32_t nr_keys = 0;

  json_keys(node, &keys, &nr_keys);
  if (!nr_keys || !keys)
    return;

  JSON **_values = FJ_CALLOC(JSON *, nr_keys);

  uint32_t c = 0;
  for (uint32_t i = 0; i < nr_keys; i++) {
    char *key = keys[i];
    if (!key)
      continue;
    JSON *val = json_get(node, key);
    if (!val)
      continue;
    _values[c] = val;
    c++;
  }

  *values = *_values;
  *len = c;
}

unsigned int json_is_dict(JSON *json) {
  if (!json)
    return 0;
  return json->type == FJ_NODE_DICT;
}

char *json_key(JSON *node) {
  if (!node)
    return 0;
  if (!node->key)
    return 0;
  return node->key->value_str;
}
