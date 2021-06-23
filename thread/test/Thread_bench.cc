#include "../Thread.h"
#include "../../datetime/Timestamp.h"

#include <sys/wait.h>

void threadFunc()
{
  printf("pid=%d, tid=%d\n", ::getpid(), muduo::CurrentThread::tid());
  sleep(10);
}

void threadFunc2()
{
}

void forkBench()
{
  sleep(10);
  muduo::Timestamp start(muduo::Timestamp::now());
  int kProcesses = 10*1000;

  for (int i = 0; i < kProcesses; ++i)
  {
    pid_t child = fork();
    if (child == 0)
    {
      exit(0);
    }
    else
    {
      waitpid(child, NULL, 0);
    }
  }

  double timeUsed = timeDifference(muduo::Timestamp::now(), start);
  printf("process creation time used %.3f us\n", timeUsed*1000000/kProcesses);
  printf("number of created processes %d\n", kProcesses);
}

int main()
{
  printf("pid=%d, tid=%d\n", ::getpid(), muduo::CurrentThread::tid());

  muduo::Thread t1(threadFunc, "Thread 1");
  t1.start();
  t1.join();

  muduo::Timestamp start = muduo::Timestamp::now();
  const int kThreads = 100 * 1000;
  for (int i = 0; i < kThreads; ++i) {
    muduo::Thread t1(threadFunc2);
    t1.start();
    t1.join();
  }
  muduo::Timestamp end = muduo::Timestamp::now();
  double seconds = timeDifference(end, start);
  printf("created and joined %d threads in %.3f ms, %.3fus per thread.\n",
         kThreads, 1e3 * seconds, 1e6 * seconds / kThreads);

  forkBench();
}
