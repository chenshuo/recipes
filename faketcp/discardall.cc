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
    const int payload_len = tot_len - iphdr_len - tcphdr_len;

    char source[INET_ADDRSTRLEN];
    char dest[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &iphdr->saddr, source, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &iphdr->daddr, dest, INET_ADDRSTRLEN);
    printf("IP %s.%d > %s.%d: ",
           source, ntohs(tcphdr->source), dest, ntohs(tcphdr->dest));
    printf("Flags [%c], seq %u, win %d, length %d%s\n",
           tcphdr->syn ? 'S' : (tcphdr->fin ? 'F' : '.'),
           ntohl(tcphdr->seq),
           ntohs(tcphdr->window),
           payload_len,
           tcphdr_len > sizeof(struct tcphdr) ? " <>" : "");

    union
    {
      unsigned char output[ETH_FRAME_LEN];
      struct
      {
        struct iphdr iphdr;
        struct tcphdr tcphdr;
      } out;
    };

    static_assert(sizeof(out) == sizeof(struct iphdr) + sizeof(struct tcphdr), "");
    int output_len = sizeof(out);
    bzero(&out, output_len + 4);
    memcpy(output, input, sizeof(struct iphdr));

    out.tcphdr.source = tcphdr->dest;
    out.tcphdr.dest = tcphdr->source;
    out.tcphdr.doff = sizeof(struct tcphdr) / 4;
    out.tcphdr.window = htons(65000);

    bool response = false;
    const uint32_t seq = ntohl(tcphdr->seq);
    const uint32_t isn = 123456;
    if (tcphdr->syn)
    {
      out.tcphdr.seq = htonl(isn);
      out.tcphdr.ack_seq = htonl(seq+1);
      out.tcphdr.syn = 1;
      out.tcphdr.ack = 1;

      // set mss=1000
      unsigned char* mss = output + output_len;
      *mss++ = 2;
      *mss++ = 4;
      *mss++ = 0x03;
      *mss++ = 0xe8;  // 1000 == 0x03e8
      out.tcphdr.doff += 1;
      output_len += 4;

      response = true;
    }
    else if (tcphdr->fin)
    {
      out.tcphdr.seq = htonl(isn+1);
      out.tcphdr.ack_seq = htonl(seq+1);
      out.tcphdr.fin = 1;
      out.tcphdr.ack = 1;
      response = true;
    }
    else if (payload_len > 0)
    {
      out.tcphdr.seq = htonl(isn+1);
      out.tcphdr.ack_seq = htonl(seq+payload_len);
      out.tcphdr.ack = 1;
      response = true;
    }

    // build IP header
    out.iphdr.tot_len = htons(output_len);
    std::swap(out.iphdr.saddr, out.iphdr.daddr);
    out.iphdr.check = 0;
    out.iphdr.check = in_checksum(output, sizeof(struct iphdr));

    unsigned char* pseudo = output + output_len;
    pseudo[0] = 0;
    pseudo[1] = IPPROTO_TCP;
    pseudo[2] = 0;
    pseudo[3] = output_len - sizeof(struct iphdr);
    out.tcphdr.check = in_checksum(&out.iphdr.saddr, output_len - 8);
    if (response)
    {
      write(fd, output, output_len);
    }
  }
}

int main(int argc, char* argv[])
{
  char ifname[IFNAMSIZ] = "tun%d";
  bool offload = argc > 1 && strcmp(argv[1], "-K") == 0;
  int fd = tun_alloc(ifname, offload);

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
      unsigned char buf[IP_MAXPACKET];
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
    else if (nread == sizeof(buf))
    {
      printf("possible message truncated.\n");
    }
    printf("read %d bytes from tunnel interface %s.\n", nread, ifname);

    const int iphdr_len = iphdr.ihl*4;  // FIXME: check nread >= sizeof iphdr before accessing iphdr.ihl.
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
    else if (iphdr.version == 4)
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
