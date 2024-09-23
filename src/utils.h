#pragma once

#include <stdio.h>
#include "types.h"

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
#define MIN(x, y) ((x) < (y) ? (x) : (y))

/**
 * max - return maximum of two values of the same or compatible types
 * @x: first value
 * @y: second value
 */
#define MAX(x, y) ((x) > (y) ? (x) : (y))

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

#define DIE(msg, ...) \
    do { \
        fprintf(stderr, (msg), ##__VA_ARGS__); \
        exit(1); \
    } while (0)
