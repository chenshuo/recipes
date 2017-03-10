#include "thread/Atomic.h"
#include "datetime/Timestamp.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpStream.h"

#include <string.h>

#include <thread>

muduo::AtomicInt64 g_bytes;

void measure()
{
  muduo::Timestamp start = muduo::Timestamp::now();
  while (true)
  {
    struct timespec ts = { 1, 0 };
    ::nanosleep(&ts, NULL);
    // unfortunately, those two assignments are not atomic
    int64_t bytes = g_bytes.getAndSet(0);
    muduo::Timestamp end = muduo::Timestamp::now();
    double elapsed = timeDifference(end, start);
    start = end;
    if (bytes)
    {
      printf("%.3f MiB/s\n", bytes / (1024.0 * 1024) / elapsed);
    }
  }
}

void discard(TcpStreamPtr stream)
{
  char buf[65536];
  while (true)
  {
    int nr = stream->receiveSome(buf, sizeof buf);
    if (nr <= 0)
      break;
    g_bytes.add(nr);
  }
}

// a thread-per-connection current discard server and client
int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    printf("Usage:\n  %s hostname port\n  %s -l port\n", argv[0], argv[0]);
    return 0;
  }

  std::thread(measure).detach();

  int port = atoi(argv[2]);
  if (strcmp(argv[1], "-l") == 0)
  {
    Acceptor acceptor((InetAddress(port)));
    printf("Accepting... Ctrl-C to exit\n");
    int count = 0;
    while (true)
    {
      TcpStreamPtr tcpStream = acceptor.accept();
      printf("accepted no. %d client\n", ++count);

      std::thread thr(discard, std::move(tcpStream));
      thr.detach();
    }
  }
  else
  {
    InetAddress addr(port);
    const char* hostname = argv[1];
    if (InetAddress::resolve(hostname, &addr))
    {
      TcpStreamPtr stream(TcpStream::connect(addr));
      if (stream)
      {
        discard(std::move(stream));
      }
      else
      {
        printf("Unable to connect %s\n", addr.toIpPort().c_str());
        perror("");
      }
    }
    else
    {
      printf("Unable to resolve %s\n", hostname);
    }
  }
}
