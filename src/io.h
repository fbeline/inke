#pragma once

#include "editor.h"

#ifdef _WIN32
#include <io.h>
#define IO_FILE_EXISTS(filename) (_access(filename, 0) == 0)
#else
#include <unistd.h>
#define IO_FILE_EXISTS(filename) (access(filename, F_OK) == 0)
#endif

int io_write_buffer(editor_t* E);
