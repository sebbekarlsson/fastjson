#include <fastjson/mem.h>
#include <fastjson/parse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPTURE_ERROR(node, code)                                              \
  {                                                                            \
    if (!code) {                                                               \
      node->type = FJ_NODE_ERROR;                                              \
      return node;                                                             \
    }                                                                          \
  }

static inline int next(FJParser *parser, FJTokenType type) {
  if (parser->token->type != type) {
    char expected_buff[256];
    char got_buff[256];
    char lexer_info[256];
    fj_lexer_get_info(parser->lexer, lexer_info);
    sprintf(expected_buff, "%s\n", fj_token_type_to_str(type));
    fj_token_to_str(parser->token, got_buff);

    fprintf(stderr,
            "fastjson: Parser (%s): Unexpected token %s, was expecting %s\n",
            lexer_info, got_buff, expected_buff);
    return 0;
  }

  if (parser->token && parser->token->value) {
    free(parser->token->value);
    parser->token->value = 0;
  }
  *parser->token = lex(parser->lexer);

  return 1;
}

static inline FJNode *parse_string(FJParser *parser) {
  FJNode *node = init_fj_node(FJ_NODE_STRING);
  node->value_str = strdup(parser->token->value ? parser->token->value : "");
  CAPTURE_ERROR(node, next(parser, FJ_TOKEN_STRING));

  return node;
}

static inline FJNode *parse_id(FJParser *parser) {
  FJNode *node = init_fj_node(FJ_NODE_ID);
  node->value_str = strdup(parser->token->value ? parser->token->value : "");
  CAPTURE_ERROR(node, next(parser, FJ_TOKEN_ID));

  return node;
}

static inline FJNode *parse_number(FJParser *parser) {
  FJNode *node = init_fj_node(FJ_NODE_NUMBER);
  node->value_num = parser->token->value ? atof(parser->token->value) : 0;
  CAPTURE_ERROR(node, next(parser, FJ_TOKEN_NUMBER));

  return node;
}

static inline FJNode *parse_tuple(FJParser *parser) {
  FJNode *node = init_fj_node(FJ_NODE_TUPLE);
  FJNode *a = parse_string(parser);

  CAPTURE_ERROR(node, next(parser, FJ_TOKEN_COLON));

  FJNode *b = parse_entry(parser);

  node->key = a;
  node->value = b;
  return node;
}

static inline FJNode *parse_dict(FJParser *parser) {
  FJNode *node = init_fj_node(FJ_NODE_DICT);
  CAPTURE_ERROR(node, next(parser, FJ_TOKEN_LBRACE))

  if (parser->token->type == FJ_TOKEN_RBRACE) {
    CAPTURE_ERROR(node, next(parser, FJ_TOKEN_RBRACE));
    return node;
  }

  FJNode *child = parse_tuple(parser);
  if (!child || child->type == FJ_NODE_ERROR)
    return node;
  fj_node_add_child(node, child);

  if (child && child->key && child->key->value_str) {
    fj_node_assign_dict(node, (const char *)child->key, child->value);
  }

  while (parser->token->type == FJ_TOKEN_COMMA) {
    CAPTURE_ERROR(node, next(parser, FJ_TOKEN_COMMA));

    FJNode *child = parse_tuple(parser);
    if (!child)
      break;
    if (child->type == FJ_NODE_ERROR)
      break;
    fj_node_add_child(node, child);

    if (child && child->key && child->key->value_str) {
      fj_node_assign_dict(node, (const char *)child->key, child->value);
    }
  }

  CAPTURE_ERROR(node, next(parser, FJ_TOKEN_RBRACE));

  return node;
}

static inline FJNode *parse_array(FJParser *parser) {
  FJNode *node = init_fj_node(FJ_NODE_ARRAY);
  if (!next(parser, FJ_TOKEN_LBRACKET))
    return node;

  if (parser->token->type == FJ_TOKEN_RBRACKET) {
    next(parser, FJ_TOKEN_RBRACKET);
    return node;
  }

  while (parser->token->type != FJ_TOKEN_RBRACKET) {
    FJNode *child = parse_entry(parser);
    if (!child)
      break;
    if (child->type == FJ_NODE_ERROR)
      break;
    fj_node_add_child(node, child);

    if (parser->token->type == FJ_TOKEN_COMMA) {
      CAPTURE_ERROR(child, next(parser, FJ_TOKEN_COMMA));
    }
  }

  CAPTURE_ERROR(node, next(parser, FJ_TOKEN_RBRACKET));

  return node;
}

FJNode *parse_entry(FJParser *parser) {
  switch (parser->token->type) {
  case FJ_TOKEN_LBRACKET:
    return parse_array(parser);
    break;
  case FJ_TOKEN_LBRACE:
    return parse_dict(parser);
    break;
  case FJ_TOKEN_STRING:
    return parse_string(parser);
    break;
  case FJ_TOKEN_ID:
    return parse_id(parser);
    break;
  case FJ_TOKEN_NUMBER:
    return parse_number(parser);
    break;
  default: {};
  }
  FJNode *noop = init_fj_node(FJ_NODE_NOOP);
  return noop;
}

FJNode *parse(FJParser *parser) {
  if (!parser->lexer) {
    fprintf(stderr, "json error: The parser has no initialized lexer.\n");
    return 0;
  }
  FJToken token = lex(parser->lexer);
  parser->token = &token;
  return parse_entry(parser);
  /*FJNode *node = init_fj_node();
  node->type = FJ_NODE_ARRAY;

  while ((*parser->token).type != FJ_TOKEN_EOF) {
    FJNode* child = parse_entry(parser);
    if (child->type == FJ_NODE_ERROR) break;
    fj_node_add_child(node, child);
    *parser->token = lex(parser->lexer);
  }
  return node;*/
}
