#pragma once

#include "Common.h"
#include "TlsConfig.h"

// Internal class
class TlsContext : noncopyable
{
 public:
  enum Endpoint { kClient, kServer };

  TlsContext(Endpoint type, TlsConfig* config)
    : context_(type == kServer ? tls_server() : tls_client())
  {
    check(tls_configure(context_, config->get()));
  }

  // TlsContext() : context_(NULL) {}

  TlsContext(TlsContext&& rhs)
  {
    swap(rhs);
  }

  ~TlsContext()
  {
    tls_free(context_);
  }

  TlsContext& operator=(TlsContext rhs)  // ???
  {
    swap(rhs);
    return *this;
  }

  void swap(TlsContext& rhs)
  {
    std::swap(context_, rhs.context_);
  }

  void reset(struct tls* ctx) { context_ = ctx; }

  struct tls* get() { return context_; }

  // if there is no error, this will segfault.
  const char* error() { return tls_error(context_); }

  int connect(const char* hostport, const char* servername = nullptr)
  {
    return tls_connect_servername(context_, hostport, nullptr, servername);
  }

  int handshake()
  {
    int ret = -1;
    do {
      ret = tls_handshake(context_);
    } while(ret == TLS_WANT_POLLIN || ret == TLS_WANT_POLLOUT);
    return ret;
  }

  // void accept(
 private:
  void check(int ret)
  {
    if (ret != 0)
    {
      LOG_FATAL << tls_error(context_);
    }
  }

  struct tls* context_ = nullptr;
};
