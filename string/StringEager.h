// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (giantchen at gmail dot com)

#ifndef MUDUO_BASE_STRINGEAGER_H
#define MUDUO_BASE_STRINGEAGER_H

#include <stddef.h>  // size_t
#include <stdint.h>  // uint32_t

// #include <boost/operators.hpp>

namespace muduo
{

/**
 * Eager copy string.
 *
 * similiar data structure of vector<char>
 * sizeof() == 12 on 32-bit
 * sizeof() == 16 on 64-bit
 * max_size() == 1GB, on both 32-bit & 64-bit
 *
 */
class StringEager // : copyable
                  // public boost::less_than_comparable<StringEager>,
                  // public boost::less_than_comparable<StringEager, const char*>,
                  // public boost::equality_comparable<StringEager>,
                  // public boost::equality_comparable<StringEager, const char*>,
                  // public boost::addable<StringEager>,
                  // public boost::addable<StringEager, const char*>
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

  static const size_type npos = static_cast<size_type>(-1);

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

  //
  // copy control
  //
  StringEager()
    : start_(kEmpty_),
      size_(0),
      capacity_(0)
  {
  }

  StringEager(const StringEager&);
  StringEager& operator=(const StringEager&);

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  StringEager(StringEager&&);
  StringEager& operator=(StringEager&&);
#endif

  ~StringEager()
  {
    if (start_ != kEmpty_)
    {
      delete[] start_;
    }
  }

  //
  // C-style string constructors & assignments
  //
  StringEager(const char* str);
  StringEager(const char* str, size_t);
  StringEager& operator=(const char*);

  //
  // operators
  //

  bool operator<(const StringEager&) const;
  bool operator<(const char*) const;
  bool operator==(const StringEager&) const;
  bool operator==(const char*) const;
  StringEager& operator+=(const StringEager&);
  StringEager& operator+=(const char*);

  //
  // member functions, not conform to the standard
  //

  void push_back(char c);

  void append(const char* str);
  void append(const char* str, size_t);

  void assign(const char* str, size_t);

  // FIXME: more

  void reserve(size_type);
  void clear() throw();
  void swap(StringEager& rhs) throw();

 private:

  struct NoAlloc { };

  void init(const char* str);
  StringEager(char* str, uint32_t, uint32_t, NoAlloc);
  void expandAndAppend(const char* str, size_t len);
  uint32_t nextCapacity(uint32_t) const;
  bool equals(const char* str, size_t len) const;
  bool lessThan(const char* str, size_t len) const;

  char* start_;
  size_type size_;
  size_type capacity_;

  static char kEmpty_[1];
  static const size_type kMinCapacity_ = 15;
};

template<typename Stream>
Stream& operator<<(Stream&, const StringEager&);

}
#endif  // MUDUO_BASE_STRINGEAGER_H
