#include "uint.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  if (argc > 2)
  {
    int M = atoi(argv[1]);
    const int N = atoi(argv[2]);
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
    printf("%s\n", result.toDec().c_str());
  }
  else
  {
    printf("Usage: %s M N\n", argv[0]);
    printf("Choose M elements out of N.\n");
  }
}
