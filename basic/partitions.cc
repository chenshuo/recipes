#include "uint.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

std::vector<int> generalized_pentagonal_numbers;

int gpn(int m)
{
  return m * (3 * m - 1) / 2;
}

void gen(int N)
{
  int m = 1;
  do {
    generalized_pentagonal_numbers.push_back(gpn(m));
    generalized_pentagonal_numbers.push_back(gpn(-m));
    ++m;
  } while (generalized_pentagonal_numbers.back() < N);
}


int main(int argc, char* argv[])
{
  const int N = atoi(argv[1]);
  gen(N);
  // printf("%zd\n", generalized_pentagonal_numbers.size());
  // printf("%d\n", generalized_pentagonal_numbers.back());

  std::vector<UnsignedInt> partitions;
  partitions.resize(N+1);
  partitions[0].add(1);

  for (int k = 1; k <= N; ++k)
  {
    UnsignedInt& result = partitions[k];
    assert(result.isZero());
    for (size_t i = 0; i < generalized_pentagonal_numbers.size(); ++i)
    {
      int x = generalized_pentagonal_numbers[i];
      if (k - x >= 0)
      {
        UnsignedInt& term = partitions[k - x];
        assert(!term.isZero());
        if ((i / 2) % 2 == 0)
        {
          result.add(term);
        }
        else
        {
          result.sub(term);
        }
      }
      else
      {
        break;
      }
    }
  }
  printf("%s\n", partitions[N].toDec().c_str());

  // for (size_t i = 0; i < partitions.size(); ++i)
  // {
  //   printf(" %d", partitions[i].isZero());
  // }
}
