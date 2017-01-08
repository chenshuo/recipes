#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpStream.h"

#include <thread>
#include <vector>

#include <assert.h>
#include <string.h>
#include <sys/time.h>

double now()
{
  struct timeval tv = { 0, 0 };
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// an interative request-response server
int main(int argc, char* argv[])
{
  InetAddress listenAddr(3210);
  Acceptor acceptor(listenAddr);
  printf("Accepting... Ctrl-C to exit\n");
  int count = 0;
  bool nodelay = argc > 1 && strcmp(argv[1], "-D") == 0;
  while (true)
  {
    TcpStreamPtr tcpStream = acceptor.accept();
    printf("accepted no. %d client\n", ++count);
    if (nodelay)
      tcpStream->setTcpNoDelay(true);

    while (true)
    {
      int len = 0;
      int nr = tcpStream->receiveAll(&len, sizeof len);
      if (nr <= 0)
        break;
      printf("%f received header %d bytes, len = %d\n", now(), nr, len);
      assert(nr == sizeof len);

      std::vector<char> payload(len);
      nr = tcpStream->receiveAll(payload.data(), len);
      printf("%f received payload %d bytes\n", now(), nr);
      assert(nr == len);
      int nw = tcpStream->sendAll(&len, sizeof len);
      assert(nw == sizeof len);
    }

    printf("no. %d client ended.\n", count);
  }
}
