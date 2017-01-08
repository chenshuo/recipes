#include "thread/Atomic.h"
#include "datetime/Timestamp.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpStream.h"

#include <thread>
#include <assert.h>
#include <unistd.h>

muduo::AtomicInt32 g_req;
muduo::AtomicInt32 g_resp;
muduo::AtomicInt32 g_resp_bad;

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
  printf("sender thread finished: %d requests\n", g_req.get());
}

bool readResponse(TcpStream* stream, bool* bad)
{
  static std::string input;
  while (input.find("\r\n") == std::string::npos)
  {
    char buf[256];
    int nr = stream->receiveSome(buf, 256);
    if (nr <= 0)
      return false;
    input.append(buf, nr);
  }

  size_t crlf = input.find("\r\n");
  assert(crlf != std::string::npos);
  *bad = (crlf + 2 != 100);
  input.erase(0, crlf + 2);
  return true;
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
  int recvDelay = 0;
  if (argc > 3)
  {
    if (std::string(argv[3]) == "-r")
    {
      printf("do not receive responses.\n");
      norecv = true;
    }
    else
    {
      recvDelay = atoi(argv[3]);
      printf("delay receiving by %d seconds.\n", recvDelay);
    }
  }

  std::thread measureThr(measure, total, norecv);

  printf("connected, sending %d requests\n", total);
  std::thread sendThr(sender, stream.get(), total);

  if (!norecv)
  {
    if (recvDelay > 0)
      sleep(recvDelay);
    bool bad = false;
    while (readResponse(stream.get(), &bad))
    {
      if (g_resp.incrementAndGet() >= total)
        break;
      if (bad)
      {
        g_resp_bad.increment();
      }
    }
    printf("all responses received: total=%d bad=%d\n", g_resp.get(), g_resp_bad.get());
  }

  sendThr.join();
  measureThr.join();
  printf("total requests  %d\ntotal responses %d\nbad responses %d\n",
         g_req.get(), g_resp.get(), g_resp_bad.get());
  getchar();
}
