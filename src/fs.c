#include "fs.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int FileWrite(const char* filename, const char* buf) {
  int len;
  FILE *file = fopen(filename, "w");

  if (file == NULL) {
    printf("Error writing to file %s\n", filename);
    return 1;
  } 
  fprintf(file, "%s", buf);
  fclose(file);

  return 0;
}

