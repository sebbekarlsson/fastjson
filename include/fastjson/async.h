#ifndef FAST_JSON_ASYNC_H
#define FAST_JSON_ASYNC_H
#include <fastjson/options.h>
#include <pthread.h>
#include <stdint.h>

struct FAST_JSON_NODE_STRUCT;

typedef struct FAST_JSON_ASYNC_RESULT {
  struct FAST_JSON_NODE_STRUCT *data;
  pthread_t id;
  char *contents;
  pthread_mutex_t lock;
  uint32_t row;
  uint32_t col;
  float progress;
  JSONOptions *options;
} JSONAsync;

JSONAsync *json_parse_async(const char *contents, JSONOptions *options);
JSONAsync *json_parse_file_async(const char *filepath, JSONOptions *options);

void json_async_free(JSONAsync *as);
#endif
