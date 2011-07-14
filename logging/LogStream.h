#ifndef MUDUO_BASE_LOGSTREAM_H
#define MUDUO_BASE_LOGSTREAM_H

#include <stdio.h>
#include <string>
#include <boost/noncopyable.hpp>

namespace muduo
{

using std::string;

namespace detail
{

class FixedBuffer : boost::noncopyable
{
 public:
  FixedBuffer();

  void append(const char*, int len);
  const char* data() const { return data_; }
  int length() const { return cur_ - data_; }

  // for used by GDB
  const char* debugString();
  // for used by unit test
  string testString() const { return string(data_, length()); }
  void reset() { cur_ = data_; }

 private:
  char data_[4000];
  char* cur_;
};

}

class LogStream : boost::noncopyable
{
  typedef LogStream self;
 public:
  typedef detail::FixedBuffer Buffer;
  LogStream();

  self& operator<<(bool);

  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);

  self& operator<<(const void*);

  self& operator<<(float);
  self& operator<<(double);
  // self& operator<<(long double);

  self& operator<<(char);
  // self& operator<<(signed char);
  // self& operator<<(unsigned char);

  self& operator<<(const char*);
  self& operator<<(const string&);

  const Buffer& buffer() const { return buffer_; }
  void resetBuffer() { buffer_.reset(); }

 private:
  void staticCheck();

  template<typename T>
  void formatInteger(T);

  Buffer buffer_;

  static const int kMaxNumericSize = 32;
};

}
#endif  // MUDUO_BASE_FASTOOUTPUTSTREAM_H

