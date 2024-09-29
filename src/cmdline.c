#include "cmdline.h"

#include <string.h>
#include "ds.h"

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
  dsichar(line.ds, line.x, ch);
  line.x++;
}

void cmdline_backspace(void) {
  if (line.x == line.min_x)
    return;

  memmove(&line.ds->buf[line.x],
          &line.ds->buf[line.x+1],
          line.ds->len - line.x + 1);

  line.ds->len--;
  line.x--;
  line.ds->buf[line.ds->len] = '\0';
}

void cmdline_init(const char *msg) {
  if (line.ds == NULL)
    line.ds = dsempty();

  cmdline_clean();

  dscat(line.ds, msg);

  usize msglen = strlen(msg);
  line.x = msglen + 1;
  line.min_x = line.x;
}

cmdline_t *cmdline(void) {
  return &line;
}
