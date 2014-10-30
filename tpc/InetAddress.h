#pragma once
#include "Common.h"

#include <string>
#include <vector>

#include <netinet/in.h>

class InetAddress : copyable
{
 public:
  // InetAddress()
  InetAddress(StringArg ip, uint16_t port);
  explicit InetAddress(StringArg ipPort);  // "1.2.3.4:5678"
  explicit InetAddress(uint16_t port, bool loopbackOnly = false);  // for listening

  explicit InetAddress(const struct sockaddr_in& saddr)
    : saddr_(saddr)
  { }

  // default copy/assignment are Okay

  std::string toIp() const;
  std::string toIpPort() const;

  const struct sockaddr_in& getSockAddrInet() const { return saddr_; }
  void setSockAddrInet(const struct sockaddr_in& saddr) { saddr_ = saddr; }

  void setPort(uint16_t port) { saddr_.sin_port = htons(port); }

  uint32_t ipNetEndian() const { return saddr_.sin_addr.s_addr; }
  uint16_t portNetEndian() const { return saddr_.sin_port; }

  uint32_t ipHostEndian() const { return ntohl(saddr_.sin_addr.s_addr); }
  uint16_t portHostEndian() const { return ntohs(saddr_.sin_port); }

  // resolve hostname to IP address, not changing port or sin_family
  // return true on success.
  // thread safe
  static bool resolve(StringArg hostname, InetAddress*);
  static std::vector<InetAddress> resolveAll(StringArg hostname, uint16_t port = 0);

  bool operator==(const InetAddress& rhs) const
  {
    return saddr_.sin_family == rhs.saddr_.sin_family
        && ipNetEndian() == rhs.ipNetEndian()
        && portNetEndian() == rhs.portNetEndian();
  }

 private:
  static bool resolveSlow(const char* hostname, InetAddress*);

  struct sockaddr_in saddr_;
};

