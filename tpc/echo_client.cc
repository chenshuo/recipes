#include "InetAddress.h"
#include "TcpStream.h"
#include <unistd.h>

int main(int argc, const char* argv[])
{
  if (argc < 3)
  {
    printf("Usage: %s hostname message_length [scp]\n", argv[0]);
    return 0;
  }

  const int len = atoi(argv[2]);

  InetAddress addr(3007);
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

  printf("connected, sending %d bytes\n", len);

  std::string message(len, 'S');
  int nw = stream->sendAll(message.c_str(), message.size());
  printf("sent %d bytes\n", nw);

  if (argc > 3)
  {
    for (char cmd : std::string(argv[3]))
    {
      if (cmd == 's')  // shutdown
      {
        printf("shutdown write\n");
        stream->shutdownWrite();
      }
      else if (cmd == 'p') // pause
      {
        printf("sleeping for 10 seconds\n");
        ::sleep(10);
        printf("done\n");
      }
      else if (cmd == 'c') // close
      {
        printf("close without reading response\n");
        return 0;
      }
      else
      {
        printf("unknown command '%c'\n", cmd);
      }
    }
  }

  std::vector<char> receive(len);
  int nr = stream->receiveAll(receive.data(), receive.size());
  printf("received %d bytes\n", nr);
  if (nr != nw)
  {
    printf("!!! Incomplete response !!!\n");
  }
}
