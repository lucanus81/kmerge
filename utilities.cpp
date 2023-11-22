#include "utilities.h"
#include "file.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <limits>
#include <cstring>
#include <cerrno>

/**
 * Utility function used to create some test data
 */
void create_test_file(const char* path, size_t chunks) {
  std::random_device rd;
  std::mt19937 gen{rd()};
  //std::uniform_int_distribution<size_t> d{1, std::numeric_limits<size_t>::max()};
  std::uniform_int_distribution<size_t> d{1, 100};
  
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
  return name + "." + std::to_string(i++);
}

static bool read_write_chunk(std::ifstream* input, char* buffer, size_t chunk_size_in_bytes, const std::string& name) {
  std::string full_name = next_file_name(name);
  std::cout <<"Opening file " <<full_name <<'\n';

  input->read(buffer, chunk_size_in_bytes);
  std::ofstream output_file{ full_name, std::ios::binary };
  return output_file.write(buffer, chunk_size_in_bytes).good();
}

/**
 * Function used to split a huge file in smaller files.
 * @param: source_file_path, the base file name full path
 * @param: chunks, the maximum number of file::FILESIZE record a splitted file will contain
 * @return: the total number of file created. Each file will have the following naming
 *          convention: <source_file_path>.1, <source_file_path>.2, ...
 */
size_t split_into_chunks(const char* source_file_path, size_t chunks) {
  std::ifstream::pos_type bytes = file_size(source_file_path);
  if (!bytes || !chunks) {
    std::cerr <<"Cannot open file " <<source_file_path <<": " <<std::strerror(errno) <<"\n";
    return 0;
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
      return 0;


  size_t last_chunk_size_in_bytes = static_cast<size_t>(bytes) - chunk_size_in_bytes * n;
  std::cout <<"Last chunk size = " <<last_chunk_size_in_bytes <<'\n';
  if (last_chunk_size_in_bytes && !read_write_chunk(&input, buffer.get(), last_chunk_size_in_bytes, source))
    return 0;

  return last_chunk_size_in_bytes ? n+1 : n;
}

/**
 * Utility function used to read, from each file, the key and the offset inside the file for a fast fseek() operation.
 * @param file_name: the file name we're trying to load
 * @return a vector of KeyPosition structures
 */
std::vector<in_memory_file_manager::KeyPosition> in_memory_file_manager::parse_file(const char* file_name) const {
  std::size_t record_count = file_size(file_name) / file::FILESIZE;
  std::cout <<"File " <<file_name <<" has " <<record_count <<" records\n";
  std::vector<KeyPosition> result;
  result.reserve(record_count);

  std::ifstream input{file_name, std::ios::binary};
  file buffer{0};
  size_t offset{0};
  while (input.read(reinterpret_cast<char*>(&buffer), file::FILESIZE)) {
    std::cout <<'<' <<buffer.key() <<',' <<offset <<">\n";
    KeyPosition kp{buffer.key(), offset};
    result.emplace_back(std::move(kp));
    offset += file::FILESIZE;
  }

  return result;
}

/**
 * Constructor. See header file for nor details on the whole structure.
 * @param base_file_name: the base name for all the splitted files
 * @param chunks: how many files we have to load
 */
in_memory_file_manager::in_memory_file_manager(const char* base_file_name, size_t chunks) : _max_records_per_file{0} {
  for (size_t i=0; i<chunks; ++i) {
    std::string name{base_file_name};
    name += '.';
    name += std::to_string(i);
    auto [it, _] = _lookup.emplace(std::move(name), parse_file(name.c_str()));
    if (it->second.size() > _max_records_per_file)
      _max_records_per_file = it->second.size();
  }
}
