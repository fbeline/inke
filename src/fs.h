#pragma once

#include <stdbool.h>
#include "types.h"

typedef struct {
    char name[255];
    char* data;
    usize len;
    bool is_valid;
} File;

File FileRead(const char* filename);
int FileWrite(const char* filename, const char* buffer);
