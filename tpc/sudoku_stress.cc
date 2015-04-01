#include "../thread/Atomic.h"
#include "../datetime/Timestamp.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpStream.h"

#include <thread>
#include <assert.h>

muduo::AtomicInt32 g_req;
muduo::AtomicInt32 g_resp;

void measure(int total, bool norecv)
{
  muduo::Timestamp start = muduo::Timestamp::now();
  int last_req = 0;
  int last_resp = 0;
  while (true)
  {
    struct timespec ts = { 1, 0 };
    ::nanosleep(&ts, NULL);
    // unfortunately, those two assignments are not atomic
    int req = g_req.get();
    int resp = g_resp.get();

    muduo::Timestamp end = muduo::Timestamp::now();
    double elapsed = timeDifference(end, start);
    start = end;
    int req_delta = req - last_req;
    int resp_delta = resp - last_resp;
    last_req = req;
    last_resp = resp;
    printf("%8.0f req/s %8.0f resp/s\n", req_delta / elapsed, resp_delta / elapsed);
    if (resp >= total || (norecv && req >= total))
      break;
  }
  printf("measure thread finished.\n");
}

void sender(TcpStream* stream, int total)
{
  long id = 0;
  for (int i = 0; i < total; ++i)
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
  printf("sender thread finished.\n");
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    printf("Usage:\n  %s ip [requests] [-r]\n", argv[0]);
    return 0;
  }

  InetAddress addr(9981);
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

  int total = 1e9;
  if (argc > 2)
  {
    total = atoi(argv[2]);
  }

  bool norecv = false;
  if (argc > 3 && std::string(argv[3]) == "-r")
  {
    printf("do not receive responses.\n");
    norecv = true;
  }

  std::thread measureThr(measure, total, norecv);

  printf("connected, sending %d requests\n", total);
  std::thread sendThr(sender, stream.get(), total);

  if (!norecv)
  {
    for (int i = 0; i < total; ++i)
    {
      char buf[256];
      int nr = stream->receiveAll(buf, 100);
      if (nr != 100)
        break;
      assert(buf[16] == ':' && buf[98] == '\r' && buf[99] == '\n');
      g_resp.increment();
    }
    printf("all responses received.\n");
  }

  sendThr.join();
  measureThr.join();
  printf("total requests  %d\ntotal responses %d\n", g_req.get(), g_resp.get());
  getchar();
}
