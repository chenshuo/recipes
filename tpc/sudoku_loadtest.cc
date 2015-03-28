#include "../thread/Atomic.h"
#include "../datetime/Timestamp.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpStream.h"

#include <thread>
#include <assert.h>

muduo::AtomicInt64 g_req;
muduo::AtomicInt64 g_resp;

void measure()
{
  muduo::Timestamp start = muduo::Timestamp::now();
  while (true)
  {
    struct timespec ts = { 1, 0 };
    ::nanosleep(&ts, NULL);
    // unfortunately, those two assignments are not atomic
    int64_t req = g_req.getAndSet(0);
    int64_t resp = g_resp.getAndSet(0);
    muduo::Timestamp end = muduo::Timestamp::now();
    double elapsed = timeDifference(end, start);
    start = end;
    if (req)
    {
      printf("%8.0f req/s %8.0f resp/s\n", req / elapsed, resp / elapsed);
    }
  }
}

void sender(TcpStream* stream)
{
  long id = 0;
  while (true)
  {
    char buf[256];
    int n = snprintf(buf, sizeof buf,
                     "%016lx:"
                     "000000010"
                     "400000000"
                     "020000000"
                     "000050407"
                     "008000300"
                     "001090000"
                     "300400200"
                     "050100000"
                     "000806000\r\n",
                     id);
    assert(n == 100);
    int nw = stream->sendAll(buf, n);
    if (nw != n)
      break;
    ++id;
    g_req.increment();
  }
}

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    printf("Usage:\n  %s ip port [-s]\n", argv[0]);
    return 0;
  }

  int port = atoi(argv[2]);
  InetAddress addr(port);
  if (!InetAddress::resolve(argv[1], &addr))
  {
    printf("Unable to resolve %s\n", argv[1]);
    return 0;
  }

  printf("connecting to %s\n", addr.toIpPort().c_str());
  TcpStreamPtr stream(TcpStream::connect(addr));
  if (!stream)
  {
    perror("");
    printf("Unable to connect %s\n", addr.toIpPort().c_str());
    return 0;
  }

  std::thread(measure).detach();

  printf("connected, sending requests\n");
  std::thread sendThr(sender, stream.get());

  if (argc > 3 && std::string(argv[3]) == "-s")
  {
    printf("do not receive responses.\n");
  }
  else
  {
    while (true)
    {
      char buf[256];
      int nr = stream->receiveAll(buf, 100);
      if (nr != 100)
        break;
      assert(buf[16] == ':' && buf[98] == '\r' && buf[99] == '\n');
      g_resp.increment();
    }
  }

  sendThr.join();
}
