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

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

#include <thread>

using muduo::Timestamp;

static struct tcp_info getTcpInfo(int fd)
{
  struct tcp_info tcpi = {0};
  socklen_t len = sizeof(tcpi);
  if (getsockopt(fd, IPPROTO_TCP, TCP_INFO, &tcpi, &len) < 0)
    perror("getsockopt(TCP_INFO)");
  return tcpi;
}

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

  void reportEnd(double now, int64_t total_bytes, const char* role)
  {
    printf("%s %sBytes in %.3fs, throughput: ",
           role, formatSI(total_bytes).c_str(), now);
    double bps = total_bytes / now;
    printf("%sBytes/s, ", formatSI(bps).c_str());
    printf("%sbits/s\n", formatSI(bps * 8).c_str());
  }

  void reportSender()
  {
    struct tcp_info tcpi = getTcpInfo(fd_);

    printf("min_rtt %s, ", formatMicrosecond(tcpi.tcpi_min_rtt).c_str());
    printf("rto %s, ", formatMicrosecond(tcpi.tcpi_rto).c_str());
    printf("data_segs_out %d, ", tcpi.tcpi_data_segs_out);
    printf("total_retrans %d, ", tcpi.tcpi_total_retrans);
    printf("bytes_retrans %lld, ", tcpi.tcpi_bytes_retrans);
    printf("reord_seen %d\n", tcpi.tcpi_reord_seen);
    printf("busy_time %s, ", formatMicrosecond(tcpi.tcpi_busy_time).c_str());
    printf("rwnd_limited %s, ", formatMicrosecond(tcpi.tcpi_rwnd_limited).c_str());
    printf("sndbuf_limited %s, ", formatMicrosecond(tcpi.tcpi_sndbuf_limited).c_str());
    printf("\n");
  }

 private:

  static std::string formatSI(double bps)
  {
    char buf[64];

    if (bps <= 9990)
      snprintf(buf, sizeof buf, "%.2fk", bps/1e3);
    else if (bps <= 99.9e3)
      snprintf(buf, sizeof buf, "%.1fk", bps/1e3);
    else if (bps <= 9999e3)
      snprintf(buf, sizeof buf, "%.0fk", bps/1e3);

    else if (bps <= 99.9e6)
      snprintf(buf, sizeof buf, "%.1fM", bps/1e6);
    else if (bps <= 9999e6)
      snprintf(buf, sizeof buf, "%.0fM", bps/1e6);

    else if (bps <= 99.9e9)
      snprintf(buf, sizeof buf, "%.1fG", bps/1e9);
    else if (bps <= 9999e9)
      snprintf(buf, sizeof buf, "%.0fG", bps/1e9);
    else
      snprintf(buf, sizeof buf, "%gT", bps/1e12);

    return buf;
  }

  static std::string formatIEC(int64_t s)
  {
    double n = static_cast<double>(s);
    char buf[64];
    const double Ki = 1024.0;
    const double Mi = Ki * 1024.0;
    const double Gi = Mi * 1024.0;

    if (n < 10000)
      snprintf(buf, sizeof buf, "%" PRId64, s);
    else if (n <= Ki*9.99)
      snprintf(buf, sizeof buf, "%.2fKi", n / Ki);
    else if (n <= Ki*99.9)
      snprintf(buf, sizeof buf, "%.1fKi", n / Ki);
    else if (n <= Ki*9999.0)
      snprintf(buf, sizeof buf, "%.0fKi", n / Ki);

    else if (n <= Mi*9.99)
      snprintf(buf, sizeof buf, "%.2fMi", n / Mi);
    else if (n <= Mi*99.9)
      snprintf(buf, sizeof buf, "%.1fMi", n / Mi);
    else if (n <= Mi*9999.0)
      snprintf(buf, sizeof buf, "%.0fMi", n / Mi);

    else if (n <= Gi*9.99)
      snprintf(buf, sizeof buf, "%.2fGi", n / Gi);
    else if (n <= Gi*99.9)
      snprintf(buf, sizeof buf, "%.1fGi", n / Gi);
    else if (n <= Gi*9999.0)
      snprintf(buf, sizeof buf, "%.0fGi", n / Gi);
    else
      snprintf(buf, sizeof buf, "%gGi", n / Gi);

    return buf;
  }

  static std::string formatMicrosecond(int us)
  {
    char buf[64];
    if (us < 1000)
      snprintf(buf, sizeof buf, "%dus", us);
    else if (us < 10000)
      snprintf(buf, sizeof buf, "%.2fms", us / 1e3);
    else if (us < 100000)
      snprintf(buf, sizeof buf, "%.1fms", us / 1e3);
    else if (us < 1000000)
      snprintf(buf, sizeof buf, "%.0fms", us / 1e3);
    else
      snprintf(buf, sizeof buf, "%.2fs", us / 1e6);

    return buf;
  }

  void report(double now, int64_t bytes, double elapsed)
  {
    double bps = elapsed > 0 ? bytes / elapsed : 0.0;

    printf("%7.3fs  ", now);
    printf("%6sB/s   ", formatSI(bps).c_str());
    printf("%5sbps  ", formatSI(bps * 8).c_str());
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

    struct tcp_info tcpi = getTcpInfo(fd_);

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
    int ca_state = tcpi.tcpi_ca_state;
    int64_t pacing = tcpi.tcpi_pacing_rate;
#elif __FreeBSD__
    int retrans = tcpi.tcpi_snd_rexmitpack;
    int ca_state = tcpi.__tcpi_ca_state;
    int64_t pacing = 0;
#endif
    int retr = retrans - last_retrans_;
    last_retrans_ = retrans;

    printf("%6s  ", formatIEC(snd_cwnd).c_str());
    printf("%6s  ", formatIEC(tcpi.tcpi_snd_wnd).c_str());
    printf("%6s  ", formatIEC(sndbuf).c_str());
    printf("%8s ",  formatIEC(ssthresh).c_str());
    printf("%5d  ", retr);
    printf("%2d  ", ca_state);
    printf("%6s  ", formatIEC(pacing).c_str());
    printf("%s/%d", formatMicrosecond(tcpi.tcpi_rtt).c_str(), tcpi.tcpi_rttvar);

    printf("\n");
  }

  void printReceiver() const
  {
    int rcvbuf = 0;
    socklen_t optlen = sizeof rcvbuf;
    if (::getsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &optlen) < 0)
      perror("getsockopt(RCVBUF)");

    struct tcp_info tcpi = getTcpInfo(fd_);

    printf("%6s  ", formatIEC(rcvbuf).c_str());
    printf("%9s  ", formatIEC(tcpi.tcpi_rcv_space).c_str());
    printf("%11s  ", formatIEC(tcpi.tcpi_rcv_ssthresh).c_str());
    printf("%7s  ", formatMicrosecond(tcpi.tcpi_rcv_rtt).c_str());
    printf("%7s  ", formatMicrosecond(tcpi.tcpi_ato).c_str());
    printf("\n");
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
  struct tcp_info tcpi = getTcpInfo(stream->fd());
  printf("Connected %s -> %s, MSS %d, congestion control: %s\n",
         stream->getLocalAddr().toIpPort().c_str(),
         stream->getPeerAddr().toIpPort().c_str(),
         tcpi.tcpi_snd_mss, cong);
  printf("Time (s)  Throughput   Bitrate    Cwnd    Rwnd  sndbuf  ssthresh  Retr  CA  Pacing  rtt/var\n");

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
  rpt.reportEnd(elapsed, total_bytes, "Tx");

  char buf[1024];
  int nr = stream->receiveSome(buf, sizeof buf);
  if (nr != 0)
    printf("nr = %d\n", nr);
  Timestamp end = Timestamp::now();
  elapsed = timeDifference(end, start);
  rpt.reportEnd(elapsed, total_bytes, "Rx");
  rpt.reportSender();
}

