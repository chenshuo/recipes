#include "StringEager.h"

#include <assert.h>
#include <string.h>
#include <algorithm>

char muduo::StringEager::kEmpty_[] = "";
const uint32_t muduo::StringEager::kMinCapacity_;

using namespace muduo;

StringEager::StringEager(const StringEager& rhs)
  : start_(kEmpty_),
    size_(rhs.size_),
    capacity_(0)
{
  init(rhs.start_);
}

StringEager& StringEager::operator=(const StringEager& rhs)
{
  // shrink is not good
  // StringEager tmp(rhs);
  // swap(tmp);

  // if (this != &rhs)
  assign(rhs.start_, rhs.size_);
  return *this;
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
StringEager::StringEager(StringEager&& rhs)
  : start_(rhs.start_),
    size_(rhs.size_),
    capacity_(rhs.capacity_)
{
  rhs.start_ = NULL;
}

StringEager& StringEager::operator=(StringEager&& rhs)
{
  swap(rhs);
  return *this;
}
#endif

StringEager::StringEager(const char* str)
  : start_(kEmpty_),
    size_(::strlen(str)),
    capacity_(0)
{
  init(str);
}

StringEager::StringEager(const char* str, size_t len)
  : start_(kEmpty_),
    size_(len),
    capacity_(0)
{
  init(str);
}

StringEager& StringEager::operator=(const char* str)
{
  assign(str, ::strlen(str));
  return *this;
}

StringEager::StringEager(char* str, uint32_t sz, uint32_t cap, NoAlloc)
  : start_(str),
    size_(sz),
    capacity_(cap)
{
}

void StringEager::init(const char* str)
{
  if (size_ > 0)
  {
    capacity_ = std::max(size_, kMinCapacity_);
    start_ = new char[capacity_+1];
    ::memcpy(start_, str, size_+1);
  }
}

bool StringEager::operator<(const StringEager& rhs) const
{
  return lessThan(rhs.start_, rhs.size_);
}

bool StringEager::operator<(const char* str) const
{
  return lessThan(str, ::strlen(str));
}

bool StringEager::operator==(const StringEager& rhs) const
{
  return this == &rhs || equals(rhs.start_, rhs.size_);
}

bool StringEager::operator==(const char* str) const
{
  return equals(str, ::strlen(str));
}

void StringEager::reserve(size_type len)
{
  if (len > capacity_)
  {
    const uint32_t newCap = nextCapacity(len);
    char* newStart = new char[newCap+1];
    ::memcpy(newStart, start_, size_);
    newStart[size_] = '\0';
    StringEager tmp(newStart, size_, newCap, NoAlloc());
    swap(tmp);
  }
}

void StringEager::clear() throw()
{
  size_ = 0;
  start_[size_] = '\0';
}

void StringEager::swap(StringEager& rhs) throw()
{
  std::swap(start_, rhs.start_);
  std::swap(size_, rhs.size_);
  std::swap(capacity_, rhs.capacity_);
}

void StringEager::assign(const char* str, size_t len)
{
  if (capacity_ >= len)
  {
    ::memmove(start_, str, len);
    size_ = len;
    start_[size_] = '\0';
  }
  else
  {
    clear();
    expandAndAppend(str, len);
  }
}

void StringEager::push_back(char c)
{
  if (capacity_ >= size_+1)
  {
    start_[size_] = c;
    start_[size_+1] = '\0';
    ++size_;
  }
  else
  {
    expandAndAppend(&c, 1);
  }
}

void StringEager::append(const char* str)
{
  append(str, ::strlen(str));
}

void StringEager::append(const char* str, size_t len)
{
  if (capacity_ >= size_ + len)
  {
    ::memcpy(start_+size_, str, len);
    size_ += len;
    start_[size_] = '\0';
  }
  else
  {
    expandAndAppend(str, len);
  }
}

void StringEager::expandAndAppend(const char* str, size_t len)
{
  const uint32_t newSize = size_+len;
  const uint32_t newCap = nextCapacity(newSize);
  char* newStart = new char[newCap+1];
  ::memcpy(newStart, start_, size_);
  ::memcpy(newStart+size_, str, len);
  newStart[newSize] = '\0';
  StringEager tmp(newStart, newSize, newCap, NoAlloc());
  swap(tmp);
}

uint32_t StringEager::nextCapacity(uint32_t newSize) const
{
  uint32_t newCap = std::max(2*capacity_, newSize);
  return std::max(newCap, kMinCapacity_);
}

bool StringEager::equals(const char* str, size_t len) const
{
  return size_ == len && ::memcmp(start_, str, size_) == 0;
}

bool StringEager::lessThan(const char* str, size_t strsize) const
{
  // std::lexicographical_compare
  uint32_t size = std::min(size_, static_cast<size_type>(strsize));
  int result = ::memcmp(start_, str, size);
  return result == 0 ? (size_ < strsize) : (result < 0);
}
