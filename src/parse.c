#include <fastjson/mem.h>
#include <fastjson/parse.h>
#include <fastjson/utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPTURE_ERROR(node, code)                                              \
  {                                                                            \
    if (!code) {                                                               \
      fprintf(stderr, "json error code %d\n", code);                           \
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

static inline void node_attach_string(FJParser *parser, FJNode *node) {
  if (parser->token->start && parser->token->end) {
    node->str_start = parser->token->start;
    node->str_end = parser->token->end;
    node->value_str = fj_node_string(node, parser->lexer);
  } else {
    node->value_str = strdup(parser->token->value ? parser->token->value : "");
  }
}

static inline FJNode *parse_string(FJParser *parser) {
  FJNode *node = init_fj_node(FJ_NODE_STRING);
  node_attach_string(parser, node);

  CAPTURE_ERROR(node, next(parser, FJ_TOKEN_STRING));

  return node;
}

static inline FJNode *parse_id(FJParser *parser) {
  FJNode *node = init_fj_node(FJ_NODE_ID);
  node_attach_string(parser, node);

  CAPTURE_ERROR(node, next(parser, FJ_TOKEN_ID));

  return node;
}

static inline FJNode *parse_int(FJParser *parser) {
  FJNode *node = init_fj_node(FJ_NODE_INT);
  char *value = parser->token->value;
  if (!value)
    return node;

  if (parser->ignore_int_types) {
    node->value_int = atoi(value);
    node->value_uint32 = node->value_int;
  } else {
    node->value_uint64 = atoll(value);
    node->value_uint32 = atol(value);
    node->value_int = node->value_uint32;

    JSONIntegerType int_type = fj_string_int_type(value);

    switch (int_type) {
    case JSON_UINT32:
      node->type = FJ_NODE_UINT32;
      break;
    case JSON_UINT64:
      node->type = FJ_NODE_UINT64;
      break;
    case JSON_INT32:
      node->type = FJ_NODE_INT32;
      break;
    case JSON_INT64:
      node->type = FJ_NODE_INT64;
      break;
    default: { node->type = FJ_NODE_INT; }
    }
  }

  CAPTURE_ERROR(node, next(parser, FJ_TOKEN_INT));

  return node;
}

static inline FJNode *parse_float(FJParser *parser) {
  FJNode *node = init_fj_node(FJ_NODE_FLOAT);
  node->value_float = parser->token->value ? atof(parser->token->value) : 0;
  //  node->value_double = (double)node->value_float;
  CAPTURE_ERROR(node, next(parser, FJ_TOKEN_FLOAT));

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
    fj_node_assign_dict(node, (const char *)child->key->value_str,
                        child->value);
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
      fj_node_assign_dict(node, (const char *)child->key->value_str,
                          child->value);
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

  FJNode *child = parse_entry(parser);
  if (!child || child->type == FJ_NODE_ERROR)
    return node;
  fj_node_add_child(node, child);

  while (parser->token->type == FJ_TOKEN_COMMA) {
    CAPTURE_ERROR(node, next(parser, FJ_TOKEN_COMMA));

    FJNode *child = parse_entry(parser);
    if (!child)
      break;
    if (child->type == FJ_NODE_ERROR)
      break;
    fj_node_add_child(node, child);
  }
  /*
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
    }*/

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
  case FJ_TOKEN_INT:
    return parse_int(parser);
    break;
  case FJ_TOKEN_FLOAT:
    return parse_float(parser);
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
