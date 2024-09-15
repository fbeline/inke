#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "vt100.h"

void die(const char* msg, ...) {
  va_list ap;

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);

  /* vt_clear_screen(); */

  exit(1);
}

