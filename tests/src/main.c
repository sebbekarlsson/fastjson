#include <assert.h>
#include <fastjson/json.h>
#include <stdio.h>

#define TEST(name, source, root_name, body)                                    \
  void test_##name() {                                                         \
    printf("TEST: %s\n", #name);                                               \
    JSON *root_name = json_parse_file(source);                                 \
    {body} printf("(%s) OK.\n", #name);                                        \
  }

// TODO: test more specific issues rather than data != 0

TEST(simple_dict_json, "sources/simple_dict.json", data, {
  assert(data != 0);
  json_free(data);
});

TEST(escaped_json, "sources/escaped.json", data, {
  assert(data != 0);
  json_free(data);
});

TEST(array_json, "sources/array.json", data, {
  assert(data != 0);
  assert(json_is_array(data));
  json_free(data);
});

TEST(large_file_json, "sources/large-file.json", data, {
  assert(data != 0);
  json_free(data);
});

int main(int argc, char *argv[]) {
  printf("Running tests...\n");
  test_simple_dict_json();
  test_escaped_json();
  test_array_json();
  test_large_file_json();
  printf("Done.\n");
  return 0;
}
