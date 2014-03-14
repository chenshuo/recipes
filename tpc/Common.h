#pragma once

class noncopyable
{
 protected:
  noncopyable() {}

 private:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;
};

struct copyable
{
};

// For passing C-style string argument to a function.
class StringArg : copyable
{
 public:
  StringArg(const char* str)
    : str_(str)
  { }

  StringArg(const string& str)
    : str_(str.c_str())
  { }

#ifndef MUDUO_STD_STRING
  StringArg(const std::string& str)
    : str_(str.c_str())
  { }
#endif

  const char* c_str() const { return str_; }

 private:
  const char* str_;
};

template<typename To, typename From>
inline To implicit_cast(const From &f)
{
  return f;
}
