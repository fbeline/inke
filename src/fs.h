#pragma once

#include <stdbool.h>

typedef struct {
    char* data;
    size_t len;
    bool is_valid;
} File;

File FileRead(const char* filename);
int FileWrite(const char* filename, const char* buffer);
