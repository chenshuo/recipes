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

  // factory methods
  static Socket createTcp();

 private:
  int sockfd_;
};
