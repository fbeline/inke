#pragma once


#include <stdio.h>

#define ASSERT_EQUAL(expected, actual) \
    if ((expected) != (actual)) { \
        printf("Test failed: %s == %s, expected %d but got %d\n", #expected, #actual, (expected), (actual)); \
        return 1; \
    }

#define ASSERT_STRING_EQUAL(expected, actual) \
    if (strcmp((expected), (actual)) != 0) { \
        printf("Test failed: %s == %s\nexpected=\"%s\"\nresult=\"%s\"\n", #expected, #actual, (expected), (actual)); \
        return 1; \
    }

