#ifndef FAST_JSON_TOKEN_H
#define FAST_JSON_TOKEN_H

typedef enum {
  FJ_TOKEN_UNKNOWN,
  FJ_TOKEN_ID,
  FJ_TOKEN_LBRACE,
  FJ_TOKEN_RBRACE,
  FJ_TOKEN_LBRACKET,
  FJ_TOKEN_RBRACKET,
  FJ_TOKEN_NUMBER,
  FJ_TOKEN_FLOAT,
  FJ_TOKEN_INT,
  FJ_TOKEN_STRING,
  FJ_TOKEN_SINGLE_QUOTE,
  FJ_TOKEN_DOUBLE_QUOTE,
  FJ_TOKEN_COLON,
  FJ_TOKEN_COMMA,
  FJ_TOKEN_EOF,
} FJTokenType;

typedef struct FAST_JSON_TOKEN_STRUCT {
  FJTokenType type;
  char *value;
  char c;
  char *start;
  char *end;

} FJToken;

#define FJ_TOKEN(type, value, character) ((FJToken){type, value, character})

void fj_token_to_str(FJToken *token, char *buffer);

const char *fj_token_type_to_str(FJTokenType type);
#endif
