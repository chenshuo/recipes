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

int TcpStream::readAll(void* buf, int len)
{
  // FIXME: EINTR
  return ::recv(sock_.fd(), buf, len, MSG_WAITALL);
}

int TcpStream::readSome(void* buf, int len)
{
  // FIXME: EINTR
  return ::read(sock_.fd(), buf, len);
}

int TcpStream::sendAll(const void* buf, int len)
{
  int written = 0;
  while (written < len)
  {
    int nr = ::write(sock_.fd(), buf, len);
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
  return ::write(sock_.fd(), buf, len);
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
  Socket sock(Socket::createTcp());
  if (sock.connect(serverAddr) == 0)
  {
    // FIXME: do poll(POLLOUT) to check errors
    stream.reset(new TcpStream(std::move(sock)));
  }
  return stream;
}

TcpStreamPtr connect(const InetAddress& serverAddr, const InetAddress& localAddr)
{
  TcpStreamPtr stream;
  Socket sock(Socket::createTcp());
  sock.bindOrDie(localAddr);
  if (sock.connect(serverAddr) == 0)
  {
    // FIXME: do poll(POLLOUT) to check errors
    stream.reset(new TcpStream(std::move(sock)));
  }
  return stream;
}
