# fastjson
> Fast JSON parsing library written in C

## About
> A fast JSON parsing library for C, with support for asynchronous parsing.  
> This library can also be used for _writing_ json, but there's no "public" API
> for it; look through the source code to find out how to do it.  
>  
> To take advantage of the full speed, set the json options to:
> `{ .optimized_strings = 1 }` when calling any of the parse functions;  
> there's really no disadvantage of doing so, it might be set to `1` by
> default in future versions.

### Usage
> The main functions are:

``` C
JSON *json_parse(const char *contents, JSONOptions* options);
JSON *json_parse_file(const char *filepath, JSONOptions* options);

JSONAsync *json_parse_async(const char *contents, JSONOptions* options);
JSONAsync *json_parse_file_async(const char *filepath, JSONOptions* options);
```

#### Parse string as json
``` C
#include <fastjson/json.h>

int main(int argc, char *argv[]) {
  JSON *data = json_parse("{ \"age\": 33 }", 0);
  json_free(data);
  return 0;
}

```

#### Parse file as json
```C 
#include <fastjson/json.h>
int main(int argc, char *argv[]) {
  JSON *data = json_parse_file(argv[1], 0);
  json_free(data);

  return 0;
}
```

#### Get value from json dict
```C 
#include <fastjson/json.h>
int main(int argc, char *argv[]) {
  JSON *data = json_parse_file(argv[1], 0);
  
  char* name = json_get_string(data, "name");
  float age = json_get_float(data, "age"); // or json_get_int, json_get_number
  JSONIterator it = json_get_array(data, "children");
  JSON* location = json_get(data, "location"); // grab a nested dict

  json_free(data);
  
  return 0;
}
```


#### Iterating over keys & values
``` C
#include <fastjson/json.h>

int main(int argc, char *argv[]) {
  JSON *data = json_parse_file(argv[1], 0);

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

#### JSONOptions
> The recommended options:
```C
#include <fastjson/json.h>

int main(int argc, char *argv[]) {
  JSONOptions options = {};
  options.optimized_strings = 1; // this will allow for "lazy" strings. (faster & recommended)

  JSON *data = json_parse("{ \"age\": 33 }", &options);
  
  json_free(data);
  return 0;
}
```

#### Async example
``` C
#include <fastjson/json.h>

int main(int argc, char* argv[]) {
  JSONOptions options = {};
  options.optimized_strings = 1;
  JSONAsync *as = json_parse_file_async("some_file.json", &options);
  
  while (!as->data) {
    printf("\rprogress: %3.3f%%", as->progress); // progress in percentage
  }
  
  json_await(as);
  printf("\n");
  
  printf("Done parsing data.\n");
  // do things with as->data here.
  
  json_free(as->data);
  json_async_free(as);
  
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

## Tests
> To run the tests:
``` bash
./test.sh
```

## benchmarks
> To run the benchmarks (requires python):
``` bash
./bench.sh
```
