#pragma once

#include <unistd.h>

#include "types.h"

#define IO_FILE_EXISTS(filename) (access(filename, F_OK) == 0)

i32 io_write_buffer(editor_t* E);
