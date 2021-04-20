#include "datetime/Timestamp.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "TcpStream.h"

#ifdef __linux
#include <linux/tcp.h>
#else
#include <netinet/tcp.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using muduo::Timestamp;

class BandwidthReporter
{
 public:
  BandwidthReporter(int fd, bool sender)
      : fd_(fd), sender_(sender)
  {
  }

  void reportDelta(double now, int64_t total_bytes)
  {
    report(now, total_bytes, total_bytes - last_bytes_, now - last_time_);
    last_time_ = now;
    last_bytes_ = total_bytes;
  }

  void reportAll(double now, int64_t total_bytes) const
  {
    report(now, total_bytes, total_bytes, now);
  }

 private:
  void report(double now, int64_t total_bytes, int64_t bytes, double elapsed) const
  {
    printf("%6.3f  %.3fM  %.3fM/s ", now, total_bytes / 1e6,
           elapsed > 0 ? bytes / 1e6 / elapsed : 0.0);
    if (sender_)
      printSender();
    else
      printReceiver();
  }

  void printSender() const
  {
    int sndbuf = 0;
    socklen_t optlen = sizeof sndbuf;
    if (::getsockopt(fd_, SOL_SOCKET, SO_SNDBUF, &sndbuf, &optlen) < 0)
      perror("getsockopt(SNDBUF)");

    struct tcp_info tcpi = {0};
    socklen_t len = sizeof(tcpi);
    if (getsockopt(fd_, IPPROTO_TCP, TCP_INFO, &tcpi, &len) < 0)
      perror("getsockopt(TCP_INFO)");

    // bytes_in_flight = tcpi.tcpi_bytes_sent - tcpi.tcpi_bytes_acked;
    // tcpi.tcpi_notsent_bytes;

    printf(" sndbuf=%.1fK snd_wnd=%.1fK rtt=%d/%d\n",
           sndbuf / 1024.0, tcpi.tcpi_snd_wnd / 1024.0,
           tcpi.tcpi_rtt, tcpi.tcpi_rttvar);
  }

  void printReceiver() const
  {
    int rcvbuf = 0;
    socklen_t optlen = sizeof rcvbuf;
    if (::getsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &optlen) < 0)
      perror("getsockopt(RCVBUF)");

    printf(" rcvbuf=%.1fK\n", rcvbuf / 1024.0);
  }

  const int fd_ = 0;
  const bool sender_ = false;
  double last_time_ = 0;
  int64_t last_bytes_ = 0;
};

void runClient(const InetAddress& serverAddr, int64_t bytes_limit, double duration)
{
  TcpStreamPtr stream(TcpStream::connect(serverAddr));
  if (!stream) {
    printf("Unable to connect %s\n", serverAddr.toIpPort().c_str());
    perror("");
    return;
  }

  const Timestamp start = Timestamp::now();
  const int block_size = 64 * 1024;
  std::string message(block_size, 'S');
  int seconds = 1;
  int64_t total_bytes = 0;
  double elapsed = 0;
  BandwidthReporter rpt(stream->fd(), true);
  rpt.reportAll(0, 0);

  while (total_bytes < bytes_limit) {
    int nw = stream->sendAll(message.data(), message.size());
    if (nw <= 0)
      break;
    total_bytes += nw;
    elapsed = timeDifference(Timestamp::now(), start);

    if (elapsed >= duration)
      break;

    if (elapsed >= seconds) {
      rpt.reportDelta(elapsed, total_bytes);
      while (elapsed >= seconds)
        ++seconds;
    }
  }

  stream->shutdownWrite();
  Timestamp shutdown = Timestamp::now();
  elapsed = timeDifference(shutdown, start);
  rpt.reportDelta(elapsed, total_bytes);

  char buf[1024];
  int nr = stream->receiveSome(buf, sizeof buf);
  if (nr != 0)
    printf("nr = %d\n", nr);
  Timestamp end = Timestamp::now();
  elapsed = timeDifference(end, start);
  rpt.reportAll(elapsed, total_bytes);
}

void runServer(int port)
{
  InetAddress listenAddr(port);
  Acceptor acceptor(listenAddr);
  int count = 0;
  while (true) {
    printf("Accepting on port %d ... Ctrl-C to exit\n", port);
    TcpStreamPtr stream = acceptor.accept();
    printf("accepted no. %d client\n", ++count);

    const Timestamp start = Timestamp::now();
    int seconds = 1;
    int64_t bytes = 0;
    double elapsed = 0;
    BandwidthReporter rpt(stream->fd(), false);
    rpt.reportAll(elapsed, bytes);

    char buf[65536];
    while (true) {
      int nr = stream->receiveSome(buf, sizeof buf);
      if (nr <= 0)
        break;
      bytes += nr;

      elapsed = timeDifference(Timestamp::now(), start);
      if (elapsed >= seconds) {
        rpt.reportDelta(elapsed, bytes);
        while (elapsed >= seconds)
          ++seconds;
      }
    }
    elapsed = timeDifference(Timestamp::now(), start);
    rpt.reportAll(elapsed, bytes);
    printf("Client no. %d done\n", count);
  }
}

int main(int argc, char* argv[])
{
  int opt;
  bool client = false, server = false;
  InetAddress serverAddr;
  const int port = 2009;
  const int64_t kGigaBytes = 1024 * 1024 * 1024;
  int64_t bytes_limit = 10 * kGigaBytes;
  double duration = 10;

  while ((opt = getopt(argc, argv, "sc:")) != -1) {
    switch (opt) {
      case 's':
        server = true;
        break;
      case 'c':
        client = true;
        serverAddr = InetAddress(optarg, port);
        break;
      default:
        fprintf(stderr, "Usage: %s FIXME\n", argv[0]);
        break;
    }
  }

  if (client)
    runClient(serverAddr, bytes_limit, duration);
  else if (server)
    runServer(port);
}
