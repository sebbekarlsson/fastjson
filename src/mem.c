#include <fastjson/mem.h>
#include <stdio.h>
#include <stdlib.h>

void *fj_calloc(uint32_t bytes, uint32_t length) {
  void *buff = calloc(bytes, length);

  if (!buff) {
    fprintf(stderr, "failed to calloc (%d, %d)\n", bytes, length);
    return 0;
  }

  return buff;
}
