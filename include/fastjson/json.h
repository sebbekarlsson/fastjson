#ifndef FAST_JSON_H
#define FAST_JSON_H
#include <fastjson/async.h>
#include <fastjson/config.h>
#include <fastjson/node.h>
#include <fastjson/parse.h>

#define NEW_JSON_DICT() (init_fj_node(FJ_NODE_DICT))
#define NEW_JSON_ARRAY() (init_fj_node(FJ_NODE_ARRAY))

typedef FJNode JSON;

typedef struct FAST_JSON_ITERATOR_STRUCT {
  JSON *current;
  JSON_LENGTH_INT index;
  JSON **items;
  JSON_LENGTH_INT length;
} JSONIterator;

JSONIterator json_iterate(JSON *node);
JSONIterator json_iterate_kv(JSON *node);
JSON *json_iterator_next(JSONIterator *iterator);

JSON *json_parse(const char *contents, JSONOptions *options);
JSON *json_parse_file(const char *filepath, JSONOptions *options);

void json_await(JSONAsync *json_async);

void json_free(JSON *node);

int json_write(JSON *node, const char *filepath, const char *mode);

JSON *json_get(JSON *node, const char *key);

JSON *json_get_array_item(JSON *node, JSON_LENGTH_INT index);
float json_get_array_item_float(JSON *node, JSON_LENGTH_INT index);
uint32_t json_get_array_item_uint32(JSON *node, JSON_LENGTH_INT index);
uint64_t json_get_array_item_uint64(JSON *node, JSON_LENGTH_INT index);
int64_t json_get_array_item_int64(JSON *node, JSON_LENGTH_INT index);
int32_t json_get_array_item_int32(JSON *node, JSON_LENGTH_INT index);
char *json_get_array_item_string(JSON *node, JSON_LENGTH_INT index);

float json_get_float(JSON *node, const char *key);
// double json_get_double(JSON *node, const char *key);
uint32_t json_get_uint32(JSON *node, const char *key);
uint64_t json_get_uint64(JSON *node, const char *key);

int32_t json_get_int32(JSON *node, const char *key);
int64_t json_get_int64(JSON *node, const char *key);
int64_t json_get_int(JSON *node, const char *key);

float json_get_number(JSON *node, const char *key);
char *json_get_string(JSON *node, const char *key);
JSONIterator json_get_array(JSON *node, const char *key);

JSON *json_set(JSON *node, const char *key, JSON *value);
JSON *json_set_float(JSON *node, const char *key, float value);
// JSON* json_set_double(JSON* node, const char* key, double value);

JSON *json_set_uint32(JSON *node, const char *key, uint32_t value);
JSON *json_set_uint64(JSON *node, const char *key, uint64_t value);

JSON *json_set_int32(JSON *node, const char *key, int32_t value);
JSON *json_set_int64(JSON *node, const char *key, int64_t value);

JSON *json_set_int(JSON *node, const char *key, int64_t value);
JSON *json_set_number(JSON *node, const char *key, float value);
JSON *json_set_string(JSON *node, const char *key, const char *value);
JSON *json_set_array(JSON *node, const char *key, JSON **children,
                     uint32_t length);
JSON *json_push(JSON *node, const char *key, JSON *value);

void json_keys(JSON *node, char ***keys, JSON_LENGTH_INT *len);

void json_values(JSON *node, JSON **values, JSON_LENGTH_INT *len);

unsigned int json_is_array(JSON *json);
unsigned int json_is_dict(JSON *json);

char *json_key(JSON *node);

char *json_stringify(JSON *node);

#endif
