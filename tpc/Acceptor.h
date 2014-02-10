#pragma once
#include "Common.h"
#include "Socket.h"

#include <memory>

class InetAddress;

class TcpStream;
typedef std::unique_ptr<TcpStream> TcpStreamPtr;

class Acceptor : noncopyable
{
 public:
  explicit Acceptor(const InetAddress& listenAddr);

  ~Acceptor() = default;
  Acceptor(Acceptor&&) = default;
  Acceptor& operator=(Acceptor&&) = default;

  // Thread safe
  TcpStreamPtr accept();

 private:
  Socket listenSock_;
};


