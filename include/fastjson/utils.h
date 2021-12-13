#ifndef FAST_JSON_UTILS_H
#define FAST_JSON_UTILS_H
#include <stdint.h>
typedef struct FAST_JSON_STRING_STRUCT {
  char *value;
  uint32_t bytes_used;
  uint32_t bytes_left;
  uint32_t length;
} FJString;

FJString fj_string_init(FJString *str, const char *value);

void fj_string_concat(FJString *str, const char *value);

void fj_string_concat_char(FJString *str, char c);

#define FJ_STRING(value) fj_string_init(&((FJString){0, 0, 0, 0}), value);
#endif
