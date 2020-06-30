#include "InetAddress.h"

#include <memory>

#include <assert.h>
#include <arpa/inet.h>
#include <netdb.h>

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "InetAddress is same size as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

InetAddress::InetAddress(StringArg ip, uint16_t port)
{
  setPort(port);

  int result = 0;
  if (strchr(ip.c_str(), ':') == NULL)
  {
    result = ::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
  }
  else
  {
    result = ::inet_pton(AF_INET6, ip.c_str(), &addr6_.sin6_addr);
  }

  assert(result == 1 && "Invalid IP format");
}

InetAddress::InetAddress(uint16_t port, bool ipv6)
{
  static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset 0");
  static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
  bool loopbackOnly = false;
  if (ipv6)
  {
    memZero(&addr6_, sizeof addr6_);
    addr6_.sin6_family = AF_INET6;
    in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
    addr6_.sin6_addr = ip;
    addr6_.sin6_port = htons(port);
  }
  else
  {
    memZero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
    addr_.sin_addr.s_addr = htonl(ip);
    addr_.sin_port = htons(port);
  }
}

InetAddress::InetAddress(const struct sockaddr& saddr)
{
  if (saddr.sa_family == AF_INET)
  {
    memcpy(&addr_, &saddr, sizeof addr_);
  }
  else if (saddr.sa_family == AF_INET6)
  {
    memcpy(&addr6_, &saddr, sizeof addr6_);
  }
  else
  {
    assert(false);
  }
}

std::string InetAddress::toIp() const
{
  char buf[64] = "";
  static_assert(sizeof buf >= INET_ADDRSTRLEN);
  static_assert(sizeof buf >= INET6_ADDRSTRLEN);

  if (family() == AF_INET)
  {
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(sizeof buf));
  }
  else if (family() == AF_INET6)
  {
    ::inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, static_cast<socklen_t>(sizeof buf));
  }

  return buf;
}

std::string InetAddress::toIpPort() const
{
  char buf[32] = "";
  snprintf(buf, sizeof buf, ":%u", port());
  return toIp() + buf;
}

bool InetAddress::operator==(const InetAddress& rhs) const
{
  if (family() == rhs.family())
  {
    if (family() == AF_INET)
    {
      return addr_.sin_port == rhs.addr_.sin_port &&
          addr_.sin_addr.s_addr == rhs.addr_.sin_addr.s_addr;
    }
    else if (family() == AF_INET6)
    {
      return addr6_.sin6_port == rhs.addr6_.sin6_port &&
          memcmp(&addr6_.sin6_addr, &rhs.addr6_.sin6_addr, sizeof addr6_.sin6_addr) == 0;
    }
  }
  return false;
}

// static
bool InetAddress::resolve(StringArg hostname, uint16_t port, InetAddress* out)
{
  assert(out);
  std::vector<InetAddress> addrs = resolveAll(hostname, port);

  if (addrs.empty())
    return false;

  // Read the first result
  *out = addrs[0];

  return true;
}

// static
std::vector<InetAddress> InetAddress::resolveAll(StringArg hostname, uint16_t port)
{
  std::vector<InetAddress> addrs;

  struct addrinfo* result = NULL;
  int error = getaddrinfo(hostname.c_str(), NULL, NULL, &result);
  if (error != 0)
  {
    if (error == EAI_SYSTEM)
    {
      perror("InetAddress::resolve");
    }
    else
    {
      fprintf(stderr, "InetAddress::resolve: %s\n", gai_strerror(error));
    }
    return addrs;
  }

  assert(result);
  std::unique_ptr<struct addrinfo, decltype(&freeaddrinfo)> freeResult(result, freeaddrinfo);

  for (struct addrinfo* ai = result; ai != NULL; ai = ai->ai_next)
  {
    InetAddress addr(*ai->ai_addr);
    addr.setPort(port);
    addrs.push_back(addr);
  }
  return addrs;
}
