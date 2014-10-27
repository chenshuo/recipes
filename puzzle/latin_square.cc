#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int N = 4;
int board[10][10];
uint32_t xmask[10], ymask[10];
int64_t count = 0;

void backtrack(int x, int y)
{
  assert(x < N && y < N);
  uint32_t mask = xmask[x] | ymask[y];
  uint32_t avail = ~mask - 1;
  while (avail)
  {
    int i = __builtin_ctz(avail); // counting trailing zeros
    avail &= avail-1;
    if (i > N)
      break;
    uint32_t needle = 1 << i;
    board[x][y] = i;
    uint32_t oldxmask = xmask[x];
    uint32_t oldymask = ymask[y];
    assert((oldxmask & needle) == 0);
    assert((oldymask & needle) == 0);
    xmask[x] |= needle;
    ymask[y] |= needle;
    if (x == N-1 && y == N-1)
      ++count;
    else if (x == N-1)
      backtrack(1, y+1);
    else
      backtrack(x+1, y);
    xmask[x] = oldxmask;
    ymask[y] = oldymask;
  }
}

void put(int x, int y, int i)
{
  board[x][y] = i;
  xmask[x] |= 1 << i;
  ymask[y] |= 1 << i;
}

int main(int argc, char* argv[])
{
  N = argc > 1 ? atoi(argv[1]) : 4;

  for (int i = 0; i < N; ++i)
  {
    put(i, 0, i+1);
    put(0, i, i+1);
  }

  backtrack(1, 1);
  printf("%zd\n", count);
}
