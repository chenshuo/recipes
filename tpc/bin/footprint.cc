#include <stdio.h>

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

void snapshot(const char* name)
{
  printf("===== %s =====\n", name);
  dump("/proc/meminfo");
  dump("/proc/slabinfo");
}

int main(int argc, char* argv[])
{
  const int N = argc > 1 ? atoi(argv[1]) : 1000;

  {
    char buf[32];
    snprintf(buf, sizeof buf, "start N=%d", N);
    snapshot(buf);
  }

  InetAddress listenAddr(2222);
  Acceptor acceptor(listenAddr);
  snapshot("acceptor created");

  std::vector<Socket> clients;
  for (int i = 0; i < N; ++i)
    clients.push_back(Socket::createTCP());
  snapshot("clients created");

  std::vector<Socket> servers;
  InetAddress serverAddr("127.0.0.1", 2222);
  for (int i = 0; i < N; ++i)
  {
    if (clients[i].connect(serverAddr))
    {
      perror("connect");
      break;
    }
    servers.push_back(acceptor.acceptSocketOrDie());
  }
  snapshot("clients connected");

  // TODO: epoll

  servers.clear();
  snapshot("servers disconnected");

  clients.clear();
  snapshot("clients disconnected");
}
