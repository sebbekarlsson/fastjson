#include <fastjson/json.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  JSON *data = json_parse_file(argv[1]);
  JSONIterator it = json_iterate(data);

  JSON *node = 0;
  while ((node = json_iterator_next(&it)) != 0) {
    if (json_is_dict(node)) {
      JSONIterator kvit = json_iterate_kv(node);

      JSON *kv = 0;
      while ((kv = json_iterator_next(&kvit)) != 0) {
        char *key = json_key(kv);
        char *value = json_get_string(kv, key);
        if (!key || !value)
          continue;
        printf("%s:%s\n", key, value);
      }
    }
  }

  json_free(data);

  return 0;
}
