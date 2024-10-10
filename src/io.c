#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "editor.h"

static i32 io_file_write(const char* filename, const char* buf) {
  FILE *file;

  if ((file = fopen(filename, "w")) == NULL)
    return 1;

  fprintf(file, "%s", buf);
  fclose(file);

  return 0;
}

i32 io_write_buffer(buffer_t* B) {
  ds_t* ds = editor_rows_to_string(B->editor.lines);

  if (io_file_write(B->filename, ds->buf) != 0) return 1;

  set_status_message("\"%s\" [unix] %dL, %dB written",
                     B->filename, B->editor.nlines, ds->len);

  dsfree(ds);

  return 0;
}