void serverThr(int id, TcpStreamPtr stream)
{
  printf("Time (s)  Throughput   Bitrate  rcvbuf  rcv_space rcv_ssthresh  rcv_rtt    ato\n");

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
  rpt.reportDelta(elapsed, bytes);
  rpt.reportEnd(elapsed, bytes, "Rx");
  struct tcp_info tcpi = getTcpInfo(stream->fd());
  printf("rcv_ooopack %d\n", tcpi.tcpi_rcv_ooopack);
  printf("Client no. %d done\n", id);
  printf("\n");
}

// a thread-per-connection discard server
void runServer(int port, bool ipv6)
{
  InetAddress listenAddr(port, ipv6);
  Acceptor acceptor(listenAddr);
  int count = 0;
  printf("Listening on port %d ... Ctrl-C to exit\n", port);
  while (true) {
    TcpStreamPtr stream = acceptor.accept();
    ++count;
    printf("Accepted no. %d client %s <- %s\n", count,
           stream->getLocalAddr().toIpPort().c_str(),
           stream->getPeerAddr().toIpPort().c_str());

    std::thread thr(serverThr, count, std::move(stream));
    thr.detach();
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

void help(const char* program)
{
  printf("Usage: %s [-s [-6]|-c Server_IP] [-t sec] [-b bytes] [-p port]\n", program);
}

int main(int argc, char* argv[])
{
  int opt;
  bool client = false, server = false;
  std::string serverAddr;
  int port = 2009;
  bool ipv6 = false;
  const int64_t kGigaBytes = 1024 * 1024 * 1024;
  int64_t bytes_limit = 10 * kGigaBytes;
  double duration = 10;

  // TODO: set congestion control
  while ((opt = getopt(argc, argv, "s6c:t:b:p:")) != -1) {
    switch (opt) {
      case 's':
        server = true;
        break;
      case '6':
        ipv6 = true;
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
        help(argv[0]);
        return 1;
        break;
    }
  }

  // TODO: resolve host name
  if (client)
    runClient(InetAddress(serverAddr, port), bytes_limit, duration);
  else if (server)
    runServer(port, ipv6);
  else
    help(argv[0]);
}
