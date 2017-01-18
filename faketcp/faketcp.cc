#include "faketcp.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/ioctl.h>

int sethostaddr(const char* dev)
{
  struct ifreq ifr;
  bzero(&ifr, sizeof(ifr));
  strcpy(ifr.ifr_name, dev);
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, "192.168.0.1", &addr.sin_addr);
  //addr.sin_addr.s_addr = htonl(0xc0a80001);
  bcopy(&addr, &ifr.ifr_addr, sizeof addr);
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    return sockfd;
  int err = 0;
  // ifconfig tun0 192.168.0.1
  if ((err = ioctl(sockfd, SIOCSIFADDR, (void *) &ifr)) < 0)
  {
    perror("ioctl SIOCSIFADDR");
    goto done;
  }
  // ifup tun0
  if ((err = ioctl(sockfd, SIOCGIFFLAGS, (void *) &ifr)) < 0)
  {
    perror("ioctl SIOCGIFFLAGS");
    goto done;
  }
  ifr.ifr_flags |= IFF_UP;
  if ((err = ioctl(sockfd, SIOCSIFFLAGS, (void *) &ifr)) < 0)
  {
    perror("ioctl SIOCSIFFLAGS");
    goto done;
  }
  // ifconfig tun0 192.168.0.1/24
  inet_pton(AF_INET, "255.255.255.0", &addr.sin_addr);
  bcopy(&addr, &ifr.ifr_netmask, sizeof addr);
  if ((err = ioctl(sockfd, SIOCSIFNETMASK, (void *) &ifr)) < 0)
  {
    perror("ioctl SIOCSIFNETMASK");
    goto done;
  }
done:
  close(sockfd);
  return err;
}

int tun_alloc(char dev[IFNAMSIZ])
{
  struct ifreq ifr;
  int fd, err;

  if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
  {
    perror("open");
    return -1;
  }

  bzero(&ifr, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

  if (*dev)
  {
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }

  if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0)
  {
    perror("ioctl TUNSETIFF");
    close(fd);
    return err;
  }
  strcpy(dev, ifr.ifr_name);
  if ((err = sethostaddr(dev)) < 0)
    return err;

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

