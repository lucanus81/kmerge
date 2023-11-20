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

static std::ifstream::pos_type file_size(const char* source_file_path) {
  std::ifstream input{source_file_path, std::ios::binary | std::ios::ate};
  return input ? input.tellg() : std::ifstream::pos_type{0};
}

static std::string next_file_name(const std::string& name) {
  static size_t i=0;
  return name + std::to_string(i++);
}

static bool read_write_chunk(std::ifstream* input, char* buffer, size_t chunk_size_in_bytes, const std::string& name) {
  std::string full_name = next_file_name(name);
  std::cout <<"Opening file " <<full_name <<'\n';

  input->read(buffer, chunk_size_in_bytes);
  std::ofstream output_file{ full_name, std::ios::binary };
  return output_file.write(buffer, chunk_size_in_bytes).good();
}

bool split_into_chunks(const char* source_file_path, size_t chunks) {
  std::ifstream::pos_type bytes = file_size(source_file_path);
  if (!bytes || !chunks) {
    std::cerr <<"Cannot open file " <<source_file_path <<": " <<std::strerror(errno) <<"\n";
    return false;
  }

  std::cout <<source_file_path <<" has size " <<bytes <<'\n';
  const std::string source{source_file_path};

  size_t chunk_size_in_bytes = chunks * file::FILESIZE;
  size_t n = bytes / chunk_size_in_bytes;
  std::cout <<"Buffer size = " <<chunk_size_in_bytes <<'\n';
  
  std::ifstream input{source_file_path, std::ios::binary};
  auto buffer = std::make_unique<char[]>(chunk_size_in_bytes);
  for (size_t i=0; i<n; ++i)
    if (!read_write_chunk(&input, buffer.get(), chunk_size_in_bytes, source))
      return false;


  size_t last_chunk_size_in_bytes = static_cast<size_t>(bytes) - chunk_size_in_bytes * n;
  std::cout <<"Last chunk size = " <<last_chunk_size_in_bytes <<'\n';
  if (last_chunk_size_in_bytes && !read_write_chunk(&input, buffer.get(), last_chunk_size_in_bytes, source))
    return false;

  return true;
}
