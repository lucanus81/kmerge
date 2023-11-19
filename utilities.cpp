#include "utilities.h"
#include "file.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <limits>
#include <cstring>
#include <cerrno>

void create_test_file(const char* path, size_t chunks) {
  std::random_device rd;
  std::mt19937 gen{rd()};
  std::uniform_int_distribution<size_t> d{1, std::numeric_limits<size_t>::max()};
  
  // TODO: for large files that don't fit in memory this code will throw OOM, but this
  //       is only for internal tests, soit should be fine.
  const size_t BUFFER_SIZE{chunks * file::FILESIZE};
  auto buffer = std::make_unique<char[]>(BUFFER_SIZE);
  char* pos = buffer.get();
  for (size_t i=0; i<chunks; ++i) {
    file* f = new (pos) file{d(gen)};
    pos += file::FILESIZE;
  }

  std::ofstream output_file{path, std::ios::binary};
  if (!output_file)
    std::cerr <<"Error when opening file: " <<std::strerror(errno) <<"\n";
  else
    output_file.write(buffer.get(), BUFFER_SIZE);
}

bool split_into_chunks(const char* source_file_path, size_t chunks) {
  std::cout <<"done\n";
  return true;
}
