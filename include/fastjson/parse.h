#ifndef FAST_JSON_PARSE_H
#define FAST_JSON_PARSE_H
#include <fastjson/async.h>
#include <fastjson/lex.h>
#include <fastjson/node.h>

typedef struct FAST_JSON_PARSER_STRUCT {
  FJLexer *lexer;
  FJToken *token;
  unsigned int ignore_int_types;
} FJParser;

FJNode *parse(FJParser *parser);
FJNode *parse_entry(FJParser *parser);

#endif
