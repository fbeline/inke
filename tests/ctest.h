#pragma once

#include <stdio.h>

#define ASSERT_EQUAL(expected, actual) \
    if ((expected) != (actual)) { \
        printf("Test failed: %s == %s, expected %zu but got %zu\n", #expected, #actual, (expected), (actual)); \
        return 1; \
    }

#define ASSERT_VEC2_EQUAL(expected_x, expected_y, actual) \
    if ((expected_x) != (actual.x) || (expected_y) != (actual.y)) { \
        printf("Test failed: (%s, %s) == %s, expected (%d, %d) but got (%d, %d)\n", \
               #expected_x, #expected_y, #actual, expected_x, expected_y, actual.x, actual.y); \
        return 1; \
    }

#define ASSERT_STRING_EQUAL(expected, actual) \
    if (strcmp((expected), (actual)) != 0) { \
        printf("Test failed: %s == %s\nexpected=\"%s\"\nresult=\"%s\"\n", #expected, #actual, (expected), (actual)); \
        return 1; \
    }



