#pragma once

#include <memory>
#include <string>
#ifdef __cpp_lib_string_view
#include <string_view>
#endif

namespace oop
{

class Digest
{
 public:
  virtual ~Digest() {}
  virtual void update(const void* data, int len) = 0;
#ifdef __cpp_lib_string_view
  void update(std::string_view str)
  {
    update(str.data(), str.length());
  }
#endif
  virtual std::string digest() = 0;
  virtual int length() const = 0;

  enum Type
  {
    SHA1 = 1,
    SHA256 = 2,
    MD5 = 5,
  };
  static std::unique_ptr<Digest> create(Type t);

 protected:
  Digest() {}

 private:
  Digest(const Digest&) = delete;
  void operator=(const Digest&) = delete;
};

}
