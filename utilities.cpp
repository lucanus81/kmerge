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
std::vector<in_memory_file_manager::KeyPosition> in_memory_file_manager::parse_file(const char* file_name, size_t file_idx) const {
  std::size_t record_count = file_size(file_name) / file::FILESIZE;
  std::cout <<"File " <<file_name <<" has " <<record_count <<" records\n";
  std::vector<KeyPosition> result;
  result.reserve(record_count);

  std::ifstream input{file_name, std::ios::binary};
  file buffer{0};
  size_t offset{0};
  while (input.read(reinterpret_cast<char*>(&buffer), file::FILESIZE)) {
    std::cout <<'<' <<buffer.key() <<',' <<offset <<">\n";
    KeyPosition kp{buffer.key(), offset, file_idx};
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
in_memory_file_manager::in_memory_file_manager(const char* base_file_name, size_t chunks) : _base_file_name{base_file_name}, _max_records_per_file{0} {
  for (size_t i=0; i<chunks; ++i) {
    std::string name{base_file_name};
    name += '.';
    name += std::to_string(i);
    auto [it, _] = _lookup.emplace(std::move(name), parse_file(name.c_str(), i));
    if (it->second.size() > _max_records_per_file)
      _max_records_per_file = it->second.size();
  }
}

void in_memory_file_manager::iterator::dump() {
  std::cout <<"Available data from files:\n";
  for (auto& [key, v] : *_source) {
    std::cout <<key <<": ";
    for (auto i=v.begin(); i!=v.end(); ++i)
      std::cout <<i->key <<' ';
    std::cout <<'\n';
  }
  std::cout <<"Current heap: ";
  for (auto it=_current.begin(); it!=_current.end(); ++it)
    std::cout <<it->key <<' ';
  std::cout <<'\n';
}

void in_memory_file_manager::iterator::push_min_to(
  std::vector<in_memory_file_manager::KeyPosition>* from, 
  std::vector<in_memory_file_manager::KeyPosition>* to) {
  
  std::pop_heap(from->begin(), from->end(), make_min_heap{});
  to->push_back(from->back());
  from->pop_back();
}

/**
 * Iterator construction
 * @param source: the internal lookup table
 * @param current_index: the initial index. Used to remember when to stop iterating
 */
in_memory_file_manager::iterator::iterator(std::unordered_map<std::string, std::vector<in_memory_file_manager::KeyPosition>>* source, bool is_end)
  : _source{source}, _is_end{is_end} {
  if (!_source)
    return;

  _current.reserve(_source->size());
  // 1) Let's get, from each heap, the minimum element and let's place all the current minimums into
  //    a temporary vector
  for (auto& [key, v] : *_source) {
    std::make_heap(v.begin(), v.end(), make_min_heap{});
    if (v.begin() != v.end())
      push_min_to(&v, &_current);
  }

  // 2) Now, we need to make the current vector a real min heap
  std::make_heap(_current.begin(), _current.end(), make_min_heap{});
}

/**
 * Move to the next bunch of records to merge
 */
in_memory_file_manager::iterator& in_memory_file_manager::iterator::operator++() {
  // 0) ++ should return the next min element between all the available data sources. We first remove the current minimum
  std::pop_heap(_current.begin(), _current.end(), make_min_heap{});
  _current.pop_back();

  if (_current.size() == 0) {
    // 1.1) reload the heap
    for (auto& [key, v] : *_source)
      if (v.begin() != v.end())
        push_min_to(&v, &_current);
    if (_current.size() != 0)
      std::make_heap(_current.begin(), _current.end(), make_min_heap{});
  } else {
    size_t current_size_heap = _current.size();
    // 1.2) Let's see if we have another minimum that needs to be inserted into the current heap
    //      from any of the available sources
    for (auto& [key, v] : *_source) {
      if (v.size() && !make_min_heap{}(v[0],_current[0])) 
        push_min_to(&v, &_current);
    }
    // 1.2.1) do we need to re-create the heap?
    if (current_size_heap != _current.size())
      std::make_heap(_current.begin(), _current.end(), make_min_heap{});
  }

  // 2) We might be at the very end and consumed all the elements alredy
  if (_current.size() == 0)
    _is_end = true;
  
  return *this; 
}

/**
 * Method used to merge all the chunks into a single sorted file
 * @param merged_file_path: output file full path
 */
bool in_memory_file_manager::merge_to(const char* merged_file_path) {
  /*
  if (!merged_file_path || _lookup.empty())
    return false;
  
  std::ofstream output_file{merged_file_path, std::ios::binary };
  if (!output_file) {
    std::cerr <<"Error when opening file: " <<std::strerror(errno) <<"\n";
    return false;
  }
  
  std::vector<const std::string*> files;
  files.reserve(_lookup.size());
  for (auto&& [key,_] : _lookup)
    files.push_back(&key);
  std::sort(files.begin(), files.end(), 
    [](const std::string* s1, const std::string* s2) {
      return *s1 < *s2;
    });

  for (auto it = begin(); it != end(); ++it) {
    for (const in_memory_file_manager::KeyPosition& kp : *it) {
      // std::cout <<'{' <<kp.key <<',' <<kp.offset_in_file <<',' <<kp.file_orig_index <<"} -> " <<*files[kp.file_orig_index] <<'\n';
      std::ifstream input{*files[kp.file_orig_index], std::ios::binary};
      file buffer{0};
      input.seekg(kp.offset_in_file).read(reinterpret_cast<char*>(&buffer), file::FILESIZE);
      if (buffer.key() != kp.key) {
        std::cerr <<"Corrupted file\n";
        return false;
      }
      output_file.write(reinterpret_cast<const char*>(&buffer), file::FILESIZE);
      std::cout <<"> " <<buffer.key() <<'\n';
    }
  }*/

  return true;
}
