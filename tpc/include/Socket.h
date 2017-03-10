#pragma once
#include "Common.h"

#include <utility>  // swap

class InetAddress;

// RAII handle for socket fd
class Socket : noncopyable
{
 public:
  explicit Socket(int sockfd);
  ~Socket();  // close sockfd_

  Socket(Socket&& rhs)
    : Socket(rhs.sockfd_)
  {
    rhs.sockfd_ = -1;
  }

  Socket& operator=(Socket&& rhs)
  {
    swap(rhs);
    return *this;
  }

  void swap(Socket& rhs)
  {
    std::swap(sockfd_, rhs.sockfd_);
  }

  int fd() { return sockfd_; }

  // Sockets API
  void bindOrDie(const InetAddress& addr);
  void listenOrDie();
  // return 0 on success
  int connect(const InetAddress& addr);
  void shutdownWrite();

  void setReuseAddr(bool on);
  void setTcpNoDelay(bool on);

  InetAddress getLocalAddr() const;
  InetAddress getPeerAddr() const;

  int read(void* buf, int len);
  int write(const void* buf, int len);

  // factory methods
  static Socket createTCP();
  static Socket createUDP();

 private:
  int sockfd_;
};
