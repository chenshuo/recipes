#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/time.h>

double now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

struct BackTracking
{
  static const int kMaxQueens = 20;

  const int N;
  const uint32_t mask;
  int64_t count;

  BackTracking(int nqueens)
      : N(nqueens), mask(~((1 << N) - 1)), count(0)
  {
    assert(0 < N && N <= kMaxQueens);
  }

  void search(const int row,
              const uint32_t columns,
              const uint32_t diagnoal,
              const uint32_t antidiagnoal)
  {
    uint32_t avail = ~(columns | diagnoal | antidiagnoal | mask);
    while (avail) {
      const int i = __builtin_ctz(avail); // counting trailing zeros
      avail &= avail-1;
      if (row == N - 1) {
        ++count;
      } else {
        const uint32_t m = 1 << i;
        search(row+1,
               columns | m,
               (diagnoal | m) >> 1,
               (antidiagnoal | m) << 1);
      }
    }
  }
};

int64_t backtrackingsub(int N, int i)
{
  const int row = 0;
  const uint32_t m = 1 << i;
  BackTracking bt(N);
  bt.search(row+1, m, m >> 1, m << 1);
  return bt.count;
}

// verify task splitting
int64_t backtracking(int N)
{
  int64_t total = 0;
  for (int i = 0; i < N/2; ++i) {
    total += backtrackingsub(N, i);
  }
  total *= 2;
  if (N % 2 == 1) {
    total += backtrackingsub(N, N/2);
  }
  return total;
}

void backtracking_thr(std::atomic<int64_t>* total, int N, int i)
{
  // printf("%d %d\n", i, backtrackingsub(N, i));
  if (N % 2 == 1 && i == N / 2) {
    total->fetch_add(backtrackingsub(N, i));
  } else {
    total->fetch_add(2*backtrackingsub(N, i));
  }
}

int64_t backtracking_mt(int N)
{
  std::atomic<int64_t> total(0);
  std::vector<std::thread> threads;
  for (int i = 0; i < (N+1)/2; ++i) {
    threads.push_back(std::thread(backtracking_thr, &total, N, i));
  }

  for (auto& thr : threads) {
    thr.join();
  }
  return total;
}

int main(int argc, char* argv[])
{
  int nqueens = argc > 1 ? atoi(argv[1]) : 8;
  double start = now();
  int64_t solutions = 0;
  if (argc > 2)
  {
    if (isdigit(argv[2][0]))
    {
      int sub = strtol(argv[2], NULL, 10);
      solutions = backtrackingsub(nqueens, sub);
      printf("Solving sub problem of %d\n", sub);
    }
    else
    {
      solutions = backtracking(nqueens);
    }
  }
  else
  {
    solutions = backtracking_mt(nqueens);
  }
  double end = now();
  printf("%ld solutions of %d queens puzzle.\n", solutions, nqueens);
  printf("%f seconds.\n", end - start);
}
