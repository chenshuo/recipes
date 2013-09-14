#include "faketcp.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/ioctl.h>

int tun_alloc(char *dev)
{
  struct ifreq ifr;
  int fd, err;

  if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
  {
    perror("open");
    return -1;
  }

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

  if (*dev)
  {
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }

  if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0)
  {
    perror("ioctl");
    close(fd);
    return err;
  }
  strcpy(dev, ifr.ifr_name);

  return fd;
}

uint16_t in_checksum(const void* buf, int len)
{
  assert(len % 2 == 0);
  const uint16_t* data = static_cast<const uint16_t*>(buf);
  int sum = 0;
  for (int i = 0; i < len; i+=2)
  {
    sum += *data++;
  }
  while (sum >> 16)
    sum = (sum & 0xFFFF) + (sum >> 16);
  assert(sum <= 0xFFFF);
  return ~sum;
}

void icmp_input(int fd, const void* input, const void* payload, int len)
{
  const struct iphdr* iphdr = static_cast<const struct iphdr*>(input);
  const struct icmphdr* icmphdr = static_cast<const struct icmphdr*>(payload);
  // const int icmphdr_size = sizeof(*icmphdr);
  const int iphdr_len = iphdr->ihl*4;

  if (icmphdr->type == ICMP_ECHO)
  {
    char source[INET_ADDRSTRLEN];
    char dest[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &iphdr->saddr, source, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &iphdr->daddr, dest, INET_ADDRSTRLEN);
    printf("%s > %s: ", source, dest);
    printf("ICMP echo request, id %d, seq %d, length %d\n",
           ntohs(icmphdr->un.echo.id),
           ntohs(icmphdr->un.echo.sequence),
           len - iphdr_len);

    union
    {
      unsigned char output[ETH_FRAME_LEN];
      struct
      {
        struct iphdr iphdr;
        struct icmphdr icmphdr;
      } out;
    };

    memcpy(output, input, len);
    out.icmphdr.type = ICMP_ECHOREPLY;
    out.icmphdr.checksum += ICMP_ECHO; // FIXME: not portable
    std::swap(out.iphdr.saddr, out.iphdr.daddr);
    write(fd, output, len);
  }
}

