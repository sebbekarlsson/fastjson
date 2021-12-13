#ifndef FAST_JSON_H
#define FAST_JSON_H
#include <fastjson/node.h>
#include <fastjson/parse.h>

typedef FJNode JSON;

typedef struct FAST_JSON_ITERATOR_STRUCT {
  JSON *current;
  uint32_t index;
  JSON **items;
  uint32_t length;
} JSONIterator;

JSONIterator json_iterate(JSON *node);
JSONIterator json_iterate_kv(JSON *node);
JSON *json_iterator_next(JSONIterator *iterator);

JSON *json_parse(const char *contents);
JSON *json_parse_file(const char *filepath);
void json_free(JSON *node);

JSON *json_get(JSON *node, const char *key);

float json_get_float(JSON *node, const char *key);
int json_get_int(JSON *node, const char *key);
float json_get_number(JSON *node, const char *key);
char *json_get_string(JSON *node, const char *key);
JSONIterator json_get_array(JSON *node, const char *key);

void json_keys(JSON *node, char ***keys, uint32_t *len);

void json_values(JSON *node, JSON **values, uint32_t *len);

unsigned int json_is_array(JSON *json);
unsigned int json_is_dict(JSON *json);

char *json_key(JSON *node);

#endif
