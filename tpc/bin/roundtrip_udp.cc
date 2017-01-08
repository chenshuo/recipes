#include "InetAddress.h"
#include "Socket.h"

#include <thread>

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

const int g_port = 3123;

struct Message
{
  int64_t request;
  int64_t response;
} __attribute__ ((__packed__));

static_assert(sizeof(Message) == 16, "Message size should be 16 bytes");

int64_t now()
{
  struct timeval tv = { 0, 0 };
  gettimeofday(&tv, NULL);
  return tv.tv_sec * int64_t(1000000) + tv.tv_usec;
}

void runServer()
{
  Socket sock(Socket::createUDP());
  sock.bindOrDie(InetAddress(g_port));

  while (true)
  {
    Message message = { 0, 0 };

    struct sockaddr peerAddr;
    bzero(&peerAddr, sizeof peerAddr);
    socklen_t addrLen = sizeof peerAddr;
    ssize_t nr = ::recvfrom(sock.fd(), &message, sizeof message, 0, &peerAddr, &addrLen);
    if (nr == sizeof message)
    {
      message.response = now();
      ssize_t nw = ::sendto(sock.fd(), &message, sizeof message, 0, &peerAddr, addrLen);
      if (nw < 0)
      {
        perror("send Message");
      }
      else if (nw != sizeof message)
      {
        printf("sent message of %zd bytes, expect %zd bytes.\n", nw, sizeof message);
      }
    }
    else if (nr < 0)
    {
      perror("recv Message");
    }
    else
    {
      printf("received message of %zd bytes, expect %zd bytes.\n", nr, sizeof message);
    }
  }
}

void runClient(const char* server_hostname)
{
  Socket sock(Socket::createUDP());
  InetAddress serverAddr(g_port);
  if (!InetAddress::resolve(server_hostname, &serverAddr))
  {
    printf("Unable to resolve %s\n", server_hostname);
    return;
  }

  if (sock.connect(serverAddr) != 0)
  {
    perror("connect to server");
    return;
  }

  std::thread thr([&sock] () {
    while (true)
    {
      Message message = { 0, 0 };
      message.request = now();
      int nw = sock.write(&message, sizeof message);
      if (nw < 0)
      {
        perror("send Message");
      }
      else if (nw != sizeof message)
      {
        printf("sent message of %d bytes, expect %zd bytes.\n", nw, sizeof message);
      }

      ::usleep(200*1000);
    }
  });

  while (true)
  {
    Message message = { 0, 0 };
    int nr = sock.read(&message, sizeof message);
    if (nr == sizeof message)
    {
      int64_t back = now();
      int64_t mine = (back + message.request) / 2;
      printf("now %jd round trip %jd clock error %jd\n",
             back, back - message.request, message.response - mine);
    }
    else if (nr < 0)
    {
      perror("send Message");
    }
    else
    {
      printf("received message of %d bytes, expect %zd bytes.\n", nr, sizeof message);
    }
  }
}

int main(int argc, const char* argv[])
{
  if (argc < 2)
  {
    printf("Usage:\nServer: %s -s\nClient: %s server_hostname\n", argv[0], argv[0]);
    return 0;
  }

  if (strcmp(argv[1], "-s") == 0)
  {
    runServer();
  }
  else
  {
    runClient(argv[1]);
  }
}
