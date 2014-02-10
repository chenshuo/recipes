#include "InetAddress.h"

#include <strings.h> // bzero

InetAddress::InetAddress(uint16_t port, bool loopbackOnly)
{
  ::bzero(&saddr_, sizeof(saddr_));
  saddr_.sin_family = AF_INET;
  saddr_.sin_addr.s_addr = htonl(loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY);
  saddr_.sin_port = htons(port);
}
