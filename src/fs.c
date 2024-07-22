#include "fs.h"

#include <stdlib.h>
#include <stdio.h>

#define ERROR_RETURN(R, ...) { fprintf(stderr, __VA_ARGS__); return R; }
#define IO_READ_CHUNK_SIZE 2097152

File FileRead(const char *filename) {
  File file = {.is_valid = false};
  FILE *fp = fopen(filename, "rb");
  if (!fp || ferror(fp)) {
    ERROR_RETURN(file, "Failed to open file: %s", filename, errno);
  }

  char *data = NULL;
  char *tmp;
  size_t used = 0;
  size_t size = 0;
  size_t n;

  while (true) {
    if (used + IO_READ_CHUNK_SIZE + 1 > size) {
      size = size + IO_READ_CHUNK_SIZE + 1;
      tmp = realloc(data, size);
      if (tmp == NULL) {
        ERROR_RETURN(file, "Failed to allocate memory for file: %s", filename, errno);
      }
      data = tmp;
    }

    n = fread(data + used, 1, IO_READ_CHUNK_SIZE, fp);
    if (ferror(fp)) {
      ERROR_RETURN(file, "Failed to read file: %s", filename, errno);
    }

    if (n == 0) {
      break;
    }

    used += n;
  }

  if (ferror(fp)) {
    ERROR_RETURN(file, "Failed to read file: %s", filename, errno);
  }
  tmp = realloc(data, used + 1);
  if (tmp == NULL) {
    ERROR_RETURN(file, "Failed to allocate memory for file: %s", filename, errno);
  }
  data = tmp;
  data[used] = 0;

  file.data = data;
  file.len = used;
  file.is_valid = true;

  return file;
}

int FileWrite(const char *buffer, size_t size, const char *filename) {
  FILE *fp = fopen(filename, "wb");
  if (!fp || ferror(fp)) {
    ERROR_RETURN(-1, "Failed to open file: %s", filename, errno);
  }

  size_t written = fwrite(buffer, size, 1, fp);

  fclose(fp);

  if (written != 1) {
    ERROR_RETURN(1, "Failed to write file: %s", filename, errno);
  }

  return 1;
}

