#ifndef FAST_JSON_STRING_VIEW_H
#define FAST_JSON_STRING_VIEW_H
#include <stdint.h>

#define FAST_JSON_STRING_VIEW_CAP 256

typedef struct {
  const char *ptr;
  int64_t length;
  char tmp[FAST_JSON_STRING_VIEW_CAP];
} JSONStringView;

const char *json_string_view_get_value(JSONStringView *view);

void json_string_view_copy(JSONStringView src, JSONStringView *dest);

#endif
