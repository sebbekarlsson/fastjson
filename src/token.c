#include <fastjson/token.h>
#include <stdio.h>

void fj_token_to_str(FJToken *token, char *buffer) {
  const char *typename = fj_token_type_to_str(token->type);
  const char *str_value = token->value ? token->value : "";
  char c = token->c;

  sprintf(buffer, "<token type=`%s` c=`%c` value=`%s`/>", typename, c,
          str_value);
}

const char *fj_token_type_to_str(FJTokenType type) {
  switch (type) {
  case FJ_TOKEN_UNKNOWN:
    return "FJ_TOKEN_UNKNOWN";
    break;
  case FJ_TOKEN_ID:
    return "FJ_TOKEN_ID";
    break;
  case FJ_TOKEN_LBRACE:
    return "FJ_TOKEN_LBRACE";
    break;
  case FJ_TOKEN_RBRACE:
    return "FJ_TOKEN_RBRACE";
    break;
  case FJ_TOKEN_LBRACKET:
    return "FJ_TOKEN_LBRACKET";
    break;
  case FJ_TOKEN_RBRACKET:
    return "FJ_TOKEN_RBRACKET";
    break;
  case FJ_TOKEN_NUMBER:
    return "FJ_TOKEN_NUMBER";
    break;
  case FJ_TOKEN_STRING:
    return "FJ_TOKEN_STRING";
    break;
  case FJ_TOKEN_SINGLE_QUOTE:
    return "FJ_TOKEN_SINGLE_QUOTE";
    break;
  case FJ_TOKEN_DOUBLE_QUOTE:
    return "FJ_TOKEN_DOUBLE_QUOTE";
    break;
  case FJ_TOKEN_COLON:
    return "FJ_TOKEN_COLON";
    break;
  case FJ_TOKEN_COMMA:
    return "FJ_TOKEN_COMMA";
    break;
  case FJ_TOKEN_EOF:
    return "FJ_TOKEN_EOF";
    break;
  default: { return "FJ_TOKEN_UNKNOWN"; } break;
  }
}
