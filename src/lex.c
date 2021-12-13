#include <ctype.h>
#include <fastjson/lex.h>
#include <fastjson/utils.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#define FINISHED(lexer) (lexer->c == '\0' || lexer->i >= lexer->length)

static inline void advance(FJLexer *lexer) {
  if (!FINISHED(lexer)) {
    lexer->i++;
    lexer->c = lexer->src[lexer->i];

    if (lexer->c == '\n' || lexer->c == '\r') {
      lexer->row += 1;
      lexer->col = 0;
    } else {
      lexer->col += 1;
    }
  }
}

static inline void concat(FJString *str, FJLexer *lexer) {
  fj_string_concat_char(str, lexer->c);
  advance(lexer);
}

static inline char peek(FJLexer *lexer, uint32_t offset) {
  return lexer->src[MAX(0, MIN(lexer->length - 1, lexer->i + offset))];
}

static inline FJTokenType char_to_token_type(char c) {
  if (isdigit(c))
    return FJ_TOKEN_NUMBER;
  if (isalnum(c))
    return FJ_TOKEN_ID;
  switch (c) {
  case '{':
    return FJ_TOKEN_LBRACE;
    break;
  case '}':
    return FJ_TOKEN_RBRACE;
    break;
  case '[':
    return FJ_TOKEN_LBRACKET;
    break;
  case ']':
    return FJ_TOKEN_RBRACKET;
    break;
  case ':':
    return FJ_TOKEN_COLON;
    break;
  case ',':
    return FJ_TOKEN_COMMA;
    break;
  case '\'':
    return FJ_TOKEN_SINGLE_QUOTE;
    break;
  case '"':
    return FJ_TOKEN_DOUBLE_QUOTE;
    break;
  default: {};
  }

  return FJ_TOKEN_UNKNOWN;
}

static inline char *collect_id(FJLexer *lexer) {
  FJString str = FJ_STRING("");

  while (isalnum(lexer->c)) {
    concat(&str, lexer);
  }

  return str.value;
}

static inline char *collect_number(FJLexer *lexer) {
  FJString str = FJ_STRING("");

  while (isdigit(lexer->c)) {
    concat(&str, lexer);
  }

  if (lexer->c == '.') {
    concat(&str, lexer);
    while (isdigit(lexer->c)) {
      concat(&str, lexer);
    }
  }

  return str.value;
}

static void skip_white(FJLexer *lexer) {

  while ((!FINISHED(lexer)) && (lexer->c == ' ' || lexer->c == '\r' ||
                                lexer->c == '\n' || lexer->c == '\t')) {
    advance(lexer);
  }
}

static inline char *collect_str(FJLexer *lexer) {
  FJString str = FJ_STRING("");

  char start = lexer->c;
  advance(lexer);

  while (lexer->c != start) {
    // escaped quotes
    if (lexer->c == '\\' && peek(lexer, 1) == '\\') {
      concat(&str, lexer);
      concat(&str, lexer);
      continue;
    }
    if (lexer->c == '\\' && peek(lexer, 1) == start) {
      advance(lexer);
    }
    concat(&str, lexer);
  }

  advance(lexer);

  return str.value;
}

FJToken lex(FJLexer *lexer) {
  skip_white(lexer);
  if (FINISHED(lexer))
    return FJ_TOKEN(FJ_TOKEN_EOF, 0, 0);

  FJTokenType type = char_to_token_type(lexer->c);
  FJToken token = FJ_TOKEN(type, 0, lexer->c);

  switch (token.type) {
  case FJ_TOKEN_ID: {
    token.value = collect_id(lexer);
    token.type = FJ_TOKEN_ID;
    return token;
  } break;
  case FJ_TOKEN_NUMBER: {
    token.value = collect_number(lexer);
    token.type = FJ_TOKEN_NUMBER;
    return token;
  } break;
  case FJ_TOKEN_SINGLE_QUOTE:
  case FJ_TOKEN_DOUBLE_QUOTE: {
    token.value = collect_str(lexer);
    token.type = FJ_TOKEN_STRING;
    return token;
  } break;
  default: {}; break;
  }

  if (!FINISHED(lexer)) {
    advance(lexer);
  } else {
    token.type = FJ_TOKEN_EOF;
  }
  return token;
}

void fj_lexer_get_info(FJLexer *lexer, char *buffer) {
  sprintf(buffer, "%d:%d", lexer->row, lexer->col);
}
