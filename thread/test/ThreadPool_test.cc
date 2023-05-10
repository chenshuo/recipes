#include "../ThreadPool.h"
#include "../CountDownLatch.h"

#include <boost/bind.hpp>
#include <stdio.h>

void print()
{
  printf("tid=%d\n", muduo::CurrentThread::tid());
}

void printString(const std::string& str)
{
  printf("tid=%d, str=%s\n", muduo::CurrentThread::tid(), str.c_str());
}

int main()
{
  muduo::ThreadPool pool("MainThreadPool");


  pool.start(5);
  pool.run(print);
  pool.run(print);
  for (int i = 0; i < 100; ++i)
  {
    char buf[32];
    snprintf(buf, sizeof buf, "task %d", i);
    pool.run(boost::bind(printString, std::string(buf)));
  }

  muduo::CountDownLatch latch(1);
  pool.run(boost::bind(&muduo::CountDownLatch::countDown, &latch));
  latch.wait();
  pool.stop();

  printf("Testing run_future\n");
  pool.start(5);
  pool.run(print);
  pool.run(print);
  std::vector<std::future<void>> futures;
  for (int i = 0; i < 100; ++i)
  {
    char buf[32];
    snprintf(buf, sizeof buf, "task %d", i);
    futures.emplace_back(std::move(pool.run_future(boost::bind(printString, std::string(buf))))); 
  }
  for (auto& future : futures)
  {
    future.get();
  }
  pool.stop();
}

