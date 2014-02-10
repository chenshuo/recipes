#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpStream.h"

// an iterative echo server
int main()
{
  InetAddress listenAddr(3007);
  Acceptor acceptor(listenAddr);
  while (true)
  {
    TcpStreamPtr tcpStream = acceptor.accept();
    char buf[4096];
    int nr = 0;
    while ( (nr = tcpStream->readSome(buf, sizeof(buf))) > 0)
    {
      int nw = tcpStream->sendAll(buf, nr);
      if (nw < nr)
      {
        break;
      }
    }
  }
}
