#include "EventLoop.h"

muduo::EventLoop* g_loop;

void threadFunc()
{
  g_loop->loop();
}

int main()
{
  muduo::EventLoop loop;
  g_loop = &loop;
  muduo::Thread t(threadFunc);
  t.start();
  t.join();
}
