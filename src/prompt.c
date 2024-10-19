#include "prompt.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "ds.h"
#include "globals.h"
#include "isearch.h"

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
      if (g_flags & MSEARCH)
        isearch(0);
      else
        g_cmd_func();
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
    case TAB_KEY:
      if (line.x == line.ds->len && g_cmd_complete_func != NULL) {
        g_cmd_complete_func();
      } else {
        for (u8 i = 0; i < TAB_STOP; i++) {
          dsichar(line.ds, line.x++, ' ');
        }
      }
      break;
    default:
      if (g_flags & MSEARCH && ch == (CONTROL | 'S')) {
        isearch(1);
      }
      else if (g_flags & MSEARCH && ch == (CONTROL | 'R')) {
        isearch(-1);
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

static i32 starts_with(const char *s, const char *prefix) {
  usize prefixlen = strlen(prefix);
  usize slen = strlen(s);
  if (prefixlen > slen) return 0;

  return strncmp(s, prefix, prefixlen) == 0;
}

void prompt_fs_completion(void) {
  DIR *dir;
  struct dirent *entry;
  const char *last_slash;
  char path[NPATH];
  char val[NPATH];

  const char *text = &line.ds->buf[line.min_x];
  if ((last_slash = strrchr(text, '/')) == NULL) {
    dscat(line.ds, "/");
    line.x++;
    return;
  }
  usize dirlen = last_slash - text + 1;
  strncpy(path, text , dirlen);
  strcpy(val, last_slash + 1);
  path[dirlen] = '\0';
  usize valuelen = strlen(val);

  if (valuelen == 0 || (dir = opendir(path)) == NULL) return;

  while ((entry = readdir(dir)) != NULL) {
    if (starts_with(entry->d_name, val) == 0) continue;

    dscat(line.ds, entry->d_name + valuelen);
    line.x += strlen(entry->d_name) - valuelen;

    if (entry->d_type == DT_DIR) {
      dscat(line.ds, "/");
      line.x++;
    }

    break;
  }

  closedir(dir);
}
