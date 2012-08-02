#include "../Mutex.h"
using muduo::MutexLock;
using muduo::MutexLockGuard;

// A thread-safe counter
class Counter : boost::noncopyable
{
  // copy-ctor and assignment should be private by default for a class.
 public:
  Counter() : value_(0) {}
  Counter& operator=(const Counter& rhs);

  int64_t value() const;
  int64_t getAndIncrease();

  friend void swap(Counter& a, Counter& b);

 private:
  mutable MutexLock mutex_;
  int64_t value_;
};

int64_t Counter::value() const
{
  MutexLockGuard lock(mutex_);
  return value_;
}

int64_t Counter::getAndIncrease()
{
  MutexLockGuard lock(mutex_);
  int64_t ret = value_++;
  return ret;
}

void swap(Counter& a, Counter& b)
{
  MutexLockGuard aLock(a.mutex_);  // potential dead lock
  MutexLockGuard bLock(b.mutex_);
  int64_t value = a.value_;
  a.value_ = b.value_;
  b.value_ = value;
}

Counter& Counter::operator=(const Counter& rhs)
{
  if (this == &rhs)
    return *this;

  MutexLockGuard myLock(mutex_);  // potential dead lock
  MutexLockGuard itsLock(rhs.mutex_);
  value_ = rhs.value_;
  return *this;
}

int main()
{
  Counter c;
  c.getAndIncrease();
}
