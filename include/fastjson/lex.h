#ifndef FAST_JSON_LEX_H
#define FAST_JSON_LEX_H
#include <fastjson/async.h>
#include <fastjson/token.h>
#include <stdint.h>
#include <string.h>

typedef struct FAST_JSON_LEXER_STRUCT {
  uint32_t i;
  char c;
  uint32_t length;
  char *src;
  uint32_t row;
  uint32_t col;
  JSONAsync *json_async;
  unsigned int optimized_strings;
} FJLexer;

FJToken lex(FJLexer *lexer);

void fj_lexer_get_info(FJLexer *lexer, char *buffer);

#define FJ_LEXER(src) ((FJLexer){0, src[0], strlen(src), src, 0, 0, 0, 0})
#define FJ_LEXER_ASYNC(src, json_async)                                        \
  ((FJLexer){0, src[0], strlen(src), src, 0, 0, json_async})
#endif
