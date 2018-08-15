#include <stdio.h>
#include <random>

// generate a 10G input of 1B entries (472M unique), top entries are:
/*
31      3209505773
29      2131583786
29      2912700161
29      494155588
29      244318914
29      3795640459
29      3736353633
29      3792368229
28      2134003008
28      3755101979
28      2615920895
28      4103497323
28      124613411
28      3877714427
*/ 
int main()
{
  int iter = 0;
  for (int x = 500 * 1000 * 1000; x > 0; x /= 2)
  {
    std::mt19937 gen(43);
    for (int i = 0; i < x; ++i)
      printf("%lu\n", gen());
    ++iter;
  }
  printf("%d\n", iter);
}
