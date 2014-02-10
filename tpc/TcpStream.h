#pragma once
#include "Common.h"
#include "Socket.h"

#include <memory>

class InetAddress;

class TcpStream;
typedef std::unique_ptr<TcpStream> TcpStreamPtr;

// A blocking TCP stream
class TcpStream : noncopyable
{
 public:
  explicit TcpStream(Socket&& sock);  // sock must be connected
  ~TcpStream() = default;
  TcpStream(TcpStream&&) = default;
  TcpStream& operator=(TcpStream&&) = default;

  static TcpStreamPtr connect(const InetAddress& serverAddr);
  static TcpStreamPtr connect(const InetAddress& serverAddr, const InetAddress& localAddr);

  // NOT thread safe
  int sendAll(const void* buf, int len);  // send len bytes, unless error happens
  int sendSome(const void* buf, int len); // send len or less bytes

  int readAll(const void* buf, int len);  // read len bytes, unless error happens
  int readSome(const void* buf, int len); // read len or less bytes
  
 private:
  Socket sock_;
};

