// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (giantchen at gmail dot com)

#ifndef MUDUO_BASE_EXCEPTION_H
#define MUDUO_BASE_EXCEPTION_H

#include <exception>
#include <string>

namespace muduo
{

class Exception : public std::exception
{
 public:
  explicit Exception(const char* what);
  virtual ~Exception() throw();
  virtual const char* what() const throw();
  const char* stackTrace() const throw();

 private:
  std::string message_;
  std::string stack_;
};

}

#endif  // MUDUO_BASE_EXCEPTION_H
