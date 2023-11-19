#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <cstddef>
void create_test_file(const char* path, size_t chunks);
bool split_into_chunks(const char* source_file_path, size_t chunks);

#endif
