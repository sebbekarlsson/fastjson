#include <fastjson/io.h>
#include <stdio.h>
#include <stdlib.h>

char *fj_read_file(const char *filepath) {
  FILE *fp = fopen(filepath, "r");
  if (!fp) {
    fprintf(stderr, "Failed to read file `%s`\n", filepath);
    return 0;
  }
  char *buffer = NULL;
  size_t len;
  ssize_t bytes_read = getdelim(&buffer, &len, '\0', fp);
  if (!(bytes_read != -1)) {
    fprintf(stderr, "Failed to read file `%s` (no bytes)\n", filepath);
    fclose(fp);
    return 0;
  }

  fclose(fp);

  return buffer;
}
