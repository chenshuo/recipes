#include "TcpStream.h"

TcpStream::TcpStream(Socket&& sock)
  : sock_(std::move(sock))
{
}
