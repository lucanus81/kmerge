#ifndef _FILE_H
#define _FILE_H

#include <utility>

// This simple record can be directly read/write to a file
class file {
public:
  static constexpr size_t FILESIZE=1024 * 4;

  constexpr file(size_t key) : _key{key} {
    fill_in_buffer(key);
  }
  constexpr size_t key() const { return _key; }

private:
  static constexpr size_t INITIAL_OFFSET = 10;
  constexpr void fill_in_buffer(size_t key) {
    size_t j=0;
    while (key) {
        size_t q = key / 10, r = key % 10;
        _data[INITIAL_OFFSET + j] = static_cast<char>(r + 48);
        key = q;
        ++j;
    }
    for (size_t l=INITIAL_OFFSET, r=INITIAL_OFFSET+j-1; l<r; ++l, --r)
        std::swap(_data[l], _data[r]);
  }

  size_t _key;
  char _data[FILESIZE - sizeof(size_t)]={'M','Y',' ','K','E','Y',' ','I','S',' '};
};

#endif
