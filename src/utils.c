#include <fastjson/mem.h>
#include <fastjson/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#define PADDING 128

FJString fj_string_init(FJString *str, const char *value) {
  str->length = strlen(value);
  uint32_t size = str->length + PADDING;
  str->value = FJ_CALLOC(char, size);
  strcat(str->value, value);
  str->bytes_left = PADDING * sizeof(char); // PADDING * sizeof(char);

  return *str;
}

void fj_string_concat(FJString *str, const char *value) {
  if (!value || !str)
    return;
  uint32_t length = strlen(value);
  uint32_t bytes_needed = (length + 1) * sizeof(char);
  //uint32_t bytes_left = str->bytes_left;
  uint32_t bytes_used = str->length * sizeof(char);
  uint32_t new_pos = bytes_used;

  //char *end = &str->value[str->length * sizeof(char)];

  if (str->bytes_left <= bytes_needed) {
    // uint32_t extra_size = (bytes_needed + PADDING) * sizeof(char);
    str->bytes_left += bytes_needed + PADDING; // extra_size;
    str->value = (char *)realloc(str->value, (bytes_used) + str->bytes_left);

    if (!str->value) {
      fprintf(stderr, "fj_string_concat: realloc failed.\n");
      return;
    }
  }

  memset(&str->value[new_pos], '\0', bytes_needed);

  str->length += length;
  str->bytes_left -= bytes_needed;
  strcat(str->value, value);
}

void fj_string_concat_char(FJString *str, char c) {
  fj_string_concat(str, (char[]){c, '\0'});
}

#define STR_LEN_32 11 // 10 + 1
#define STR_LEN_64 21 // 20 + 1

JSONIntegerType fj_string_int_type(const char* str) {
  if (!str) return JSON_UINT32;

  unsigned int is_signed = (unsigned int)(str[0] == '-');

  if (strlen(str) >= STR_LEN_64) return is_signed ? JSON_INT64 : JSON_UINT64;
  return is_signed ? JSON_INT32 : JSON_UINT32;
}
