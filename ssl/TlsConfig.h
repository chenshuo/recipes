#pragma once

#include "Common.h"
#include "logging/Logging.h"

#include <tls.h>

class TlsConfig : noncopyable
{
 public:
  TlsConfig()
    : config_(CHECK_NOTNULL(tls_config_new()))
  {
    if (initialized <= 0)
    {
      LOG_FATAL;
    }
  }

  ~TlsConfig()
  {
    tls_config_free(config_);
  }

  void setCaFile(StringArg caFile)
  {
    check(tls_config_set_ca_file(config_, caFile.c_str()));
  }

  void setCertFile(StringArg certFile)
  {
    check(tls_config_set_cert_file(config_, certFile.c_str()));
  }

  void setKeyFile(StringArg keyFile)
  {
    check(tls_config_set_key_file(config_, keyFile.c_str()));
  }

  struct tls_config* get() { return config_; }

 private:
  void check(int ret)
  {
    if (ret != 0)
    {
      LOG_FATAL << tls_config_error(config_);
    }
  }

  struct tls_config* config_;

  static int initialized;
};
