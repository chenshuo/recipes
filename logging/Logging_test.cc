#include "Logging.h"

long g_total;

void dummyOutput(const char* msg, int len)
{
  g_total += len;
}

void bench()
{
  muduo::Logger::setOutput(dummyOutput);
  muduo::Timestamp start(muduo::Timestamp::now());
  g_total = 0;

  for (int i = 0; i < 1000*1000; ++i)
  {
    LOG_INFO << "Hello";
  }
  muduo::Timestamp end(muduo::Timestamp::now());
  printf("%f seconds, %ld bytes\n", timeDifference(end, start), g_total);
}

int main()
{
  getppid(); // for ltrace and strace
  LOG_TRACE << "trace";
  LOG_DEBUG << "debug";
  LOG_INFO << "Hello";
  LOG_WARN << "World";
  LOG_ERROR << "Error";
  LOG_INFO << sizeof(muduo::Logger);
  LOG_INFO << sizeof(muduo::LogStream);
  LOG_INFO << sizeof(muduo::Fmt);
  LOG_INFO << sizeof(muduo::LogStream::Buffer);

  bench();
}
