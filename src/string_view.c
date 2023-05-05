#include <fastjson/string_view.h>

#include <stdlib.h>
#include <string.h>

const char *json_string_view_get_value(JSONStringView *view) {
  if (!view || (view->ptr == 0 || view->length <= 0))
    return 0;
  memset(&view->tmp[0], 0, FAST_JSON_STRING_VIEW_CAP * sizeof(char));
  memcpy(&view->tmp[0], view->ptr, view->length * sizeof(char));
  return view->tmp;
}

void json_string_view_copy(JSONStringView src, JSONStringView *dest) {
  if (!dest)
    return;
  dest->length = src.length;
  dest->ptr = src.ptr;
}
