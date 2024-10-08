#include "isearch.h"

#include <string.h>

#include "buffer.h"
#include "cmdline.h"
#include "cursor.h"
#include "globals.h"

#define SEARCH_BACKWARD 0
#define SEARCH_FORWARD  1

static buffer_t obuffer;

static void isearch_search(buffer_t *B, const char *query, u8 dir) {
  cursor_t *C = B->cursor;
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

  cursor_bol(B);
  for (u32 i = 0; i < y; i++)
    if (dir == SEARCH_FORWARD) cursor_down(B);
    else cursor_up(B);

  for (u32 i = 0; i < g_isearch.x; i++)
    cursor_right(B);

  // adjust to show matches in screen
  if (C->coloff > 0 && C->x >= g_isearch.qlen) {
    C->coloff += g_isearch.qlen;
    C->x -= g_isearch.qlen;
  }
}

static void isearch_return(buffer_t *B) {
  g_mode = MODE_INSERT;
  clear_status_message();
}

void isearch(i32 opt) {
  buffer_t *B = buffer_get();
  const char *cmd = cmdline_text();
  switch (opt) {
    case -1:
      isearch_search(B, cmd, SEARCH_BACKWARD);
      break;
    case 0:
      isearch_return(B);
      break;
    case 1:
      isearch_search(B, cmd, SEARCH_FORWARD);
      break;
  }
}

void isearch_abort(buffer_t *B) {
  cursor_set(B, &obuffer);
  g_mode = MODE_INSERT;
  clear_status_message();
}

void isearch_start(buffer_t *B) {
  obuffer = *B;
  cmdline_init("I-Search: ");
  g_mode = MODE_SEARCH;
  g_cmd_func = isearch;
  g_isearch = (isearch_t) {.lp = NULL, .qlen = 0, .x = 0};
}
