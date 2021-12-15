#include <fastjson/async.h>
#include <fastjson/io.h>
#include <fastjson/json.h>
#include <fastjson/lex.h>
#include <fastjson/mem.h>
#include <stdio.h>
#include <stdlib.h>

static void *json_async_process(void *ptr) {
  if (!ptr) {
    fprintf(stderr, "json: background process received null pointer.\n");
    pthread_exit(0);
    return 0;
  }
  JSONAsync *as = (JSONAsync *)ptr;

  if (!as->contents) {
    fprintf(stderr,
            "json: background process received null contents to parse.\n");
    pthread_exit(0);
    return 0;
  }

  JSONOptions *options = as->options;

  FJLexer lexer = FJ_LEXER_ASYNC(as->contents, as);

  if (options) {
    lexer.optimized_strings = options->optimized_strings;
  }

  FJParser parser = (FJParser){&lexer, 0};


  if (options) {
    parser.ignore_int_types = options->ignore_int_types;
  }

  JSON *root = parse(&parser);

  pthread_mutex_lock(&as->lock);
  root->source = as->contents;
  as->data = root;
  pthread_mutex_unlock(&as->lock);

  return 0;
}

JSONAsync *json_parse_async(const char *contents, JSONOptions *options) {
  if (!contents)
    return 0;
  JSONAsync *as = FJ_CALLOC(JSONAsync, 1);

  if (options) {
    as->options = FJ_CALLOC(JSONOptions, 1);
    as->options->optimized_strings = options->optimized_strings;
    as->options->ignore_int_types = options->ignore_int_types;
  }

  as->contents = strdup(contents);

  if (pthread_mutex_init(&as->lock, 0)) {
    fprintf(stderr, "json: failed to create mutex.\n");
    return as;
  }

  if (pthread_create(&as->id, 0, json_async_process, as)) {
    fprintf(stderr, "json: failed to create background async process.\n");
  }

  return as;
}

JSONAsync *json_parse_file_async(const char *filepath, JSONOptions *options) {
  char *contents = fj_read_file(filepath);
  if (!contents) {
    fprintf(stderr, "json: Failed to parse file `%s`\n", filepath);
    return 0;
  }

  JSONAsync *as = json_parse_async(contents, options);
  free(contents);
  return as;
}

void json_async_free(JSONAsync *as) {
  if (as->options) {
    free(as->options);
  }
  free(as);
}
