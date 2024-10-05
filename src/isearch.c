#include "isearch.h"

#include <string.h>

#include "cmdline.h"
#include "cursor.h"
#include "globals.h"

#define SEARCH_BACKWARD 0
#define SEARCH_FORWARD  1

static cursor_t oc;

static void isearch_search(cursor_t *C, const char *query, u8 dir) {
  line_t *l = C->clp;
  u32 offset = C->x + C->coloff;
  u32 y = 0;
  u32 x = 0;

  while (l != NULL) {
    const char *match = strstr(l->ds->buf + offset, query);

    if (match == NULL) {
      offset = 0;
      y++;
      l = dir == SEARCH_FORWARD ? l->next : l->prev;
      continue;
    }

    x = match - l->ds->buf;
    if (g_isearch.lp == l && g_isearch.x == x) {
      offset += strlen(query);
      continue;
    }

    g_isearch.lp = l;
    g_isearch.x = x;
    g_isearch.qlen = strlen(query);
    break;
  }

  if (l == NULL) return;

  cursor_bol(C);
  for (u32 i = 0; i < y; i++)
    if (dir == SEARCH_FORWARD) cursor_down(C);
    else cursor_up(C);

  for (u32 i = 0; i < g_isearch.x; i++)
    cursor_right(C);

  // adjust to show matches in screen
  if (C->coloff > 0 && C->x >= g_isearch.qlen) {
    C->coloff += g_isearch.qlen;
    C->x -= g_isearch.qlen;
  }
}

static void isearch_return(cursor_t *C) {
  g_mode = MODE_INSERT;
  clear_status_message();
}

void isearch(cursor_t *C, int opt) {
  const char *cmd = cmdline_text();
  switch (opt) {
    case -1:
      isearch_search(C, cmd, SEARCH_BACKWARD);
      break;
    case 0:
      isearch_return(C);
      break;
    case 1:
      isearch_search(C, cmd, SEARCH_FORWARD);
      break;
  }
}

void isearch_abort(cursor_t *C) {
  cursor_set(C, &oc);
  g_mode = MODE_INSERT;
  clear_status_message();
}

void isearch_start(cursor_t *C) {
  oc = *C;
  cmdline_init("I-Search: ");
  g_mode = MODE_SEARCH;
  g_cmd_func = isearch;
  g_isearch = (isearch_t) {.lp = NULL, .qlen = 0, .x = 0};
}
