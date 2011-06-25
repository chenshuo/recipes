#include <algorithm>

#include <assert.h>
#include <stdint.h>
#include <arpa/inet.h>  // inet_ntop
#include <net/if.h>

int tun_alloc(char *dev);
uint16_t in_checksum(const void* buf, int len);

void icmp_input(int fd, const void* input, const void* payload, int len);
