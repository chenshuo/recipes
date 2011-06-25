#include <algorithm>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <linux/if_tun.h>

#define PACKET_SIZE 1514

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

void icmp_echo(int fd, const void* input, const void* payload, int len)
{
  const struct iphdr* iphdr = static_cast<const struct iphdr*>(input);
  const struct icmphdr* icmphdr = static_cast<const struct icmphdr*>(payload);
  const int icmphdr_size = sizeof(*icmphdr);

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
           len - iphdr->ihl*4);
  }

  union
  {
    unsigned char output[PACKET_SIZE];
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

uint16_t iphdr_checksum(const void* buf, int len)
{
  const uint16_t* data = static_cast<const uint16_t*>(buf);
  int sum = 0;
  for (int i = 0; i < len; i+=2)
  {
    sum += *data++;
  }
  // while (sum >> 16)
  sum = (sum & 0xFFFF) + (sum >> 16);
  return ~sum;
}

int main()
{
  union
  {
    unsigned char buf[PACKET_SIZE];
    struct iphdr iphdr;
  };

  const int iphdr_size = sizeof iphdr;

  char ifname[IFNAMSIZ] = "tun%d";
  int fd = tun_alloc(ifname);

  if (fd < 0)
  {
    fprintf(stderr, "tunnel interface allocation failed\n");
    exit(1);
  }

  printf("allocted tunnel interface %s\n", ifname);
  sleep(1);

  for (;;)
  {
    int nread = read(fd, buf, sizeof(buf));
    if (nread < 0)
    {
      perror("read");
      close(fd);
      exit(1);
    }
    printf("read %d bytes from tunnel interface %s.\n", nread, ifname);

    if (nread >= iphdr_size
        && iphdr.version == 4
        && iphdr.ihl*4 >= iphdr_size
        && iphdr.ihl*4 <= nread
        && iphdr.tot_len == htons(nread)
        && iphdr_checksum(buf, iphdr.ihl*4) == 0)
    {
      const void* payload = buf + iphdr.ihl*4;
      if (iphdr.protocol == 1)
      {
        icmp_echo(fd, buf, payload, nread);
      }
    }
    else
    {
      printf("bad packet\n");
      for (int i = 0; i < nread; ++i)
      {
        if (i % 4 == 0) printf("\n");
        printf("%02x ", buf[i]);
      }
      printf("\n");
    }
  }

  return 0;
}
