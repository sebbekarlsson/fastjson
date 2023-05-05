#include <fastjson/io.h>
#include <fastjson/json.h>
#include <fastjson/macros.h>
#include <fastjson/mem.h>
#include <fastjson/node.h>
#include <fastjson/utils.h>
#include <math.h>
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

  if (options) {
    parser.ignore_int_types = options->ignore_int_types;
  }

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

int json_iterate_kv2(JSON *node, JSONIterator *it) {
  if (!json_is_dict(node)) {
    fprintf(stderr,
            "json warning: trying to iterate over %p which is not a dict.\n",
            node);
    return 0;
  }
  if (!node->children) {
    return 0;
  }

  it->index = 0;
  it->length = node->children_length;
  it->items = node->children;
  it->current = it->items[it->index];

  return node->children != 0 && node->children_length > 0;
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

bool json_array_includes_string(JSON *node, const char *value, bool fuzzy) {
  if (!node || !value)
    return false;
  if (!json_is_array(node))
    return false;

  JSONIterator it = json_iterate(node);

  JSON *child = 0;

  while ((child = json_iterator_next(&it))) {
    if (child->value_str == 0)
      continue;
    if (strcmp(child->value_str, value) == 0)
      return true;

    if (fuzzy) {
      if (strstr(child->value_str, value) != 0)
        return true;
      if (strstr(value, child->value_str) != 0)
        return true;
      if (strcasecmp(child->value_str, value) == 0)
        return true;
    }
  }

  return false;
}

static float json_compare_string(const char *a, const char *b) {
  if (!a || !b)
    return 0.0f;

  int64_t len_a = strlen(a);
  int64_t len_b = strlen(b);
  int64_t max_len = MAX(len_a, len_b);
  int64_t min_len = MIN(len_a, len_b);

  const char *max_value = len_a > len_b ? a : b;
  const char *min_value = len_a < len_b ? a : b;

  char first_char_a = a[0];
  char first_char_b = b[0];

  char last_char_a = a[MAX(len_a - 1, 0)];
  char last_char_b = b[MAX(len_b - 1, 0)];

  float score = (float)min_len / (float)max_len;

  for (int64_t i = 0; i < min_len; i++) {
    char ca = a[i % len_a];
    char cb = b[i % len_b];

    if (ca == cb) {
      score += 1.0f / (float)max_len;
    } else {
      score *= 0.9f;
    }
  }

  if (strcmp(a, b) == 0) {
    score += 1.0f;
  }

  if (strcasecmp(a, b) == 0) {
    score += 0.9f;
  }

  float len_diff = (float)max_len - (float)min_len;
  score = fmaxf(0.0f, score - len_diff);

  if (strstr(max_value, min_value) != 0) {
    score += 0.5f;
  }

  if (first_char_a == first_char_b) {
    score += 0.1f;
  }

  if (last_char_a == last_char_b) {
    score += 0.1f;
  }

  return score;
}

bool json_array_find_match(JSON *node, const char *value, bool fuzzy,
                           JSONMatch *match) {
  if (!node || !value)
    return false;
  if (!json_is_array(node))
    return false;

  JSONIterator it = json_iterate(node);

  JSON *child = 0;

  match->score = 0;
  match->node = 0;

  while ((child = json_iterator_next(&it))) {
    if (child->value_str == 0)
      continue;
    if (strcmp(child->value_str, value) == 0) {
      match->score = json_compare_string(child->value_str, value);
      match->node = child;
      return true;
    }

    if (strcasecmp(child->value_str, value) == 0) {
      match->score = json_compare_string(child->value_str, value);
      match->node = child;
      return true;
    }

    if (fuzzy) {
      if (strstr(child->value_str, value) != 0) {
        match->score = json_compare_string(child->value_str, value);
        match->node = child;
        return true;
      }
      if (strstr(value, child->value_str) != 0) {
        match->score = json_compare_string(child->value_str, value);
        match->node = child;
        return true;
      }
    }
  }

  return false;
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
    fprintf(stderr,
            "json warning: calling json_get on a node which does not "
            "have a hashmap. key=%s\n",
            key);
    char *str = json_stringify(node);
    if (str) {
      printf("The following json had problems: %s\n", str);
      free(str);
      str = 0;
    }
    return 0;
  }

  return (JSON *)map_get_value(node->map, (char *)key);
}

