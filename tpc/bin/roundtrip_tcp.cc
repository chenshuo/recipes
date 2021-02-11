#include "InetAddress.h"
#include "TcpStream.h"

#include <memory>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <vector>

int64_t kNanos = 1e9;
int64_t clock_diff(struct timespec x, struct timespec y)
{
  return (x.tv_sec - y.tv_sec) * kNanos + x.tv_nsec - y.tv_nsec;
}

struct timespec get_time()
{
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now;
}

struct Sample
{
  int index;
  int64_t start_nano;
  int64_t rtt_nano;
};

std::vector<Sample> run(const char* host, int delay_ms, int length_s, int batch, int payload_size)
{
  struct timespec start = get_time();
  std::vector<Sample> rtts;

  InetAddress addr;
  if (!InetAddress::resolve(host, 3007, &addr))
  {
    printf("Unable to resolve %s\n", host);
    return rtts;
  }

  // printf("connecting to %s\n", addr.toIpPort().c_str());
  TcpStreamPtr stream(TcpStream::connect(addr));
  if (!stream)
  {
    printf("Unable to connect %s\n", addr.toIpPort().c_str());
    perror("");
    return rtts;
  }

  std::unique_ptr<char[]> payload(new char[payload_size]);

  while (true) {
    struct timespec now = get_time();
    if (clock_diff(now, start) > length_s * kNanos)
      break;

    for (int i = 0; i < batch; ++i) {
      struct timespec before = get_time();
      int nw = stream->sendAll(payload.get(), payload_size);
      if (nw != payload_size)
        return rtts;
      int nr = stream->receiveAll(payload.get(), payload_size);
      if (nr != payload_size)
        return rtts;
      struct timespec after = get_time();
      int64_t nanos = clock_diff(after, before);
      Sample s = {
        .index = i,
        .rtt_nano = nanos,
      };
      if (i == 0)
        s.start_nano = clock_diff(before, start);
      else
        s.start_nano = clock_diff(before, now);
      rtts.push_back(s);
    }

    if (delay_ms > 0) {
      ::usleep(delay_ms * 1000);
    }
  }
  return rtts;
}

int main(int argc, char* argv[])
{
  int opt;
  int delay = 0, length = 3, batch = 4, payload = 1;
  bool silent = false;
  while ((opt = getopt(argc, argv, "b:d:l:p:s")) != -1) {
    switch (opt) {
      case 'b':
        batch = atoi(optarg);
        break;
      case 'd':
        delay = atoi(optarg);
        break;
      case 'l':
        length = atoi(optarg);
        break;
      case 'p':
        payload = atoi(optarg);
        break;
      case 's':
        silent = true;
        break;
      default:
        ;
    }
  }
  if (optind >= argc) {
    fprintf(stderr, "Usage:\nroundtrip_tcp [-b batch_size] [-d delay_ms] [-l length_in_seconds] echo_server_host\n");
    return 1;
  }
  std::vector<Sample> rtts = run(argv[optind], delay, length, batch, payload);
  if (!silent) {
    for (Sample s : rtts) {
      printf("%d %ld %ld\n", s.index, s.start_nano, s.rtt_nano);
    }
  }
}
