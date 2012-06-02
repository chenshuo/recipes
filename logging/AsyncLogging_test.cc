#include "AsyncLoggingQueue.h"
#include "AsyncLoggingDoubleBuffering.h"

int kRollSize = 100*1000*1000;

int main(int argc, char* argv[])
{
  muduo::AsyncLoggingUnboundedQueue  log1("log1", kRollSize);
  muduo::AsyncLoggingBoundedQueue    log2("log2", kRollSize, 1024);
  muduo::AsyncLoggingUnboundedQueueL log3("log3", kRollSize);
  muduo::AsyncLoggingBoundedQueueL   log4("log4", kRollSize, 1024);
  muduo::AsyncLoggingDoubleBuffering log5("log5", kRollSize);
  log1.start();
  log2.start();
  log3.start();
  log4.start();
  log5.start();

  log1.append("hello\n", 6);
  log2.append("world\n", 6);
  log3.append("12345\n", 6);
  log4.append("67890\n", 6);
  log5.append("abcde\n", 6);

  sleep(5);
}
