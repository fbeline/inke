#include "isearch.h"

#include <string.h>

#include "cmdline.h"
#include "cursor.h"
#include "globals.h"

static cursor_t ocursor = {0};

static void isearch_forward(cursor_t *C, const char *query) {
  line_t *l = C->clp;
  u32 offset = C->x + C->coloff;
  u32 x = 0, y = 0;

  while (l != NULL) {
    char *match = strstr(l->ds->buf + offset, query);
    if (match) {
      x = match - (l->ds->buf + offset);
      break;
    }
    l = l->next;
    offset = 0;
    y++;
  }

  for (u32 i = 0; i < y; i++)
    cursor_down(C);

  for (u32 i = 0; i < x; i++)
    cursor_right(C);
}

static void isearch_reverse(cursor_t *C, const char *query) {
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

void isearch_start(cursor_t *C) {
  ocursor = *C;

  cmdline_init("I-Search: ");
  g_mode = MODE_SEARCH;
  g_cmd_func = isearch;
}
