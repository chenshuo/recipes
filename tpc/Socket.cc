#include "Socket.h"

#include "InetAddress.h"

#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace
{

typedef struct sockaddr SA;

const SA* sockaddr_cast(const struct sockaddr_in* addr)
{
  return static_cast<const SA*>(implicit_cast<const void*>(addr));
}

SA* sockaddr_cast(struct sockaddr_in* addr)
{
  return static_cast<SA*>(implicit_cast<void*>(addr));
}

}

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
  const struct sockaddr_in& saddr = addr.getSockAddrInet();
  int ret = ::bind(sockfd_,  sockaddr_cast(&saddr), static_cast<socklen_t>(sizeof saddr));
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

Socket Socket::createTcp()
{
  int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  assert(sockfd >= 0);
  return Socket(sockfd);
}

