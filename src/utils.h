#pragma once

#include <stdbool.h>

#define COMPILE_TIME_ASSERT(name, expr) \
    typedef char __assert_##name[(expr) ? 1 : -1]

#define IS_SIGNED(type) ((type)(-1) < 0)
#define IS_UNSIGNED(type) (!IS_SIGNED(type))

#define MIN(x, y) ({ \
    COMPILE_TIME_ASSERT(min_same_type, IS_SIGNED(typeof(x)) == IS_SIGNED(typeof(y)) || IS_UNSIGNED(typeof(x)) == IS_UNSIGNED(typeof(y))); \
    (x) < (y) ? (x) : (y); \
})

#define MAX(x, y) ({ \
    COMPILE_TIME_ASSERT(max_same_type, (IS_SIGNED(typeof(x)) == IS_SIGNED(typeof(y))) || (IS_UNSIGNED(typeof(x)) == IS_UNSIGNED(typeof(y)))); \
    (x) > (y) ? (x) : (y); \
})


bool IsCharBetween(char c, int a, int b);
