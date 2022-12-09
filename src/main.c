#include <fastjson/json.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  JSONOptions options = {};
  options.optimized_strings = 1;

  JSON *j = json_parse_file(argv[1], &options);


  JSONIterator it = {0};

  it = json_iterate_kv(j);

  JSON* kv = 0;
  while ((kv = json_iterator_next(&it))) {
    if (json_array_includes_string(kv->value, "person ", true)) {
      printf("yeah!\n");
    }
  }
  printf("done\n");
  return 0;
}

int main2(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Please specify file.\n");
    return 1;
  }

  unsigned int show_progress =
      argc >= 3 && argv[2] && strcmp(argv[2], "progress") == 0;

  JSONOptions options = {};
  options.optimized_strings = 1;
  options.ignore_int_types = 0;
  JSONAsync *as = json_parse_file_async(argv[1], &options);

  if (show_progress) {
    while (as->data == 0) {
      printf("\rprogress: %3.3f%%", as->progress);
      fflush(stdout);
    }

    printf("\n");
  }

  json_await(as);
  printf("%s\n", json_stringify(as->data));
  json_free(as->data);
  json_async_free(as);

  if (show_progress) {
    printf("done parsing.\n");
  }

  return 0;
}
