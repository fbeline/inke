#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mode.h"

static int io_file_write(const char* filename, const char* buf) {
  int len;
  FILE *file = fopen(filename, "w");

  if (file == NULL) {
    printf("Error writing to file %s\n", filename);
    return 1;
  }
  fprintf(file, "%s", buf);
  fclose(file);

  return 0;
}

int io_write_buffer(editor_t* E) {
  line_t* lp = editor_rows_to_string(E->rows, E->row_size);

  if (io_file_write(E->filename, lp->text) != 0) return 1;

  E->dirty = false;
  mode_cmd_clean();
  mode_set_message("\"%s\" [unix] %dL, %dB written", E->filename, E->row_size, lp->size);

  line_free(lp);

  return 0;
}
