#include "faketcp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/if_ether.h>

void tcp_input(int fd, const void* input, const void* payload, int tot_len)
{
  const struct iphdr* iphdr = static_cast<const struct iphdr*>(input);
  const struct tcphdr* tcphdr = static_cast<const struct tcphdr*>(payload);
  const int iphdr_len = iphdr->ihl*4;
  const int tcp_seg_len = tot_len - iphdr_len;
  const int tcphdr_size = sizeof(*tcphdr);
  if (tcp_seg_len >= tcphdr_size
      && tcp_seg_len >= tcphdr->doff*4)
  {
    const int tcphdr_len = tcphdr->doff*4;

    char source[INET_ADDRSTRLEN];
    char dest[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &iphdr->saddr, source, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &iphdr->daddr, dest, INET_ADDRSTRLEN);
    printf("IP %s.%d > %s.%d: ",
           source, ntohs(tcphdr->source), dest, ntohs(tcphdr->dest));
    printf("Flags [%c], seq %u, win %d, length %d\n",
           tcphdr->syn ? 'S' : (tcphdr->fin ? 'F' : '.'),
           ntohl(tcphdr->seq),
           ntohs(tcphdr->window),
           tot_len - iphdr_len - tcphdr_len);

    union
    {
      unsigned char output[ETH_FRAME_LEN];
      struct
      {
        struct iphdr iphdr;
        struct tcphdr tcphdr;
      } out;
    };

    assert(sizeof(out) == sizeof(struct iphdr) + sizeof(struct tcphdr));
    int output_len = sizeof(out);
    bzero(&out, output_len + 4);
    memcpy(output, input, sizeof(struct iphdr));

    out.iphdr.tot_len = htons(output_len);
    std::swap(out.iphdr.saddr, out.iphdr.daddr);
    out.iphdr.check = 0;
    out.iphdr.check = in_checksum(output, sizeof(struct iphdr));

    out.tcphdr.source = tcphdr->dest;
    out.tcphdr.dest = tcphdr->source;
    out.tcphdr.ack_seq = htonl(ntohl(tcphdr->seq)+1);
    out.tcphdr.doff = sizeof(struct tcphdr) / 4;
    out.tcphdr.window = htons(5000);

    bool response = false;
    if (tcphdr->syn)
    {
      out.tcphdr.seq = htonl(123456);
      out.tcphdr.syn = 1;
      out.tcphdr.ack = 1;
      response = true;
    }
    else if (tcphdr->fin)
    {
      out.tcphdr.seq = htonl(123457);
      out.tcphdr.fin = 1;
      out.tcphdr.ack = 1;
      response = true;
    }

    unsigned char* pseudo = output + output_len;
    pseudo[0] = 0;
    pseudo[1] = IPPROTO_TCP;
    pseudo[2] = 0;
    pseudo[3] = sizeof(struct tcphdr);
    out.tcphdr.check = in_checksum(&out.iphdr.saddr, sizeof(struct tcphdr)+12);
    if (response)
    {
      write(fd, output, output_len);
    }
  }
}

int main()
{
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
    union
    {
      unsigned char buf[ETH_FRAME_LEN];
      struct iphdr iphdr;
    };

    const int iphdr_size = sizeof iphdr;

    int nread = read(fd, buf, sizeof(buf));
    if (nread < 0)
    {
      perror("read");
      close(fd);
      exit(1);
    }
    printf("read %d bytes from tunnel interface %s.\n", nread, ifname);

    const int iphdr_len = iphdr.ihl*4;
    if (nread >= iphdr_size
        && iphdr.version == 4
        && iphdr_len >= iphdr_size
        && iphdr_len <= nread
        && iphdr.tot_len == htons(nread)
        && in_checksum(buf, iphdr_len) == 0)
    {
      const void* payload = buf + iphdr_len;
      if (iphdr.protocol == IPPROTO_ICMP)
      {
        icmp_input(fd, buf, payload, nread);
      }
      else if (iphdr.protocol == IPPROTO_TCP)
      {
        tcp_input(fd, buf, payload, nread);
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
