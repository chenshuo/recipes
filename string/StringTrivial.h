#pragma once

#include <utility>
#include <assert.h>
#include <string.h>

namespace trivial
{

// A trivial String class that designed for write-on-paper in an interview
class String
{
 public:
  String()
    : data_(new char[1])
  {
    *data_ = '\0';
  }

  String(const char* str)
    : data_(new char[strlen(str) + 1])
  {
    strcpy(data_, str);
  }

  String(const String& rhs)
    : data_(new char[rhs.size() + 1])
  {
    strcpy(data_, rhs.c_str());
  }
  /* Implement copy-ctor with delegating constructor in C++11
  String(const String& rhs)
    : String(rhs.data_)
  {
  }
  */

  ~String() noexcept
  {
    delete[] data_;
  }

  /* Traditional:
  String& operator=(const String& rhs)
  {
    String tmp(rhs);
    swap(tmp);
    return *this;
  }
  */
  // In C++11, this is unifying assignment operator
  String& operator=(String rhs) // yes, pass-by-value
  {
    // http://en.wikibooks.org/wiki/More_C++_Idioms/Copy-and-swap
    swap(rhs);
    return *this;
  }

  // C++11 move-ctor
  String(String&& rhs) noexcept
    : data_(rhs.data_)
  {
    rhs.data_ = nullptr;
  }

  /* Not needed if we have pass-by-value operator=() above,
   * and it conflits. http://stackoverflow.com/questions/17961719/
  String& operator=(String&& rhs)
  {
    swap(rhs);
    return *this;
  }
  */

  // Accessors

  size_t size() const
  {
    return strlen(data_);
  }

  const char* c_str() const
  {
    return data_;
  }

  void swap(String& rhs)
  {
    std::swap(data_, rhs.data_);
  }

 private:
  char* data_;
};

}

namespace trivial2
{

// string in C++11 with a length member
class String
{
 public:
  String() noexcept
    : data_(nullptr), len_(0)
  { }

  ~String()
  {
    delete[] data_;
  }

  // only read str when len > 0
  String(const char* str, size_t len)
    : data_(len > 0 ? new char[len+1] : nullptr), len_(len)
  {
    if (len_ > 0)
    {
      memcpy(data_, str, len_);
      data_[len_] = '\0';
    }
    else
    {
      assert(data_ == nullptr);
    }
  }

  String(const char* str)
    : String(str, strlen(str))
  { }

  String(const String& rhs)
    : String(rhs.data_, rhs.len_)
  { }

  String(String&& rhs) noexcept
    : data_(rhs.data_), len_(rhs.len_)
  {
    rhs.len_ = 0;
    rhs.data_ = nullptr;
  }

  String& operator=(String rhs)
  {
    swap(rhs);
    return *this;
  }

  void swap(String& rhs) noexcept
  {
    std::swap(len_, rhs.len_);
    std::swap(data_, rhs.data_);
  }

  // const char* data() const { return c_str(); }
  const char* c_str() const noexcept { return data_ ? data_ : kEmpty; }
  size_t size() const noexcept { return len_; }

 private:
  char* data_;
  size_t len_;
  static const char kEmpty[];
};

// const char String::kEmpty[] = "";
}
