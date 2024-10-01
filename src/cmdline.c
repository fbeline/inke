#include "cmdline.h"

#include <string.h>

#include "ds.h"
#include "utils.h"

static cmdline_t line = {
  .min_x = 0,
  .x = 0,
  .ds = NULL
};

void cmdline_clean(void) {
  line.ds->buf[0] = '\0';
  line.ds->len = 0;
  line.x = 0;
  line.min_x = 0;
}

void cmdline_cat(const char *str) {
  dscat(line.ds, str);
  line.x += strlen(str);
}

void cmdline_insert(char ch) {
  u32 cposx = line.x == 0 ? 0 : line.x - 1;
  dsichar(line.ds, cposx, ch);
  line.x++;
}

void cmdline_backspace(void) {
  if (line.x == line.min_x)
    return;

  memmove(line.ds->buf + line.x - 2,
          line.ds->buf + line.x - 1,
          line.ds->len - line.x + 1);

  line.x--;
  line.ds->len--;
  line.ds->buf[line.ds->len] = '\0';
}

void cmdline_init(const char *msg) {
  if (line.ds == NULL)
    line.ds = dsempty();

  cmdline_clean();

  dscat(line.ds, msg);

  line.x = line.ds->len + 1;
  line.min_x = line.x;
}

cmdline_t *cmdline(void) {
  return &line;
}

void cmdline_left(void) {
  if (line.x == line.min_x)
    return;

  line.x--;
}

void cmdline_right(void) {
  if (line.x > line.ds->len)
    return;

  line.x++;
}

void cmdline_eol(void) {
  line.x = line.ds->len + 1;
}

void cmdline_bol(void) {
  line.x = line.min_x;
}

void cmdline_del_before(void) {
  memmove(line.ds->buf + line.min_x,
          line.ds->buf + line.x,
          line.ds->len - line.x + 1);
  line.ds->len -= (line.x - line.min_x);
  line.x = line.min_x;
}

