#include <algorithm>  // std::swap

#include <boost/static_assert.hpp>

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>  // inet_ntop
#include <net/if.h>

struct SocketAddr
{
  uint32_t saddr, daddr;
  uint16_t sport, dport;

  bool operator==(const SocketAddr& rhs) const
  {
    return saddr == rhs.saddr && daddr == rhs.daddr && sport == rhs.sport && dport == rhs.dport;
  }

  bool operator<(const SocketAddr& rhs) const
  {
    BOOST_STATIC_ASSERT(sizeof(SocketAddr) == 12);
    return memcmp(this, &rhs, sizeof(rhs)) < 0;
  }
};

int tun_alloc(char dev[IFNAMSIZ]);
uint16_t in_checksum(const void* buf, int len);

void icmp_input(int fd, const void* input, const void* payload, int len);
