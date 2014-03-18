#include <vector>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  size_t N = argc > 1 ? atol(argv[1]) : 100;
  int count = 1;
  std::vector<bool> sieve(N, true);
  printf("2\n");
  for (size_t i = 1; i < sieve.size(); ++i)
  {
    if (sieve[i])
    {
      size_t p = 2*i + 1;
      ++count;
      printf("%zd\n", p);
      size_t q = 3 * p;
      size_t j = (q-1) / 2;
      while (j < sieve.size())
      {
        sieve[j] = false;
        q += 2 * p;
        j = (q-1) / 2;
      }
    }
  }
  //printf("count = %d\n", count);
}
