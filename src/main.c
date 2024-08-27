#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "editor.h"
#include "input.h"
#include "render.h"

static editor_t E = {0};
static render_t R = {0};

void Init(const char* filename) {
  E = editor_init(filename);
  R = render_init(1280, 720, 30);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("usage: olive [file path]\n");
    exit(1);
  }

  Init(argv[1]);

  /* editor_undo_push(0, (vec2_t){0, 0}, "0"); */
  /* editor_undo_push(1, (vec2_t){0, 0}, NULL); */
  /* editor_undo_push(2, (vec2_t){0, 0}, "2"); */

  /* undo_t *u = editor_undo_pop(); */
  /* printf("UNDO %d\n", u->action); */
  /* u = editor_undo_pop(); */
  /* printf("UNDO %d\n", u->action); */
  /* editor_undo_free(u); */
  /* u = editor_undo_pop(); */
  /* printf("UNDO %d\n", u->action); */
  /* editor_undo_free(u); */

  while (!WindowShouldClose() && E.running) {
    input_keyboard_handler(&E);

    render_update_window(&R);
    render_draw(&E, &R);
  }

  CloseWindow();        // Close window and OpenGL context

  return 0;
}
