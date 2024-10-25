#pragma once

#include <unistd.h>

#include "definitions.h"

#define IO_FILE_EXISTS(filename) (access(filename, F_OK) == 0)

/**
 * io_write_buffer - Writes data from a buffer to disk.
 *
 * @B: A pointer to the buffer_t structure containing the data to be written.
 * @bytes: A pointer to a size_t variable that will store the number of bytes
 *         successfully written on a successful operation.
 *
 * Return: 0 on success, -1 on failure.
 */
i32 io_write_buffer(buffer_t* B, size_t *bytes);
