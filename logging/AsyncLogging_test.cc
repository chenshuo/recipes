#include <stdio.h>
#include "datetime/Timestamp.h"

#include "AsyncLoggingQueue.h"
#include "AsyncLoggingDoubleBuffering.h"

#include "Logging.h"

int kRollSize = 20*1000*1000;

void* g_asyncLog = NULL;

template<typename ASYNC>
void asyncOutput(const char* msg, int len)
{
  ASYNC* log = static_cast<ASYNC*>(g_asyncLog);
  log->append(msg, len);
}

template<typename ASYNC>
void bench(ASYNC* log)
{
  g_asyncLog = log;
  log->start();
  muduo::Logger::setOutput(asyncOutput<ASYNC>);

  int cnt = 0;
  const int kBatch = 1000;
  const bool kLongLog = true;
  muduo::string empty = " ";
  muduo::string longStr(3000, 'X');
  longStr += " ";

  for (int t = 0; t < 10; ++t)
  {
    muduo::Timestamp start = muduo::Timestamp::now();
    for (int i = 0; i < kBatch; ++i)
    {
      LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
               << (kLongLog ? longStr : empty)
               << cnt;
      ++cnt;
    }
    muduo::Timestamp end = muduo::Timestamp::now();
    printf("%f\n", timeDifference(end, start)*1000000/kBatch);
    struct timespec ts = { 1, 0 };
    nanosleep(&ts, NULL);
  }
}

int main(int argc, char* argv[])
{
  muduo::AsyncLoggingUnboundedQueue  log1("log1", kRollSize);
  muduo::AsyncLoggingBoundedQueue    log2("log2", kRollSize, 1024);
  muduo::AsyncLoggingUnboundedQueueL log3("log3", kRollSize);
  muduo::AsyncLoggingBoundedQueueL   log4("log4", kRollSize, 1024);
  muduo::AsyncLoggingDoubleBuffering log5("log5", kRollSize);
  int which = argc > 1 ? atoi(argv[1]) : 1;

  printf("pid = %d\n", getpid());

  switch (which)
  {
    case 1:
      bench(&log1);
      break;
    case 2:
      bench(&log2);
      break;
    case 3:
      bench(&log3);
      break;
    case 4:
      bench(&log4);
      break;
    case 5:
      bench(&log5);
      break;
  }
}
