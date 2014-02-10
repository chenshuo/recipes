#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpStream.h"

int main()
{
  InetAddress listenAddr(3007);
  Acceptor acceptor(listenAddr);
  TcpStreamPtr tcpStream = acceptor.accept();
  // tcpStream->close();
}
