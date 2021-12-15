#include <fastjson/json.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  JSON* dict = NEW_JSON_DICT();
  json_set_string(dict, "name", "john doe");
  json_set_int(dict, "age", 33);
  json_set_float(dict, "weight", 2132.5f);

  json_write(dict, "data.json", "w+");

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
