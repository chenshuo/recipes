#include "InetAddress.h"
#include "Socket.h"
#include <string.h>

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    printf("Usage:\n  %s hostname port\n", argv[0]);
    return 0;
  }

  int port = atoi(argv[2]);
  InetAddress addr(port);
  const char* hostname = argv[1];
  if (InetAddress::resolve(hostname, &addr))
  {
    Socket sock(Socket::createTCP());
    if (sock.connect(addr) == 0)
    {
      const char* buf = "hello";
      ssize_t nw = ::send(sock.fd(), buf, strlen(buf), MSG_OOB);
      printf("sent %zd bytes\n", nw);
      getchar();
    }
    else
    {
      perror("connect");
    }
  }
}
