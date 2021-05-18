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
    report(now, total_bytes - last_bytes_, now - last_time_);
    last_time_ = now;
    last_bytes_ = total_bytes;
  }

  void reportAll(double now, int64_t total_bytes, int64_t syscalls)
  {
    printf("Transferred %.3fMB %.3fMiB in %.3fs, %lld syscalls, %.1f Bytes/syscall\n",
           total_bytes / 1e6, total_bytes / (1024.0 * 1024), now, (long long)syscalls,
           total_bytes * 1.0 / syscalls);
    report(now, total_bytes, now);
  }

 private:
  void report(double now, int64_t bytes, double elapsed)
  {
    double mbps = elapsed > 0 ? bytes / 1e6 / elapsed : 0.0;
    printf("%6.3f  %6.2fMB/s  %6.1fMbits/s ", now, mbps, mbps*8);
    if (sender_)
      printSender();
    else
      printReceiver();
  }

  void printSender()
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
    int snd_cwnd = tcpi.tcpi_snd_cwnd;
    int ssthresh = tcpi.tcpi_snd_ssthresh;
#ifdef __linux
    snd_cwnd *= tcpi.tcpi_snd_mss;  // Linux's cwnd is # of mss.
    if (ssthresh < INT32_MAX)
      ssthresh *= tcpi.tcpi_snd_mss;
#endif

#ifdef __linux
    int retrans = tcpi.tcpi_total_retrans;
#elif __FreeBSD__
    int retrans = tcpi.tcpi_snd_rexmitpack;
#endif

    printf(" sndbuf=%.1fK snd_cwnd=%.1fK ssthresh=%.1fK snd_wnd=%.1fK rtt=%d/%d",
           sndbuf / 1024.0, snd_cwnd / 1024.0, ssthresh / 1024.0,
           tcpi.tcpi_snd_wnd / 1024.0, tcpi.tcpi_rtt, tcpi.tcpi_rttvar);
    if (retrans - last_retrans_ > 0) {
      printf(" retrans=%d", retrans - last_retrans_);
    }
    printf("\n");
    last_retrans_ = retrans;
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
  int last_retrans_ = 0;
};

void runClient(const InetAddress& serverAddr, int64_t bytes_limit, double duration)
{
  TcpStreamPtr stream(TcpStream::connect(serverAddr));
  if (!stream) {
    printf("Unable to connect %s\n", serverAddr.toIpPort().c_str());
    perror("");
    return;
  }
  char cong[64] = "";
  socklen_t optlen = sizeof cong;
  if (::getsockopt(stream->fd(), IPPROTO_TCP, TCP_CONGESTION, cong, &optlen) < 0)
      perror("getsockopt(TCP_CONGESTION)");
  printf("Connected %s -> %s, congestion control: %s\n",
         stream->getLocalAddr().toIpPort().c_str(),
         stream->getPeerAddr().toIpPort().c_str(), cong);

  const Timestamp start = Timestamp::now();
  const int block_size = 64 * 1024;
  std::string message(block_size, 'S');
  int seconds = 1;
  int64_t total_bytes = 0;
  int64_t syscalls = 0;
  double elapsed = 0;
  BandwidthReporter rpt(stream->fd(), true);
  rpt.reportDelta(0, 0);

  while (total_bytes < bytes_limit) {
    int bytes = std::min<int64_t>(message.size(), bytes_limit - total_bytes);
    int nw = stream->sendSome(message.data(), bytes);
    if (nw <= 0)
      break;
    total_bytes += nw;
    syscalls++;
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
  rpt.reportAll(elapsed, total_bytes, syscalls);
}

void runServer(int port)
{
  InetAddress listenAddr(port);
  Acceptor acceptor(listenAddr);
  int count = 0;
  while (true) {
    printf("Accepting on port %d ... Ctrl-C to exit\n", port);
    TcpStreamPtr stream = acceptor.accept();
    ++count;
    printf("accepted no. %d client %s <- %s\n", count,
           stream->getLocalAddr().toIpPort().c_str(),
           stream->getPeerAddr().toIpPort().c_str());

    const Timestamp start = Timestamp::now();
    int seconds = 1;
    int64_t bytes = 0;
    int64_t syscalls = 0;
    double elapsed = 0;
    BandwidthReporter rpt(stream->fd(), false);
    rpt.reportDelta(elapsed, bytes);

    char buf[65536];
    while (true) {
      int nr = stream->receiveSome(buf, sizeof buf);
      if (nr <= 0)
        break;
      bytes += nr;
      syscalls++;

      elapsed = timeDifference(Timestamp::now(), start);
      if (elapsed >= seconds) {
        rpt.reportDelta(elapsed, bytes);
        while (elapsed >= seconds)
          ++seconds;
      }
    }
    elapsed = timeDifference(Timestamp::now(), start);
    rpt.reportAll(elapsed, bytes, syscalls);
    printf("Client no. %d done\n", count);
  }
}

int64_t parseBytes(const char* arg)
{
  char* end = NULL;
  int64_t bytes = strtoll(arg, &end, 10);
  switch (*end) {
    case '\0':
      return bytes;
    case 'k':
      return bytes * 1000;
    case 'K':
      return bytes * 1024;
    case 'm':
      return bytes * 1000 * 1000;
    case 'M':
      return bytes * 1024 * 1024;
    case 'g':
      return bytes * 1000 * 1000 * 1000;
    case 'G':
      return bytes * 1024 * 1024 * 1024;
    default:
      return 0;
  }
}

int main(int argc, char* argv[])
{
  int opt;
  bool client = false, server = false;
  std::string serverAddr;
  int port = 2009;
  const int64_t kGigaBytes = 1024 * 1024 * 1024;
  int64_t bytes_limit = 10 * kGigaBytes;
  double duration = 10;

  while ((opt = getopt(argc, argv, "sc:t:b:p:")) != -1) {
    switch (opt) {
      case 's':
        server = true;
        break;
      case 'c':
        client = true;
        serverAddr = optarg;
        break;
      case 't':
        duration = strtod(optarg, NULL);
        break;
      case 'b':
        bytes_limit = parseBytes(optarg);
        break;
      case 'p':
        port = strtol(optarg, NULL, 10);
        break;
      default:
        fprintf(stderr, "Usage: %s FIXME\n", argv[0]);
        break;
    }
  }

  if (client)
    runClient(InetAddress(serverAddr, port), bytes_limit, duration);
  else if (server)
    runServer(port);
}
