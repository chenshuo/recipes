#include "Socket.h"
#include "InetAddress.h"

#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

Socket::Socket(int sockfd)
  : sockfd_(sockfd)
{
  assert(sockfd_ >= 0);
}

Socket::~Socket()
{
  if (sockfd_ >= 0)
  {
    int ret = ::close(sockfd_);
    assert(ret == 0); (void)ret;
  }
}

void Socket::bindOrDie(const InetAddress& addr)
{
  int ret = ::bind(sockfd_, addr.get_sockaddr(), addr.length());
  if (ret)
  {
    perror("Socket::bindOrDie");
    abort();
  }
}

void Socket::listenOrDie()
{
  int ret = ::listen(sockfd_, SOMAXCONN);
  if (ret)
  {
    perror("Socket::listen");
    abort();
  }
}

int Socket::connect(const InetAddress& addr)
{
  return ::connect(sockfd_, addr.get_sockaddr(), addr.length());
}

void Socket::shutdownWrite()
{
  if (::shutdown(sockfd_, SHUT_WR) < 0)
  {
    perror("Socket::shutdownWrite");
  }
}

void Socket::setReuseAddr(bool on)
{
  int optval = on ? 1 : 0;
  if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                   &optval, sizeof optval) < 0)
  {
    perror("Socket::setReuseAddr");
  }
}

void Socket::setTcpNoDelay(bool on)
{
  int optval = on ? 1 : 0;
  if (::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof optval)) < 0)
  {
    perror("Socket::setTcpNoDelay");
  }
}

InetAddress Socket::getLocalAddr() const
{
  struct sockaddr localaddr;
  socklen_t addrlen = sizeof localaddr;
  if (::getsockname(sockfd_, &localaddr, &addrlen) < 0)
  {
    perror("Socket::getLocalAddr");
  }
  return InetAddress(localaddr);
}

InetAddress Socket::getPeerAddr() const
{
  struct sockaddr peeraddr;
  socklen_t addrlen = sizeof peeraddr;
  if (::getpeername(sockfd_, &peeraddr, &addrlen) < 0)
  {
    perror("Socket::getPeerAddr");
  }
  return InetAddress(peeraddr);
}

int Socket::recv(void* buf, int len)
{
  return TEMP_FAILURE_RETRY(::recv(sockfd_, buf, len, 0));
}

int Socket::send(const void* buf, int len)
{
  return TEMP_FAILURE_RETRY(::send(sockfd_, buf, len, 0));
}

Socket Socket::createTCP(sa_family_t family)
{
  int sockfd = ::socket(family, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
  assert(sockfd >= 0);
  return Socket(sockfd);
}

Socket Socket::createUDP(sa_family_t family)
{
  int sockfd = ::socket(family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
  assert(sockfd >= 0);
  return Socket(sockfd);
}

