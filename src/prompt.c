#include "prompt.h"

#include <string.h>

#include "ds.h"
#include "globals.h"

typedef struct prompt_s {
  u32 min_x;
  u32 x;
  ds_t *ds;
} prompt_t;

static prompt_t line = {
  .min_x = 0,
  .x = 0,
  .ds = NULL
};

u32 prompt_x(void) {
  return line.x;
}

void prompt_clean(void) {
  if (line.ds == NULL)
    line.ds = dsempty();

  line.ds->buf[0] = '\0';
  line.ds->len = 0;
  line.x = 0;
  line.min_x = 0;
}

void prompt_cat(const char *str) {
  dscat(line.ds, str);
  line.x += strlen(str);
}

void prompt_insert(char ch) {
  dsichar(line.ds, line.x, ch);
  line.x++;
}

void prompt_backspace(void) {
  if (line.x == line.min_x)
    return;

  memmove(line.ds->buf + line.x - 1,
          line.ds->buf + line.x,
          line.ds->len - line.x + 1);

  line.x--;
  line.ds->len--;
  line.ds->buf[line.ds->len] = '\0';
}

void prompt_init(const char *msg) {
  if (line.ds == NULL)
    line.ds = dsempty();

  prompt_clean();

  dscat(line.ds, msg);

  line.x = line.ds->len;
  line.min_x = line.x;
}

prompt_t *prompt(void) {
  return &line;
}

void prompt_left(void) {
  if (line.x == line.min_x)
    return;

  line.x--;
}

void prompt_right(void) {
  if (line.x == line.ds->len)
    return;

  line.x++;
}

void prompt_eol(void) {
  line.x = line.ds->len;
}

void prompt_bol(void) {
  line.x = line.min_x;
}

void prompt_del_before(void) {
  if (line.x == line.min_x)
    return;

  usize diff = line.x - line.min_x;
  memmove(line.ds->buf + line.min_x,
          line.ds->buf + line.x,
          line.ds->len - line.x + 1);

  line.x = line.min_x;
  line.ds->len -= diff;
}

const char *prompt_full_text(void) {
  return line.ds->buf;
}

const char *prompt_text(void) {
  return &line.ds->buf[line.min_x];
}

void prompt_handle_char(i32 ch) {
  switch (ch) {
    case ENTER_KEY:
      g_cmd_func(0);
      break;
    case BACKSPACE_KEY:
      prompt_backspace();
      break;
    case ARROW_LEFT:
      prompt_left();
      break;
    case ARROW_RIGHT:
      prompt_right();
      break;
    default:
      if (g_mode & MODE_SEARCH && ch == (CONTROL | 'S')) {
        g_cmd_func(1);
      }
      else if (g_mode & MODE_SEARCH && ch == (CONTROL | 'R')) {
        g_cmd_func(-1);
      }
      else if (ch == (CONTROL | 'E')) {
        prompt_eol();
      }
      else if (ch == (CONTROL | 'A')) {
        prompt_bol();
      }
      else if (ch == (CONTROL | 'U')) {
        prompt_del_before();
      } else if (ch >= 32 && ch <= 126) {
        prompt_insert(ch);
      }
  }
}
