# fastjson
> Fast JSON parsing library written in C

## Usage

### Parse string as json
``` C
#include <fastjson/json.h>

int main(int argc, char *argv[]) {
  JSON *data = json_parse("{ \"age\": 33 }");
  json_free(data);
  return 0;
}

```

### Parse file as json
```C 
#include <fastjson/json.h>
int main(int argc, char *argv[]) {
  JSON *data = json_parse_file(argv[1]);
  json_free(data);

  return 0;
}
```

### Get value from json dict
```C 
#include <fastjson/json.h>
int main(int argc, char *argv[]) {
  JSON *data = json_parse_file(argv[1]);
  
  char* name = json_get_string(data, "name");
  float age = json_get_float(data, "age"); // or json_get_int, json_get_number
  JSONIterator it = json_get_array(data, "children");
  JSON* location = json_get(data, "location"); // grab a nested dict

  json_free(data);
  
  return 0;
}
```


### Iterating over keys & values
``` C
#include <fastjson/json.h>

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
```

## Put it in your project (CMake)
``` cmake
include(FetchContent)

FetchContent_Declare(
  fjson_static
  GIT_REPOSITORY https://github.com/sebbekarlsson/fastjson.git
)

FetchContent_MakeAvailable(fjson_static)
target_link_libraries(YOUR_TARGET_NAME PUBLIC fjson_static)
```
