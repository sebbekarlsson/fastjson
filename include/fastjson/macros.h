#ifndef FAST_JSON_MACROS_H
#define FAST_JSON_MACROS_H
#include <stdlib.h>
#define OR(a, b) (a ? a : b)
#define NEW(T) ((T *)calloc(1, sizeof(T)))

#define FJSON_GENERATE_ENUM(ENUM) ENUM,
#define FJSON_GENERATE_STRING(STRING) #STRING,

#endif
