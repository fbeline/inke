#pragma once

#include <unistd.h>

#include "definitions.h"

#define IO_FILE_EXISTS(filename) (access(filename, F_OK) == 0)

i32 io_write_buffer(buffer_t* B);
