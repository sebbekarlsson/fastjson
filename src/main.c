#include <fastjson/json.h>

int main(int argc, char *argv[]) {
  JSON *data = json_parse("{ \"age\": 33 }");
  json_free(data);
  return 0;
}
