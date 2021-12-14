#include <fastjson/io.h>
#include <fastjson/json.h>
#include <fastjson/macros.h>
#include <fastjson/mem.h>
#include <fastjson/node.h>
#include <fastjson/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

JSON *json_parse(const char *contents, JSONOptions *options) {
  char *ctnt = (char *)contents;
  FJLexer lexer = FJ_LEXER(ctnt);

  if (options) {
    lexer.optimized_strings = options->optimized_strings;
  }

  FJParser parser = (FJParser){&lexer, 0};
  JSON *root = parse(&parser);

  return root;
}
JSON *json_parse_file(const char *filepath, JSONOptions *options) {
  char *contents = fj_read_file(filepath);
  if (!contents) {
    fprintf(stderr, "json: Failed to parse file `%s`\n", filepath);
    return 0;
  }

  JSON *root = json_parse(contents, options);

  root->source = contents;

  return root;
}

void json_await(JSONAsync *json_async) {
  if (pthread_join(json_async->id, 0)) {
    printf("json: failed to await async response.\n");
  }
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
  JSON *current = iterator->current;

  if (iterator->index >= iterator->length) {
    return 0;
  }

  iterator->index++;
  if (iterator->index < iterator->length) {
    iterator->current = iterator->items[iterator->index];
  }
  return current;
}

JSON *json_get(JSON *node, const char *key) {
  if (!node)
    return 0;
  if (!key)
    return 0;

  if (node->type == FJ_NODE_TUPLE && node->value) {
    return node->value;
  }

  if (!node->map) {
    fprintf(stderr, "json warning: calling json_get on a node which does not "
                    "have a hashmap.\n");
    return 0;
  }

  return (JSON *)map_get_value(node->map, (char *)key);
}

float json_get_float(JSON *node, const char *key) {
  JSON *value = json_get(node, key);
  if (!value)
    return 0;
  return OR(value->value_float, (float)value->value_int);
}
int json_get_int(JSON *node, const char *key) {
  JSON *value = json_get(node, key);
  if (!value)
    return 0;
  return OR(value->value_int, (int)value->value_float);
}
float json_get_number(JSON *node, const char *key) {
  return OR(json_get_float(node, key), (float)json_get_int(node, key));
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

char *json_stringify_array(JSON *node) {
  FJString s = FJ_STRING("");
  fj_string_concat_char(&s, '[');

  JSONIterator it = json_iterate(node);

  JSON *child = 0;
  while ((child = json_iterator_next(&it)) != 0) {
    fj_string_concat(&s, json_stringify(child));

    if (it.index < it.length)
      fj_string_concat_char(&s, ',');
  }

  fj_string_concat_char(&s, ']');

  return s.value;
}

char *json_stringify_dict(JSON *node) {
  FJString s = FJ_STRING("");
  fj_string_concat_char(&s, '{');

  JSONIterator it = json_iterate_kv(node);

  JSON *child = 0;
  while ((child = json_iterator_next(&it)) != 0) {
    fj_string_concat(&s, json_stringify(child));

    if (it.index < it.length)
      fj_string_concat_char(&s, ',');
  }
  fj_string_concat_char(&s, '}');

  return s.value;
}

char *json_stringify_string(JSON *node) {
  char *v = (node->value_str ? node->value_str : "");
  const char *template = "\"%s\"";
  char *buff = (char *)calloc(strlen(v) + strlen(template) + 1, sizeof(char));
  sprintf(buff, template, v);
  return buff;
}

char *json_stringify_float(JSON *node) {
  char buff[128];
  sprintf(buff, "%12.6f", node->value_float);
  return strdup(buff);
}

char *json_stringify_int(JSON *node) {
  char buff[128];
  sprintf(buff, "%d", node->value_int);
  return strdup(buff);
}

char *json_stringify_tuple(JSON *node) {
  FJString s = FJ_STRING("");
  fj_string_concat(&s, json_stringify(node->key));
  fj_string_concat_char(&s, ':');
  fj_string_concat(&s, json_stringify(node->value));
  return s.value;
}

char *json_stringify(JSON *node) {
  switch (node->type) {
  case FJ_NODE_ARRAY:
    return json_stringify_array(node);
    break;
  case FJ_NODE_DICT:
    return json_stringify_dict(node);
    break;
  case FJ_NODE_STRING:
    return json_stringify_string(node);
    break;
  case FJ_NODE_FLOAT:
    return json_stringify_float(node);
    break;
  case FJ_NODE_INT:
    return json_stringify_int(node);
    break;
  case FJ_NODE_TUPLE:
    return json_stringify_tuple(node);
    break;
  default: { return strdup(""); } break;
  }
}
