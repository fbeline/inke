#include "utils.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

void die(const char* msg, ...) {
  va_list ap;

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);

  exit(1);
}

void *nrealloc(void *section, usize size) {
	section = realloc(section, size);

	if (section == NULL)
		die("Olive is out of memory!\n");

	return section;
}

void *reallocstrcpy(char *dest, char *src, usize size) {
  if (dest == NULL || src == NULL)
    die("realloc & strcpy with invalid pointer\n");

  dest = nrealloc(dest, size);
  strcpy(dest, src);

  return dest;
}


