#include "isearch.h"

#include "cmdline.h"

static void isearch_forward(cursor_t *C, const char *text) {
  // TODO
}

static void isearch_reverse(cursor_t *C, const char *text) {
  // TODO
}

static void isearch_return(cursor_t *C) {
  // TODO
}

void isearch(cursor_t *C, int opt) {
  const char *cmd = cmdline_text();
  switch (opt) {
    case -1:
      isearch_reverse(C, cmd);
      break;
    case 0:
      isearch_return(C);
      break;
    case 1:
      isearch_forward(C, cmd);
      break;
  }
}
