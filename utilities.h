#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <cstddef>

void create_test_file(const char* path, size_t chunks);
bool split_into_chunks(const char* source_file_path, size_t chunks);

struct in_memory_file_manager {
  in_memory_file_manager(const char* base_file_name, size_t chunks);
  // - add a method for making each array an heap
  // - begin(): creates an heap from all the first element of each array (pop_heap())
  // - end(): dummy iterator
  // ++: extract the next min element
private:
  /*
   * The manager maintains the following structure:
   *
   *           +---------> file.1: 0: [key(1,1), data]
   *           |                   1: [key(1,2), data]
   *           |                   ...
   *           |                   N: [key(1,N), data]
   *           |
   *           +---------> file.2: 0: [key(2,1), data]
   *           |                   1: [key(2,2), data]
   *           |                   ...
   *           |                   N: [key(2,N), data]
   *   file ---+
   *           |
   *           +---------> file.M: 0: [key(M,1), data]
   *                               1: [key(M,2), data]
   *                               ...
   *                               M: [key(M,M), data] where 0<=M<=N
   *
   * The manager knows:
   * - how many files we have
   * - what offset a particular key is inside a given file. This way it is easy to get
   *   always the minimum element for each file with a single read operation.
   * - the elements inside the vector are an heap
   */
  std::unordered<std::string, std::vector<KeyPosition>> _lookup;
};

#endif
