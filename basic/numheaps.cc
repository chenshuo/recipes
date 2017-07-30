// http://oeis.org/A056971

#include "uint.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <unordered_map>

UnsignedInt combination(int N, int M)
{
  // printf("Choose N=%d M=%d\n", N, M);
  assert(0 <= M && M <= N);
  if (M > N/2)
    M = N - M;
  UnsignedInt result(1);
  for (int i = 1; i <= M; ++i)
  {
    result.multiply(N - i + 1);
    int r = result.devide(i);
    assert(r == 0);
    if (r != 0)
      abort();
  }
  return result;
}

std::unordered_map<int, UnsignedInt> table = {
  { 0, 1 },
  { 1, 1 },
};

const UnsignedInt& num_heaps(int N)
{
  UnsignedInt& result = table[N];
  if (result.isZero())
  {
    int h = log2(N);
    int left = std::min(pow(2, h)-1, N - pow(2,h-1));
    result = combination(N-1, left);
    result.multiply(num_heaps(left));
    int right = N - 1 - left;
    result.multiply(num_heaps(right));
  }
  else
  {
    // printf("hit %d\n", N);
  }
  return result;
}

int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    UnsignedInt result = num_heaps(atoi(argv[1]));
    printf("%s\n", result.toDec().c_str());
  }
  else
  {
    printf("Usage: %s N\n", argv[0]);
    printf("Number of binary heaps on N elements.\n");
  }
}

