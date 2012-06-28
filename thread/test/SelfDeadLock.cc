#include "../Mutex.h"

class Request
{
 public:
  void process() // __attribute__ ((noinline))
  {
    muduo::MutexLockGuard lock(mutex_);
    print();
  }

  void print() const // __attribute__ ((noinline))
  {
    muduo::MutexLockGuard lock(mutex_);
  }

 private:
  mutable muduo::MutexLock mutex_;
};

int main()
{
  Request req;
  req.process();
}
