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
  };

  struct iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::vector<KeyPosition>;
    using pointer = std::vector<KeyPosition>*;
    using reference = std::vector<KeyPosition>&;
    using const_reference = const std::vector<KeyPosition>&;
    
    iterator(std::unordered_map<std::string, std::vector<KeyPosition>>* source, size_t current_index) : _source{source}, _current_index{current_index} {
      if (!_source)
        return;

      _current.reserve(_source->size());
      for (auto& [key, v] : *_source) {
        std::make_heap(v.begin(), v.end(), 
            [](const KeyPosition& kp1, const KeyPosition& kp2) { 
              return kp1.key > kp2.key; 
            });
        if (v.begin() != v.end())
          _current.push_back(*v.begin());
      }
      std::sort(_current.begin(), _current.end(), 
        [](const KeyPosition& kp1, const KeyPosition& kp2) { 
          return kp1.key < kp2.key; 
        });
    }

    const_reference operator*() const { return _current; }
    pointer operator->() { return &_current; }
    iterator& operator++() {
      _current.clear();
      for (auto& [key, v] : *_source) {
        std::pop_heap(v.begin(), v.end(),
            [](const KeyPosition& kp1, const KeyPosition& kp2) {
              return kp1.key > kp2.key;
            });
        if (v.begin() != v.end()) {
          v.pop_back();
          if (v.begin() != v.end())
            _current.push_back(*v.begin());
        }
      }
      std::sort(_current.begin(), _current.end(),
        [](const KeyPosition& kp1, const KeyPosition& kp2) {
          return kp1.key < kp2.key;
        });

      ++_current_index; 
      return *this; 
    }
    friend bool operator!= (const iterator& a, const iterator& b) {
       return a._current_index != b._current_index;
    }

  private:
    std::unordered_map<std::string, std::vector<KeyPosition>>*  _source;
    std::vector<KeyPosition> _current;
    size_t _current_index;
  };

  std::vector<KeyPosition> parse_file(const char* file_name) const;
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
  std::unordered_map<std::string, std::vector<KeyPosition>> _lookup;
  size_t _max_records_per_file;

public:
  in_memory_file_manager(const char* base_file_name, size_t chunks);
  iterator begin() { return iterator{&_lookup, 0}; }
  iterator end() { return iterator{nullptr, _max_records_per_file}; }
  // - add a method for making each array an heap
  // - begin(): creates an heap from all the first element of each array (pop_heap())
  // - end(): dummy iterator
  // ++: extract the next min element
};

#endif
