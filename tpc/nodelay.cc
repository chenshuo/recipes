#include "InetAddress.h"
#include "TcpStream.h"

#include <string.h>
#include <sys/time.h>
#include <unistd.h>

double now()
{
  struct timeval tv = { 0, 0 };
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    printf("Usage: %s [-b] [-D] [-n num] hostname message_length\n", argv[0]);
    printf("  -b Buffering request before sending.\n"
           "  -D Set TCP_NODELAY.\n"
           "  -n num Send num concurrent requests, default = 1.\n");
    return 0;
  }

  int opt = 0;
  bool buffering = false;
  bool tcpnodelay = false;
  int num = 1;
  while ( (opt = getopt(argc, argv, "bDn:")) != -1)
  {
    switch (opt)
    {
      case 'b':
        buffering = true;
        break;
      case 'D':
        tcpnodelay = true;
        break;
      case 'n':
        num = atoi(optarg);
        break;
      default:
        printf("Unknown option '%c'\n", opt);
        return 0;
    }
  }

  if (optind > argc - 2)
  {
    printf("Please specify hostname and message_length.\n");
    return 0;
  }

  const char* hostname = argv[optind];
  int len = atoi(argv[optind+1]);

  InetAddress addr(3210);
  if (!InetAddress::resolve(hostname, &addr))
  {
    printf("Unable to resolve %s\n", argv[1]);
    return 0;
  }

  printf("connecting to %s\n", addr.toIpPort().c_str());
  TcpStreamPtr stream(TcpStream::connect(addr));
  if (!stream)
  {
    printf("Unable to connect %s\n", addr.toIpPort().c_str());
    perror("");
    return 0;
  }

  if (tcpnodelay)
  {
    stream->setTcpNoDelay(true);
    printf("connected, set TCP_NODELAY\n");
  }
  else
  {
    stream->setTcpNoDelay(false);
    printf("connected\n");
  }


  double start = now();
  for (int n = 0; n < num; ++n)
  {
    printf("Request no. %d, sending %d bytes\n", n, len);
    if (buffering)
    {
      std::vector<char> message(len + sizeof len, 'S');
      memcpy(message.data(), &len, sizeof len);
      int nw = stream->sendAll(message.data(), message.size());
      printf("%.6f sent %d bytes\n", now(), nw);
    }
    else
    {
      stream->sendAll(&len, sizeof len);
      printf("%.6f sent header\n", now());
      usleep(1000); // prevent kernel merging TCP segments
      std::string payload(len, 'S');
      int nw = stream->sendAll(payload.data(), payload.size());
      printf("%.6f sent %d bytes\n", now(), nw);
    }
  }

  printf("Sent all %d requests, receiving responses.\n", num);
  for (int n = 0; n < num; ++n)
  {
    int ack = 0;
    int nr = stream->receiveAll(&ack, sizeof ack);
    printf("%.6f received %d bytes, ack = %d\n", now(), nr, ack);
  }
  printf("total %f seconds\n", now() - start);
}
