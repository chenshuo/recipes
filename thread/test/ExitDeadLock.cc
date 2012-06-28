#include "../Mutex.h"

#include <stdio.h>
#include <stdlib.h>

using namespace muduo;

void someFunctionMayCallExit()
{
  exit(1);
}

class GlobalObject
{
 public:
  void doit()
  {
    MutexLockGuard lock(mutex_);
    someFunctionMayCallExit();
  }

  ~GlobalObject()
  {
    printf("GlobalObject:~GlobalObject\n");
    MutexLockGuard g(mutex_);
    // clean up
    printf("GlobalObject:~GlobalObject cleanning\n");
  }

 private:
  MutexLock mutex_;
};

GlobalObject g_obj;

int main()
{
  g_obj.doit();
}
