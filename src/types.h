#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

#include "ds.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;
typedef size_t usize;

#define CONTROL 0x10000000
#define META 0x20000000

#define MODE_INSERT 0x01
#define MODE_SEARCH 0x02
#define MODE_CMD 0x04
#define MODE_CMD_CHAR 0x08
#define MODE_VISUAL 0x10

#define NPATH  256
#define NBINDS 256
#define CLIPBUF (1 << 16)  // 64 KB (2^16 bytes)
#define CMDBUFSIZE 512

#define UNDO_OFF 0
#define UNDO_ON  1

enum keys {
  TAB_KEY = 9,
  ENTER_KEY = 13,
  BACKSPACE_KEY = 127,
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  PAGE_UP,
  PAGE_DOWN,
  HOME_KEY,
  END_KEY,
  DEL_KEY
};

typedef struct cmdline_s {
  u32 min_x;
  u32 x;
  ds_t *ds;
} cmdline_t;

typedef struct line_s {
  struct line_s *next;
  struct line_s *prev;

  ds_t *ds;
} line_t;

typedef struct editor_s {
  unsigned char mode;
  char filename[NPATH];
  bool dirty, new_file;
  usize row_size;
  line_t *lines;
} editor_t;

typedef struct isearch_s {
  line_t *lp;
  usize qlen;
  u32 x;
} isearch_t;

typedef struct mark_s {
  line_t *start_lp;
  line_t *end_lp;
  u32 start_offset;
  u32 end_offset;
} mark_t;

typedef struct cursor_s {
  u32 x, y;
  u16 coloff, rowoff;
  u16 max_col, max_row;
  editor_t *editor;
  line_t *clp;
} cursor_t;

typedef void (*key_func_t)(cursor_t *C);
typedef void (*cmd_func_t)(cursor_t *C, int ch);

typedef struct keytab {
  i32 code;
  key_func_t fp;
} keytab_t;
