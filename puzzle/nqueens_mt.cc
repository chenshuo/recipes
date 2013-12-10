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
  const static int kMaxQueens = 20;
  const int N;
  int64_t count;
  bool columns[kMaxQueens];
  bool diagnoal[2*kMaxQueens], antidiagnoal[2*kMaxQueens];

  BackTracking(int nqueens)
      : N(nqueens), count(0)
  {
    assert(0 < N && N <= kMaxQueens);
    bzero(columns, sizeof columns);
    bzero(diagnoal, sizeof diagnoal);
    bzero(antidiagnoal, sizeof antidiagnoal);
  }

  void search(const int row)
  {
    for (int i = 0; i < N; ++i) {
      const int d = N + row - i;
      if (!(columns[i] || antidiagnoal[row + i] || diagnoal[d])) {
        if (row == N - 1)
          ++count;
        else {
          columns[i] = true;
          antidiagnoal[row + i] = true;
          diagnoal[d] = true;
          search(row+1);
          columns[i] = false;
          antidiagnoal[row + i] = false;
          diagnoal[d] = false;
        }
      }
    }
  }
};

int64_t backtrackingsub(int N, int i)
{
    const int row = 0;
    const int d = N + row - i;
    BackTracking bt(N);
    bt.columns[i] = true;
    bt.antidiagnoal[row + i] = true;
    bt.diagnoal[d] = true;
    bt.search(row+1);
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
  for (int i = 0; i < N/2; ++i) {
    threads.push_back(std::thread(backtracking_thr, &total, N, i));
  }
  if (N % 2 == 1) {
    threads.push_back(std::thread(backtracking_thr, &total, N, N/2));
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
  int64_t solutions = backtracking_mt(nqueens);
  double end = now();
  printf("%ld solutions of %d queens puzzle.\n", solutions, nqueens);
  printf("%f seconds.\n", end - start);
}
