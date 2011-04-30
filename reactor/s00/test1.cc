#include "EventLoop.h"

int main()
{
  muduo::EventLoop loop;
  loop.loop();
}
