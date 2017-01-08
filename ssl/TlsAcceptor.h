#pragma once
#include "Common.h"
#include "Socket.h"

#include "TlsContext.h"

#include <memory>

class InetAddress;

class TlsStream;
typedef std::unique_ptr<TlsStream> TlsStreamPtr;

class TlsAcceptor : noncopyable
{
 public:
  TlsAcceptor(TlsConfig* config, const InetAddress& listenAddr);

  ~TlsAcceptor() = default;
  TlsAcceptor(TlsAcceptor&&) = default;
  TlsAcceptor& operator=(TlsAcceptor&&) = default;

  // thread safe
  TlsStreamPtr accept();

 private:
  TlsContext context_;
  Socket listenSock_;
};


