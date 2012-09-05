#include "Connector.h"
#include "EventLoop.h"

#include <stdio.h>

muduo::EventLoop* g_loop;

void connectCallback(int sockfd)
{
  printf("connected.\n");
  g_loop->quit();
}

int main(int argc, char* argv[])
{
  muduo::EventLoop loop;
  g_loop = &loop;
  muduo::InetAddress addr("127.0.0.1", 9981);
  muduo::ConnectorPtr connector(new muduo::Connector(&loop, addr));
  connector->setNewConnectionCallback(connectCallback);
  connector->start();

  loop.loop();
}
