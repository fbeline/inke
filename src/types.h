#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>

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

#define FALSE   0
#define TRUE    1
#define ABORT   2		/* Death, ^G, abort, etc.       */
#define FAILED  3		/* not-quite fatal false return */

typedef struct vec2_s {
  i32 x, y;
} vec2_t;
