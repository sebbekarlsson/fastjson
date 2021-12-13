#ifndef FAST_JSON_MEM_H
#define FAST_JSON_MEM_H
#include <stdint.h>
void *fj_calloc(uint32_t bytes, uint32_t length);

#define FJ_CALLOC(T, length) (T *)fj_calloc(sizeof(T), length)
#endif