int64_t json_get_int(JSON *node, const char *key) {
  JSON *value = json_get(node, key);
  if (!value)
    return 0;

  return value->value_float;
}

float json_get_float(JSON *node, const char *key) {
  JSON *value = json_get(node, key);
  if (!value)
    return 0;
  return value->value_float;
}
/*double json_get_double(JSON *node, const char *key) {
  JSON *value = json_get(node, key);
  if (!value)
    return 0;
  return (double)OR(value->value_double, OR(value->value_float,
OR(value->value_int, OR(value->value_uint32, value->value_int32))));
}*/
uint32_t json_get_uint32(JSON *node, const char *key) {
  JSON *value = json_get(node, key);
  if (!value)
    return 0;

  return value->value_float;
}

uint64_t json_get_uint64(JSON *node, const char *key) {
  JSON *value = json_get(node, key);
  if (!value)
    return 0;

  return value->value_float;
}

int32_t json_get_int32(JSON *node, const char *key) {
  JSON *value = json_get(node, key);
  if (!value)
    return 0;

  return value->value_float;
}

int64_t json_get_int64(JSON *node, const char *key) {
  JSON *value = json_get(node, key);
  if (!value)
    return 0;
  return value->value_float;
}

JSON *json_set_int(JSON *node, const char *key, int64_t value) {
  return json_set_int64(node, key, value);
}
float json_get_number(JSON *node, const char *key) {
  return OR(json_get_float(node, key), (float)json_get_int(node, key));
}
float json_get_value_number(JSON *node) {
  if (!node)
    return 0.0f;

  return node->value_float;
}

