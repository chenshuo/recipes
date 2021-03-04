#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "benchmark/benchmark.h"

void readwrite(int fds[], void* buf, int len)
{
  int nw = write(fds[1], buf, len);
  if (nw != len) {
    perror("write");
    printf("%d != %d\n", nw, len);
    assert(0);
  }
  int nr = read(fds[0], buf, len);
  assert(nr == len);
}

void do_benchmark(benchmark::State& state, int fds[], int len)
{
  void* buf = ::malloc(len);
  memset(buf, 0, len);
  for (auto _ : state)
  {
    readwrite(fds, buf, len);
  }
  state.SetBytesProcessed(len * state.iterations());
  state.SetItemsProcessed(state.iterations());
  ::free(buf);
}

const int bufsize = 128 * 1024;

static void BM_pipe(benchmark::State& state)
{
  static int fds[2] = { 0, 0 };
  if (fds[0] == 0) {
    if (pipe(fds) < 0) {
      perror("pipe");
      assert(0);
    }
    fcntl(fds[0], F_SETPIPE_SZ, bufsize);
  }

  int64_t len = state.range(0);
  do_benchmark(state, fds, len);
}
BENCHMARK(BM_pipe)->Range(8, 1 << 17);

static void BM_unix(benchmark::State& state)
{
  static int fds[2] = { 0, 0 };
  if (fds[0] == 0) {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
      perror("socketpair");
      assert(0);
    }
  }

  int64_t len = state.range(0);
  do_benchmark(state, fds, len);
}
BENCHMARK(BM_unix)->Range(8, 1 << 17);

static void BM_tcp(benchmark::State& state)
{
  static int fds[2];
  if (fds[0] == 0) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    listen(listenfd, 5);
    struct sockaddr_storage saddr;
    socklen_t addr_len = sizeof saddr;
    getsockname(listenfd, (struct sockaddr*)&saddr, &addr_len);

    fds[0] = socket(AF_INET, SOCK_STREAM, 0);
    connect(fds[0], (struct sockaddr*)&saddr, addr_len);

    fds[1] = accept(listenfd, NULL, NULL);
    close(listenfd);

    int one = 1;
    setsockopt(fds[1], IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
    setsockopt(fds[1], SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof bufsize);
    setsockopt(fds[0], SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof bufsize);
  }

  int64_t len = state.range(0);
  do_benchmark(state, fds, len);
}
BENCHMARK(BM_tcp)->Range(8, 1 << 17);

