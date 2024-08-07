#pragma once

#ifdef _WIN32
#include <io.h>
#define FS_FILE_EXISTS(filename) (_access(filename, 0) == 0)
#else
#include <unistd.h>
#define FS_FILE_EXISTS(filename) (access(filename, F_OK) == 0)
#endif

int FileWrite(const char* filename, const char* buffer);
