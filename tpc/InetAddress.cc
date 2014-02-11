#include "InetAddress.h"

#include <assert.h>
#include <netdb.h>
#include <strings.h> // bzero
#include <arpa/inet.h>


InetAddress::InetAddress(uint16_t port, bool loopbackOnly)
{
  ::bzero(&saddr_, sizeof(saddr_));
  saddr_.sin_family = AF_INET;
  saddr_.sin_addr.s_addr = htonl(loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY);
  saddr_.sin_port = htons(port);
}

std::string InetAddress::toIp() const
{
  char buf[32];
  ::inet_ntop(AF_INET, &saddr_.sin_addr, buf, sizeof buf);
  return buf;
}

std::string InetAddress::toIpPort() const
{
  char buf[32]; // "255.255.255.255:65535" 4*4+5+1 = 22
  uint16_t port = portHostEndian();
  snprintf(buf, sizeof buf, ":%u", port);
  std::string ip = toIp();
  ip += buf;
  return ip;
}

static const int kResolveBufSize = 4096; // RFC6891: EDNS payload 4096 bytes

bool InetAddress::resolveSlow(const char* hostname, InetAddress* out)
{
  std::vector<char> buf(2 * kResolveBufSize);
  struct hostent hent;
  struct hostent* he = NULL;
  int herrno = 0;
  bzero(&hent, sizeof(hent));

  while (buf.size() <= 16 * kResolveBufSize)
  {
    printf("len = %zd\n", buf.size());
    int ret = gethostbyname_r(hostname, &hent, buf.data(), buf.size(), &he, &herrno);
    if (ret == 0 && he != NULL)
    {
      assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
      out->saddr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
      return true;
    }
    else if (ret == ERANGE)
    {
      buf.resize(buf.size() * 2);
    }
    else
    {
      if (ret)
        perror("InetAddress::resolve");
      return false;
    }
  }
  return false;
}

bool InetAddress::resolve(const char* hostname, InetAddress* out)
{
  assert(out);
  char buf[kResolveBufSize];
  struct hostent hent;
  struct hostent* he = NULL;
  int herrno = 0;
  bzero(&hent, sizeof(hent));

  int ret = gethostbyname_r(hostname, &hent, buf, sizeof buf, &he, &herrno);
  if (ret == 0 && he != NULL)
  {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    out->saddr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  }
  else if (ret == ERANGE)
  {
    return resolveSlow(hostname, out);
  }
  else
  {
    if (ret)
      perror("InetAddress::resolve");
    return false;
  }
}
