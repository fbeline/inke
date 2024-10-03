#include "isearch.h"

#include <string.h>

#include "cmdline.h"
#include "cursor.h"
#include "globals.h"

static cursor_t ocursor = {0};

static void isearch_forward(cursor_t *C, const char *query) {
  line_t *l = C->clp;
  u32 ocy = C->y + C->rowoff;
  u32 offset = C->x + C->coloff;

  g_isearch = (isearch_t){.x = 0, .y = ocy, .qlen = 0};

  while (l != NULL) {
    char *match = strstr(l->ds->buf + offset, query);
    if (match) {
      g_isearch.x = match - (l->ds->buf + offset);
      g_isearch.qlen = strlen(query);
      break;
    }
    l = l->next;
    g_isearch.y++;
    offset = 0;
  }

  if (l == NULL) return;

  for (u32 i = 0, max = g_isearch.y - ocy; i < max; i++) {
    cursor_down(C);
    cursor_bol(C);
  }

  for (u32 i = 0, max = g_isearch.x - offset; i < max; i++)
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
  g_isearch = (isearch_t) {0, 0, 0};
}
