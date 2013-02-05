#include "uint.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    const int N = atoi(argv[1]);
    UnsignedInt result(1);
    for (int i = 1; i <= N; ++i)
    {
      result.multiply(i);
    }
    printf("%s\n", result.toDec().c_str());
  }
  else
  {
    printf("Usage: %s number\n", argv[0]);
  }
}
