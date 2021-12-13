#include <fastjson/mem.h>
#include <fastjson/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PADDING 128

FJString fj_string_init(FJString *str, const char *value) {
  str->length = strlen(value);
  uint32_t size = str->length + PADDING;
  str->value = FJ_CALLOC(char, size);
  strcat(str->value, value);
  str->bytes_left =
      (((size - 1) - str->length) * sizeof(char)); // PADDING * sizeof(char);

  return *str;
}

void fj_string_concat(FJString *str, const char *value) {
  if (!value || !str)
    return;
  uint32_t length = strlen(value);
  uint32_t bytes_needed = (length + 1) * sizeof(char);
  uint32_t bytes_left = str->bytes_left;

  if (str->bytes_left <= bytes_needed) {
    uint32_t extra_size = (bytes_needed + PADDING) * sizeof(char);
    str->bytes_left += extra_size;
    str->value = (char *)realloc(str->value, str->bytes_left);

    if (!str->value) {
      fprintf(stderr, "fj_string_concat: realloc failed.\n");
      return;
    }

    memset(&str->value[bytes_left], '\0', extra_size);
  }

  str->length += length;
  str->bytes_left -= bytes_needed;
  strcat(str->value, value);
}

void fj_string_concat_char(FJString *str, char c) {
  fj_string_concat(str, (char[]){c, '\0'});
}
