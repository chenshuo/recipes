#include <algorithm>
#include <vector>

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
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
  const static int kMaxQueens = 20;

  const int N;
  int64_t count;
  const uint32_t sentinel;
  uint32_t columns[kMaxQueens];
  uint32_t diagnoal[kMaxQueens];
  uint32_t antidiagnoal[kMaxQueens];

  BackTracking(int nqueens)
      : N(nqueens), count(0), sentinel(1 << N)
  {
    assert(0 < N && N <= kMaxQueens);
    bzero(columns, sizeof columns);
    bzero(diagnoal, sizeof diagnoal);
    bzero(antidiagnoal, sizeof antidiagnoal);
  }

  void search(const int row)
  {
    uint32_t avail = columns[row] | diagnoal[row] | antidiagnoal[row];
    avail = ~avail;

    while (avail) {
      int i = __builtin_ctz(avail); // counting trailing zeros
      if (i >= N) {
        break;
      }
      if (row == N - 1) {
        ++count;
      } else {
        const uint32_t m = 1 << i;
        columns[row+1] = columns[row] | m;
        diagnoal[row+1] = (diagnoal[row] | m) >> 1;
        antidiagnoal[row+1] = (antidiagnoal[row] | m) << 1;
        search(row+1);
      }

      avail &= avail-1;
    }
  }
};

int64_t backtrackingsub(int N, int i)
{
    const int row = 0;
    const uint32_t m = 1 << i;
    BackTracking bt(N);
    bt.columns[row+1] = m;
    bt.diagnoal[row+1] = m >> 1;
    bt.antidiagnoal[row+1] = m << 1;
    bt.search(row+1);
    return bt.count;
}

int backtracking(int N)
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

int main(int argc, char* argv[])
{
  int nqueens = argc > 1 ? atoi(argv[1]) : 8;
  double start = now();
  int64_t solutions = backtracking(nqueens);
  double end = now();
  printf("%ld solutions of %d queens puzzle.\n", solutions, nqueens);
  printf("%f seconds.\n", end - start);
}
