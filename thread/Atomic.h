// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (giantchen at gmail dot com)

#ifndef MUDUO_BASE_ATOMIC_H
#define MUDUO_BASE_ATOMIC_H

#include <boost/noncopyable.hpp>
#include <stdint.h>

namespace muduo
{

namespace detail
{
template<typename T>
class AtomicIntegerT : boost::noncopyable
{
 public:
  AtomicIntegerT()
    : value_(0)
  {
  }

  T get() const
  {
    return value_;
  }

  T getAndAdd(T x)
  {
    return __sync_fetch_and_add(&value_, x);
  }

  T addAndGet(T x)
  {
    return getAndAdd(x) + x;
  }

  T incrementAndGet()
  {
    return addAndGet(1);
  }

  void increment()
  {
    incrementAndGet();
  }

  void decrement()
  {
    getAndAdd(-1);
  }

  T getAndSet(T newValue)
  {
    return __sync_lock_test_and_set(&value_, newValue);
  }

 private:
  volatile T value_;
};
}

typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
typedef detail::AtomicIntegerT<int64_t> AtomicInt64;
}

#endif  // MUDUO_BASE_ATOMIC_H
