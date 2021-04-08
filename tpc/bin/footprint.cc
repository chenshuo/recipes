#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "Acceptor.h"
#include "InetAddress.h"
#include "Socket.h"
#include "TcpStream.h"


void dump(const char* filename)
{
  char buf[65536];
  FILE* fp = fopen(filename, "r");
  if (fp)
  {
    ssize_t nr;
    while ( (nr = fread(buf, 1, sizeof buf, fp)) > 0)
      fwrite(buf, 1, nr, stdout);

    fclose(fp);
  }
}

void snapshot(const char* name, int N)
{
  printf("===== %d %s =====\n", N, name);
  dump("/proc/meminfo");
  dump("/proc/slabinfo");
}

int main(int argc, char* argv[])
{
  const int N = argc > 1 ? atoi(argv[1]) : 1000;

  snapshot("started", N);

  InetAddress listenAddr(2222);
  Acceptor acceptor(listenAddr);
  snapshot("acceptor created", 1);

  InetAddress serverAddr("127.0.0.1", 2222);

  std::vector<Socket> clients;
  clients.reserve(N);
  for (int i = 0; i < N; ++i)
    clients.push_back(Socket::createTCP(serverAddr.family()));
  snapshot("clients created", N);

  std::vector<Socket> servers;
  servers.reserve(N);
  for (int i = 0; i < N; ++i)
  {
    char buf[64];
    const int ports_per_ip = 16384;
    int clientIP = i / ports_per_ip;
    snprintf(buf, sizeof buf, "127.1.%d.%d", clientIP / 128, clientIP % 128);
    InetAddress localAddr(buf, 10000 + i % ports_per_ip);
    clients[i].bindOrDie(localAddr);
    if (i % 10000 == 0) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      fprintf(stderr, "%ld.%06ld Client %d bind to %s\n", tv.tv_sec, tv.tv_usec, i, buf);
    }
    if (clients[i].connect(serverAddr))
    {
      perror("connect");
      break;
    }
    servers.push_back(acceptor.acceptSocketOrDie());
  }
  snapshot("clients connected", N);

  // TODO: epoll

  servers.clear();
  snapshot("servers disconnected", N);

  clients.clear();
  snapshot("clients disconnected", N);
}
