#include "TcpStream.h"

#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>

namespace
{

class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
  }
} initObj;

}

TcpStream::TcpStream(Socket&& sock)
  : sock_(std::move(sock))
{
}

int TcpStream::receiveAll(void* buf, int len)
{
  // FIXME: EINTR
  return ::recv(sock_.fd(), buf, len, MSG_WAITALL);
}

int TcpStream::receiveSome(void* buf, int len)
{
  // FIXME: EINTR
  return sock_.read(buf, len);
}

int TcpStream::sendAll(const void* buf, int len)
{
  int written = 0;
  while (written < len)
  {
    int nr = sock_.write(buf, len);
    if (nr > 0)
    {
      written += nr;
    }
    else if (nr == 0)
    {
      break;
    }
    else if (errno != EINTR)  // nr < 0
    {
      break;
    }
  }
  return written;
}

int TcpStream::sendSome(const void* buf, int len)
{
  // FIXME: EINTR
  return sock_.write(buf, len);
}

void TcpStream::setNoDelay(bool on)
{
  sock_.setTcpNoDelay(on);
}

void TcpStream::shutdownWrite()
{
  sock_.shutdownWrite();
}

TcpStreamPtr TcpStream::connect(const InetAddress& serverAddr)
{
  TcpStreamPtr stream;
  Socket sock(Socket::createTCP());
  if (sock.connect(serverAddr) == 0)
  {
    // FIXME: do poll(POLLOUT) to check errors
    stream.reset(new TcpStream(std::move(sock)));
  }
  return stream;
}

TcpStreamPtr TcpStream::connect(const InetAddress& serverAddr, const InetAddress& localAddr)
{
  TcpStreamPtr stream;
  Socket sock(Socket::createTCP());
  sock.bindOrDie(localAddr);
  if (sock.connect(serverAddr) == 0)
  {
    // FIXME: do poll(POLLOUT) to check errors
    stream.reset(new TcpStream(std::move(sock)));
  }
  return stream;
}
