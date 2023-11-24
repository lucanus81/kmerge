#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <cstddef>
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

void create_test_file(const char* path, size_t chunks);
size_t split_into_chunks(const char* source_file_path, size_t chunks);

struct in_memory_file_manager {
private:
  struct KeyPosition {
    size_t key;
    size_t offset_in_file;
    size_t file_orig_index;
  };
  
  struct make_min_heap {
    bool operator()(const KeyPosition& kp1, const KeyPosition& kp2) {
      return kp1.key > kp2.key;
    }
  };

  struct sort_by_key {
    bool operator()(const KeyPosition& kp1, const KeyPosition& kp2) {
      return kp1.key < kp2.key;
    }
  };

  struct iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::vector<KeyPosition>;
    using pointer = KeyPosition*;
    using reference = KeyPosition&;
    using const_reference = const KeyPosition&;
    
    iterator(std::unordered_map<std::string, std::vector<KeyPosition>>* source, bool is_end);
    const_reference operator*() const { return _current[0]; }
    pointer operator->() { return &_current[0]; }
    iterator& operator++();
    friend bool operator!= (const iterator& a, const iterator& b) {
       return a._is_end != b._is_end;
    }

  private:
    std::unordered_map<std::string, std::vector<KeyPosition>>*  _source;
    std::vector<KeyPosition> _current;
    bool _is_end;

    void dump();
  };

  std::vector<KeyPosition> parse_file(const char* file_name, size_t file_idx) const;
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
  // TODO: I think I might need a simple vector pair<std::string, std::vector<KeyPosition>>
  std::unordered_map<std::string, std::vector<KeyPosition>> _lookup;
  const char* _base_file_name;
  size_t _max_records_per_file;

public:
  in_memory_file_manager(const char* base_file_name, size_t chunks);
  iterator begin() { return iterator{&_lookup, false}; }
  iterator end() { return iterator{nullptr, true}; }
  bool merge_to(const char* merged_file_path);
};

#endif
