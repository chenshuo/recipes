#include <algorithm>
#include <numeric>
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

int bruteforce(const int N)
{
  int count = 0;
  std::vector<int> pos(N);
  std::iota(pos.begin(), pos.end(), 0);
  do {
    bool diagnoal[2*N];
    bzero(diagnoal, sizeof(diagnoal));
    bool antidiagnoal[2*N];
    bzero(antidiagnoal, sizeof(antidiagnoal));

    for (int i = 0; i < N; ++i) {
      if (antidiagnoal[i + pos[i]]) {
        goto infeasible;
      } else {
        antidiagnoal[i + pos[i]] = true;
      }
    // }

    // for (int i = 0; i < N; ++i) {
      const int d = N + i - pos[i];
      if (diagnoal[d]) {
        goto infeasible;
      } else {
        diagnoal[d] = true;
      }
    }

    ++count;
infeasible:
    ;
  } while (next_permutation(pos.begin(), pos.end()));
  return count;
}

struct BackTracking
{
  const static int kMaxQueens = 16;
  const int N;
  int count;
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
        if (row == N - 1) {
          ++count;
        }
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

int backtracking(int N)
{
  BackTracking bt(N);
  bt.search(0);
  return bt.count;
}

int main(int argc, char* argv[])
{
  int nqueens = argc > 1 ? atoi(argv[1]) : 8;
  double start = now();
  int solutions = backtracking(nqueens);
  double end = now();
  printf("%d solutions of %d queens puzzle.\n", solutions, nqueens);
  printf("%f seconds.\n", end - start);

  /*
  double start = now();
  int s1 = bruteforce(nqueens);
  double middle = now();
  int s2 = backtracking(nqueens);
  double end = now();
  printf("brute force %d, backtracking %d\n", s1, s2);
  printf("brute force  %f\nbacktracking %f\n", middle - start, end - middle);
  */
}
