#pragma once
#include "Common.h"

#include <string>

#include <netinet/in.h>

class InetAddress : copyable
{
 public:
  // InetAddress()
  InetAddress(StringPiece hostOrIp, uint16_t port);
  InetAddress(StringPiece hostPort);  // "chenshuo.com:9981"
  explicit InetAddress(uint16_t port, bool loopbackOnly = false);  // for listening

  InetAddress(const struct sockaddr_in& saddr)
    : saddr_(saddr)
  { }

  // default copy/assignment are Okay

  std::string toIpString() const;
  std::string toIpPortString() const;

  const struct sockaddr_in& getSockAddrInet() const { return saddr_; }
  void setSockAddrInet(const struct sockaddr_in& saddr) { saddr_ = saddr; }

  uint32_t ipNetEndian() const { return saddr_.sin_addr.s_addr; }
  uint16_t portNetEndian() const { return saddr_.sin_port; }

  uint32_t ipHostEndian() const { return ntohl(saddr_.sin_addr.s_addr); }
  uint16_t portHostEndian() const { return ntohs(saddr_.sin_port); }

 private:
  struct sockaddr_in saddr_;
};

