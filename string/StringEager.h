// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (giantchen at gmail dot com)

#ifndef MUDUO_BASE_STRINGEAGER_H
#define MUDUO_BASE_STRINGEAGER_H

//#include <stddef.h>
#include <stdint.h>

namespace muduo
{

class StringEager
{
 public:
  typedef char          value_type;
  typedef uint32_t      size_type;
  typedef int32_t       difference_type;
  typedef value_type&   reference;
  typedef const char&   const_reference;
  typedef value_type*   pointer;
  typedef const char*   const_pointer;
  typedef pointer       iterator;
  typedef const_pointer const_iterator;

  static const size_type npos = -1;

  const_pointer c_str() const { return start_; }
  const_pointer data() const  { return start_; }

  iterator begin()              { return start_; }
  const_iterator begin() const  { return start_; }
  const_iterator cbegin() const { return start_; }

  iterator end()              { return start_ + size_; }
  const_iterator end() const  { return start_ + size_; }
  const_iterator cend() const { return start_ + size_; }

  size_type size() const { return size_; }
  size_type length() const { return size_; }
  size_type capacity() const { return capacity_; }

  size_type max_size() const { return 1 << 30; }
  bool empty() const { return size_ == 0; }

 private:

  char* start_;
  uint32_t size_;
  uint32_t capacity_;

  static char kEmpty_[1];

 public:

  StringEager()
    : start_(kEmpty_),
      size_(0),
      capacity_(0)
  {
  }

};

}
#endif  // MUDUO_BASE_STRINGEAGER_H
