#include "faketcp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/if_ether.h>

void tcp_input(int fd, const void* input, const void* payload, int tot_len, bool passive)
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
    printf("Flags [%c], seq %u, win %d, length %d\n",
           tcphdr->syn ? 'S' : (tcphdr->fin ? 'F' : '.'),
           ntohl(tcphdr->seq),
           ntohs(tcphdr->window),
           payload_len);

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
    out.tcphdr.doff = sizeof(struct tcphdr) / 4;
    out.tcphdr.window = htons(5000);

    bool response = false;
    if (tcphdr->syn)
    {
      out.tcphdr.seq = htonl(passive ? 123456 : 123457);
      out.tcphdr.ack_seq = htonl(ntohl(tcphdr->seq)+1);
      if (passive)
      {
        out.tcphdr.syn = 1;
      }
      out.tcphdr.ack = 1;
      response = true;
    }
    else if (tcphdr->fin)
    {
      out.tcphdr.seq = htonl(123457);
      out.tcphdr.ack_seq = htonl(ntohl(tcphdr->seq)+1);
      out.tcphdr.fin = 1;
      out.tcphdr.ack = 1;
      response = true;
    }
    else if (payload_len > 0)
    {
      out.tcphdr.seq = htonl(123457);
      out.tcphdr.ack_seq = htonl(ntohl(tcphdr->seq)+payload_len);
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


bool connect_one(int fd, uint32_t daddr, int dport, uint32_t saddr, int sport)
{
  {
    union
    {
      unsigned char output[ETH_FRAME_LEN];
      struct
      {
        struct iphdr iphdr;
        struct tcphdr tcphdr;
      } out;
    };

    bzero(&out, (sizeof out)+4);

    out.iphdr.version = IPVERSION;
    out.iphdr.ihl = sizeof(out.iphdr)/4;
    out.iphdr.tos = 0;
    out.iphdr.tot_len = htons(sizeof(out));
    out.iphdr.id = 55564;
    out.iphdr.frag_off |= htons(IP_DF);
    out.iphdr.ttl = IPDEFTTL;
    out.iphdr.protocol = IPPROTO_TCP;
    out.iphdr.saddr = saddr;
    out.iphdr.daddr = daddr;
    out.iphdr.check = in_checksum(output, sizeof(struct iphdr));

    out.tcphdr.source = sport;
    out.tcphdr.dest = dport;
    out.tcphdr.seq = htonl(123456);
    out.tcphdr.ack_seq = 0;
    out.tcphdr.doff = sizeof(out.tcphdr)/4;
    out.tcphdr.syn = 1;
    out.tcphdr.window = htons(4096);

    unsigned char* pseudo = output + sizeof out;
    pseudo[0] = 0;
    pseudo[1] = IPPROTO_TCP;
    pseudo[2] = 0;
    pseudo[3] = sizeof(struct tcphdr);
    out.tcphdr.check = in_checksum(&out.iphdr.saddr, sizeof(struct tcphdr)+12);

    write(fd, output, sizeof out);
  }

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
  // printf("read %d bytes from tunnel interface %s.\n", nread, ifname);

  if (nread >= iphdr_size
      && iphdr.version == 4
      && iphdr.ihl*4 >= iphdr_size
      && iphdr.ihl*4 <= nread
      && iphdr.tot_len == htons(nread)
      && in_checksum(buf, iphdr.ihl*4) == 0)
  {
    const void* payload = buf + iphdr.ihl*4;
    if (iphdr.protocol == IPPROTO_ICMP)
    {
      icmp_input(fd, buf, payload, nread);
    }
    else if (iphdr.protocol == IPPROTO_TCP)
    {
      tcp_input(fd, buf, payload, nread, false);
    }
  }

  return true;
}

void connect_many(int fd, const char* ipstr, int port, int count)
{
  uint32_t destip;
  inet_pton(AF_INET, ipstr, &destip);

  uint32_t srcip = ntohl(destip)+1;
  int srcport = 1024;

  for (int i = 0; i < count; ++i)
  {
    connect_one(fd, destip, htons(port), htonl(srcip), htons(srcport));
    srcport++;
    if (srcport > 0xFFFF)
    {
      srcport = 1024;
      srcip++;
    }
  }
}

void usage()
{
}

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    usage();
    return 0;
  }

  char ifname[IFNAMSIZ] = "tun%d";
  int fd = tun_alloc(ifname);

  if (fd < 0)
  {
    fprintf(stderr, "tunnel interface allocation failed\n");
    exit(1);
  }

  const char* ip = argv[1];
  int port = atoi(argv[2]);
  int count = atoi(argv[3]);
  printf("allocted tunnel interface %s\n", ifname);
  printf("press enter key to start connecting %s:%d\n", ip, port);
  getchar();

  connect_many(fd, ip, port, count);

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
        tcp_input(fd, buf, payload, nread, true);
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
