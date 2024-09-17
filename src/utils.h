#pragma once

#include "types.h"

/*
 * Force a compilation error if condition is true, but also produce a
 * result (of value 0 and type int), so the expression can be used
 * e.g. in a structure initializer (or where-ever else comma expressions
 * aren't permitted).
 */
#define BUILD_BUG_ON_ZERO(e) (sizeof(struct { int:-!!(e); }))

#define IS_SIGNED(type) ((type)(-1) < 0)
#define IS_UNSIGNED(type) (!IS_SIGNED(type))

#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))

#define __must_be_array(a)	BUILD_BUG_ON_ZERO(__same_type((a), &(a)[0]))

static inline bool in_range64(u64 val, u64 start, u64 len) {
	return (val - start) < len;
}

static inline bool in_range32(u32 val, u32 start, u32 len) {
	return (val - start) < len;
}

/**
 * IN_RANGE - Determine if a value lies within a range.
 * @val: Value to test.
 * @start: First value in range.
 * @len: Number of values in range.
 */
#define IN_RANGE(val, start, len)					\
	((sizeof(start) | sizeof(len) | sizeof(val)) <= sizeof(u32) ?	\
		in_range32(val, start, len) : in_range64(val, start, len))

/**
 * SWAP - swap values of @a and @b
 * @a: first value
 * @b: second value
 */
#define SWAP(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

/**
 * min - return minimum of two values of the same or compatible types
 * @x: first value
 * @y: second value
 */
#define MIN(x, y) ({ \
    BUILD_BUG_ON_ZERO(!(IS_SIGNED(typeof(x)) == IS_SIGNED(typeof(y)) || IS_UNSIGNED(typeof(x)) == IS_UNSIGNED(typeof(y)))); \
    (x) < (y) ? (x) : (y); \
})

/**
 * max - return maximum of two values of the same or compatible types
 * @x: first value
 * @y: second value
 */
#define MAX(x, y) ({ \
    BUILD_BUG_ON_ZERO(!(IS_SIGNED(typeof(x)) == IS_SIGNED(typeof(y)) || (IS_UNSIGNED(typeof(x)) == IS_UNSIGNED(typeof(y))))); \
    (x) > (y) ? (x) : (y); \
})

/**
 * CLAMP - limit a value between a minimum and maximum range
 * @value: the value to be clamped
 * @min: the minimum allowable value
 * @max: the maximum allowable value
 *
 * This macro returns @value limited to the range between @min and @max.
 * If @value is less than @min, the result is @min. If @value is greater
 * than @max, the result is @max. Otherwise, it returns @value.
 */
#define CLAMP(value, min, max) (MAX(MIN((value), (max)), (min)))

/**
 * ARRAY_SIZE - get the number of elements in array @arr
 * @arr: array to be sized
 */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

void die(const char* msg, ...);