const char *json_get_value_string(JSON *node) {
  if (!node)
    return 0;
  return node->value_str;
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

void json_keys(JSON *node, char ***keys, JSON_LENGTH_INT *len) {
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

const char *json_key(JSON *node) {
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

/*char *json_stringify_double(JSON *node) {
  char buff[128];
  sprintf(buff, "%12.6f", (float)OR(OR(node->value_double, node->value_float),
OR(node->value_uint32, node->value_int32))); return strdup(buff);
}*/

char *json_stringify_int(JSON *node) {
  char buff[128];
  sprintf(buff, "%d", (int)node->value_float);
  return strdup(buff);
}

char *json_stringify_uint32(JSON *node) {
  char buff[128];
  sprintf(buff, "%d", (uint32_t)node->value_float);
  return strdup(buff);
}

char *json_stringify_uint64(JSON *node) {
  char buff[128];
  sprintf(buff, "%ld", (uint64_t)node->value_float);
  return strdup(buff);
}

char *json_stringify_int64(JSON *node) {
  char buff[128];
  sprintf(buff, "%ld", (int64_t)node->value_float);
  return strdup(buff);
}

char *json_stringify_int32(JSON *node) {
  char buff[128];
  sprintf(buff, "%d", (int32_t)node->value_float);
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
  // case FJ_NODE_DOUBLE:
  //  return json_stringify_double(node);
  //  break;
  case FJ_NODE_INT:
    return json_stringify_int(node);
    break;
  case FJ_NODE_UINT32:
    return json_stringify_uint32(node);
    break;
  case FJ_NODE_UINT64:
    return json_stringify_uint64(node);
    break;
  case FJ_NODE_INT64:
    return json_stringify_int64(node);
    break;
  case FJ_NODE_INT32:
    return json_stringify_int64(node);
    break;
  case FJ_NODE_TUPLE:
    return json_stringify_tuple(node);
    break;
  default: {
    return strdup("");
  } break;
  }
}

JSON *json_set(JSON *node, const char *key, JSON *value) {
  if (!key || !node || !value)
    return 0;
  JSON *existing = json_get(node, key);
  if (existing) {
    if (existing->type == FJ_NODE_STRING && value->type == FJ_NODE_STRING) {
      const char *next_val = json_get_value_string(value);

      if (existing->value_str) {
        free(existing->value_str);
        existing->value_str = 0;
      }

      existing->value_str = strdup(next_val);

      return value;
    }

    if (json_is_scalar(*existing) && json_is_scalar(*value)) {
      existing->value_float = value->value_float;
      return value;
    }
    json_free(existing);
  }

  if (node->type == FJ_NODE_DICT || node->map != 0) {
    JSON *tuple = init_fj_node(FJ_NODE_TUPLE);
    tuple->key = init_fj_node(FJ_NODE_STRING);
    tuple->key->value_str = strdup(key);
    tuple->value = value;
    fj_node_add_child(node, tuple);
    map_set(node->map, (char *)key, value);
  }
  return value;
}
JSON *json_set_float(JSON *node, const char *key, float value) {
  if (!key || !node)
    return 0;
  JSON *json_value = init_fj_node(FJ_NODE_FLOAT);
  json_value->value_float = value;
  json_set(node, key, json_value);
  return json_value;
}

/*JSON* json_set_double(JSON* node, const char* key, double value) {
  if (!key || !node) return 0;
  JSON* json_value = init_fj_node(FJ_NODE_DOUBLE);
  json_value->value_float = (float)value;
  json_value->value_double = value;
  json_set(node, key, json_value);
  return json_value;
}*/
JSON *json_set_uint32(JSON *node, const char *key, uint32_t value) {
  if (!key || !node)
    return 0;
  JSON *json_value = init_fj_node(FJ_NODE_UINT32);
  json_value->value_float = (float)value;
  json_set(node, key, json_value);
  return json_value;
}

JSON *json_set_uint64(JSON *node, const char *key, uint64_t value) {
  if (!key || !node)
    return 0;
  JSON *json_value = init_fj_node(FJ_NODE_UINT64);
  json_value->value_float = (float)value;
  json_set(node, key, json_value);
  return json_value;
}

JSON *json_set_int32(JSON *node, const char *key, int32_t value) {
  if (!key || !node)
    return 0;
  JSON *json_value = init_fj_node(FJ_NODE_INT32);
  json_value->value_float = (float)value;
  json_set(node, key, json_value);
  return json_value;
}

JSON *json_set_int64(JSON *node, const char *key, int64_t value) {
  if (!key || !node)
    return 0;
  JSON *json_value = init_fj_node(FJ_NODE_INT64);
  json_value->value_float = (float)value;
  json_set(node, key, json_value);
  return json_value;
}

JSON *json_set_number(JSON *node, const char *key, float value) {
  if (!key || !node)
    return 0;
  return json_set_float(node, key, value);
}
JSON *json_set_array(JSON *node, const char *key, JSON **children,
                     uint32_t length) {
  if (!key || !node || !children || !length)
    return 0;
  JSON *json_value = init_fj_node(FJ_NODE_ARRAY);
  json_value->children = children;
  json_value->children_length = length;
  json_set(node, key, json_value);
  return json_value;
}

JSON *json_set_string(JSON *node, const char *key, const char *value) {
  if (!key || !node || !value)
    return 0;
  JSON *json_value = init_fj_node(FJ_NODE_STRING);
  json_value->value_str = strdup(value);
  json_set(node, key, json_value);
  return json_value;
}

JSON *json_push(JSON *node, const char *key, JSON *value) {
  if (!node || !value)
    return 0;
  if (!value) {
    fprintf(stderr, "json_push: Error, trying to push NULL value.\n");
    return 0;
  }
  if (node->type == FJ_NODE_ARRAY && !key) {
    fj_node_add_child(node, value);
  } else if (!key) {
    fprintf(stderr, "json_push: no key specified.\n");
    return 0;
  }

  JSON *existing = json_get(node, key);
  if (existing) {
    fj_node_add_child(existing, value);
  } else if (key) {
    JSON **arr = (JSON **)calloc(1, sizeof(JSON *));
    arr[0] = value;
    json_set_array(node, key, arr, 1);
  }

  return value;
}

int json_write(JSON *node, const char *filepath, const char *mode) {
  if (!node || !filepath)
    return 0;
  FILE *fp = fopen(filepath, mode);

  if (!fp) {
    fprintf(stderr, "json_write: Failed to open `%s` for writing mode=`%s`.\n",
            filepath, mode);
    return 0;
  }

  char *json_str = json_stringify(node);

  if (!json_str) {
    fprintf(stderr, "json_write: failed to stringify json.\n");
    return 0;
  }

  uint32_t size = strlen(json_str) * sizeof(char);

  if (!fwrite(&json_str[0], size, 1, fp)) {
    fprintf(stderr, "json_write: error writing data to file `%s` mode=`%s`\n",
            filepath, mode);
  }

  if (fp != 0) {
    fclose(fp);
  }

  if (json_str != 0) {
    free(json_str);
  }

  return 1;
}

JSON *json_get_array_item(JSON *node, JSON_LENGTH_INT index) {
  if (!node)
    return 0;
  if (!json_is_array(node))
    return 0;
  if (node->children_length <= 0)
    return 0;

  JSON_LENGTH_INT i = index % node->children_length;

  return (JSON *)node->children[i];
}

float json_get_array_item_float(JSON *node, JSON_LENGTH_INT index) {
  JSON *value = json_get_array_item(node, index);
  if (!value)
    return 0;

  return value->value_float;
}
uint32_t json_get_array_item_uint32(JSON *node, JSON_LENGTH_INT index) {
  JSON *value = json_get_array_item(node, index);
  if (!value)
    return 0;

  return value->value_float;
}
uint64_t json_get_array_item_uint64(JSON *node, JSON_LENGTH_INT index) {
  JSON *value = json_get_array_item(node, index);
  if (!value)
    return 0;

  return value->value_float;
}
int64_t json_get_array_item_int64(JSON *node, JSON_LENGTH_INT index) {
  JSON *value = json_get_array_item(node, index);
  if (!value)
    return 0;

  return value->value_float;
}
int32_t json_get_array_item_int32(JSON *node, JSON_LENGTH_INT index) {
  JSON *value = json_get_array_item(node, index);
  if (!value)
    return 0;

  return value->value_float;
}
const char *json_get_array_item_string(JSON *node, JSON_LENGTH_INT index) {
  JSON *value = json_get_array_item(node, index);
  if (!value)
    return 0;

  return value->value_str;
}

JSON *json_new_dict() { return init_fj_node(FJ_NODE_DICT); }

bool json_is_scalar(JSON json) {
  return (json.type == FJ_NODE_FLOAT || json.type == FJ_NODE_INT ||
          json.type == FJ_NODE_INT32 || json.type == FJ_NODE_UINT32 ||
          json.type == FJ_NODE_INT64 || json.type == FJ_NODE_UINT64);
}

void json_copy_string_into(JSON src, JSON *target) {
  if (src.type != FJ_NODE_STRING || !src.value_str)
    return;
  target->type = FJ_NODE_STRING;
  target->value_str = strdup(src.value_str);
}

JSON *json_copy(JSON node) {

  if (node.type == FJ_NODE_STRING) {
    JSON *new_str = init_fj_node(FJ_NODE_STRING);
    json_copy_string_into(node, new_str);
    return new_str;
  }

  if (json_is_scalar(node)) {
    JSON *out = init_fj_node(node.type);
    out->value_float = node.value_float;
    return out;
  }

  if (json_is_array(&node)) {
    JSON *arr = init_fj_node(FJ_NODE_ARRAY);
    JSONIterator it = json_iterate(&node);

    JSON *child = 0;

    while ((child = json_iterator_next(&it))) {
      JSON *copied_child = json_copy(*child);
      if (copied_child) {
        json_push(arr, 0, copied_child);
      }
    }
    return arr;
  }

  if (json_is_dict(&node)) {
    JSON *new_dict = json_new_dict();
    JSONIterator it = json_iterate_kv(&node);
    JSON *child = 0;
    while ((child = json_iterator_next(&it))) {
      if (!child->key)
        continue;
      const char *key = json_get_value_string(child);
      if (!key)
        key = json_get_value_string(child->key);
      JSON *value = child->value;
      if (!key || !value)
        continue;

      JSON *copied_value = json_copy(*value);

      if (!copied_value)
        continue;

      json_set(new_dict, key, copied_value);
    }

    return new_dict;
  }

  return 0;
}

int json_merge(JSON *dest, JSON other) {
  if (!dest)
    return 0;
  dest->type = other.type;

  if (json_is_scalar(other)) {
    dest->value_float = other.value_float;
    return 1;
  }

  if (json_is_dict(&other)) {
    JSONIterator it = json_iterate_kv(&other);

    JSON *child = 0;

    while ((child = json_iterator_next(&it))) {
      if (!child->key)
        continue;
      const char *key = json_get_value_string(child);
      if (!key)
        key = json_get_value_string(child->key);
      JSON *value = child->value;
      if (!key || !value)
        continue;

      JSON *copied_value = json_copy(*value);

      if (!copied_value)
        continue;

      json_set(dest, key, copied_value);
    }

    return 1;
  }

  return 0;
}
