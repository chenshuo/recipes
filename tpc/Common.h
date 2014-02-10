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

// for passing string parameter to a function
struct StringPiece : copyable
{
 public:
  // FIXME
 private:
  const char* data_;
  int length_;
};

template<typename To, typename From>
inline To implicit_cast(const From &f)
{
  return f;
}
