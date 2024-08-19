#pragma once

#include "types.h"
#include <stdbool.h>

#define COMPILE_TIME_ASSERT(name, expr) \
    typedef char __assert_##name[(expr) ? 1 : -1]

#define IS_SIGNED(type) ((type)(-1) < 0)
#define IS_UNSIGNED(type) (!IS_SIGNED(type))

static inline bool in_range64(u64 val, u64 start, u64 len) {
	return (val - start) < len;
}

static inline bool in_range32(u32 val, u32 start, u32 len) {
	return (val - start) < len;
}

/**
 * in_range - Determine if a value lies within a range.
 * @val: Value to test.
 * @start: First value in range.
 * @len: Number of values in range.
 */
#define in_range(val, start, len)					\
	((sizeof(start) | sizeof(len) | sizeof(val)) <= sizeof(u32) ?	\
		in_range32(val, start, len) : in_range64(val, start, len))

/**
 * swap - swap values of @a and @b
 * @a: first value
 * @b: second value
 */
#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

/**
 * min - return minimum of two values of the same or compatible types
 * @x: first value
 * @y: second value
 */
#define MIN(x, y) ({ \
    COMPILE_TIME_ASSERT(min_same_type, IS_SIGNED(typeof(x)) == IS_SIGNED(typeof(y)) || IS_UNSIGNED(typeof(x)) == IS_UNSIGNED(typeof(y))); \
    (x) < (y) ? (x) : (y); \
})

/**
 * max - return maximum of two values of the same or compatible types
 * @x: first value
 * @y: second value
 */
#define MAX(x, y) ({ \
    COMPILE_TIME_ASSERT(max_same_type, (IS_SIGNED(typeof(x)) == IS_SIGNED(typeof(y))) || (IS_UNSIGNED(typeof(x)) == IS_UNSIGNED(typeof(y)))); \
    (x) > (y) ? (x) : (y); \
})
