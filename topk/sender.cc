#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <fstream>
#include <vector>

#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

using namespace muduo;
using namespace muduo::net;

typedef std::vector<std::pair<int64_t, string> > WordCountList;

WordCountList g_wordCounts;

void read(const char* file)
{
  std::ifstream in(file);
  std::string line;
  while (getline(in, line))
  {
    size_t tab = line.find('\t');
    if (tab != string::npos)
    {
      int64_t count = strtoll(line.c_str() + tab, NULL, 10);
      if (count > 0)
      {
        string word(line.begin(), line.begin()+tab);
        g_wordCounts.push_back(make_pair(count, word));
      }
    }
  }
  std::sort(g_wordCounts.begin(), g_wordCounts.end(),
            std::greater<WordCountList::value_type>());
}

WordCountList::iterator fillBuffer(WordCountList::iterator first, Buffer* buf)
{
  while (first != g_wordCounts.end())
  {
    char count[32];
    snprintf(count, sizeof count, "%" PRId64 "\t", first->first);
    buf->append(count);
    buf->append(first->second);
    buf->append("\n", 1);
    ++first;
    if (buf->readableBytes() > 65536)
    {
      break;
    }
  }
  return first;
}

void send(const TcpConnectionPtr& conn, WordCountList::iterator first)
{
  Buffer buf;
  WordCountList::iterator last = fillBuffer(first, &buf);
  conn->setContext(last);
  conn->send(&buf);
}

void onConnection(const TcpConnectionPtr& conn)
{
  LOG_INFO << "Sender - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected())
  {
    send(conn, g_wordCounts.begin());
  }
}

void onWriteComplete(const TcpConnectionPtr& conn)
{
  WordCountList::iterator first = boost::any_cast<WordCountList::iterator>(conn->getContext());
  if (first != g_wordCounts.end())
  {
    send(conn, first);
  }
  else
  {
    conn->shutdown();
    LOG_INFO << "Sender - done";
  }
}

void serve(uint16_t port)
{
  LOG_INFO << "Listen on port " << port;
  EventLoop loop;
  InetAddress listenAddr(port);
  TcpServer server(&loop, listenAddr, "Sender");
  server.setConnectionCallback(onConnection);
  server.setWriteCompleteCallback(onWriteComplete);
  server.start();
  loop.loop();
}

int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    read(argv[1]);
    int port = argc > 2 ? atoi(argv[2]) : 2013;
    serve(static_cast<uint16_t>(port));
  }
  else
  {
    fprintf(stderr, "Usage: %s shard_file [port]\n", argv[0]);
  }
}
