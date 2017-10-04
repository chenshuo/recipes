#include "uint.h"

#include <iostream>

#include <math.h>
#include <stdio.h>

using namespace std;

void reserve()
{
  const double b = log10(2.0);
  for (int i = 0; i <= 100000; ++i)
  {
    int d = ceil(i*32*b);
    int r = 9*i + 7*i/11 + 1;
    printf("%6d %7d %7d %3d %f\n", i, d, r, r-d, double(r-d)/d);
    if (d > r)
      break;
  }
}

int main()
{
  UnsignedInt u0;
  u0.setValue(100, -1);
  cout << u0.toDec() << endl;
  u0.setValue(1000, -1);
  cout << u0.toDec() << endl;
  reserve();
  u0.setValue(1, 2);
  u0.power(1024);
  cout << u0.toDec() << endl;
  u0.setValue(1, 5);
  u0.power(1022);
  cout << u0.toDec() << endl;
}
