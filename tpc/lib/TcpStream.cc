#include "TcpStream.h"
#include "InetAddress.h"

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

bool isSelfConnection(const Socket& sock)
{
  return sock.getLocalAddr() == sock.getPeerAddr();
}

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
    int nw = sock_.write(static_cast<const char*>(buf) + written, len - written);
    if (nw > 0)
    {
      written += nw;
    }
    else if (nw == 0)
    {
      break;
    }
    else if (errno != EINTR)  // nw < 0
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

void TcpStream::setTcpNoDelay(bool on)
{
  sock_.setTcpNoDelay(on);
}

void TcpStream::shutdownWrite()
{
  sock_.shutdownWrite();
}

TcpStreamPtr TcpStream::connect(const InetAddress& serverAddr)
{
  return connectInternal(serverAddr, nullptr);
}

TcpStreamPtr TcpStream::connect(const InetAddress& serverAddr, const InetAddress& localAddr)
{
  return connectInternal(serverAddr, &localAddr);
}

TcpStreamPtr TcpStream::connectInternal(const InetAddress& serverAddr, const InetAddress* localAddr)
{
  TcpStreamPtr stream;
  Socket sock(Socket::createTCP());
  if (localAddr)
  {
    sock.bindOrDie(*localAddr);
  }
  if (sock.connect(serverAddr) == 0 && !isSelfConnection(sock))
  {
    // FIXME: do poll(POLLOUT) to check errors
    stream.reset(new TcpStream(std::move(sock)));
  }
  return stream;
}
