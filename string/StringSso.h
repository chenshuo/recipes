// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (giantchen at gmail dot com)

#ifndef MUDUO_BASE_STRINGSSO_H
#define MUDUO_BASE_STRINGSSO_H

#include <stddef.h>  // size_t
#include <stdint.h>  // uint32_t

namespace muduo
{

/**
 * Short string optimized.
 *
 * similiar data structure of __gnu_cxx::__sso_string
 * sizeof() == 24 on 32-bit
 * sizeof() == 32 on 64-bit
 * max_size() == 1GB, on both 32-bit & 64-bit
 * local buffer == 15 on 32-bit
 * local buffer == 19 on 64-bit
 *
 */
class StringSso // : copyable
                // public boost::less_than_comparable<StringSso>,
                // public boost::less_than_comparable<StringSso, const char*>,
                // public boost::equality_comparable<StringSso>,
                // public boost::equality_comparable<StringSso, const char*>,
                // public boost::addable<StringSso>,
                // public boost::addable<StringSso, const char*>
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

  StringSso(const StringSso&);
  StringSso& operator=(const StringSso&);

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  StringSso(StringSso&&);
  StringSso& operator=(StringSso&&);
#endif

 private:

  const static int kLocalBuffer = sizeof(void*) == 8 ? 19 : 15;

  char* start_;
  uint32_t size_;

  union
  {
    uint32_t capacity_;
    char buf_[kLocalBuffer+1];
  } data_;
};

}
#endif  // MUDUO_BASE_STRINGSSO_H
