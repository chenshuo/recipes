#pragma once

#include "TlsContext.h"

class InetAddress;

class TlsStream;
typedef std::unique_ptr<TlsStream> TlsStreamPtr;

// A blocking TLS stream
class TlsStream : noncopyable
{
 public:
  explicit TlsStream(TlsContext&& context)
    : context_(std::move(context))  // must be established
  {
    LOG_INFO << context_.cipher();
  }

  ~TlsStream() = default;
  TlsStream(TlsStream&&) = default;
  // TlsStream& operator=(TlsStream&&) = default;

  static TlsStreamPtr connect(TlsConfig* config, const char* hostport, const char* servername = nullptr);

  // NOT thread safe
  int receiveAll(void* buf, int len);  // read len bytes, unless error happens
  int receiveSome(void* buf, int len); // read len or less bytes

  int sendAll(const void* buf, int len);  // send len bytes, unless error happens
  int sendSome(const void* buf, int len); // send len or less bytes

 private:
  TlsContext context_;
};

