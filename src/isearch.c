#include "isearch.h"

#include "cmdline.h"


void isearch_forward(cursor_t *C, const char *text) {
  // TODO
}

void isearch_reverse(cursor_t *C, const char *text) {
  // TODO
}

void isearch(cursor_t *C, int ch) {
  char *cmd = cmdline_text();
  switch (ch) {
    case -1:
      isearch_reverse(C, cmd);
      break;
    case 1:
      isearch_forward(C, cmd);
      break;
  }
}
