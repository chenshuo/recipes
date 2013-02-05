#include "uint.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

double now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

std::string fastFibonacci(int N)
{
  assert (N >= 0);
  // double start = now();
  UnsignedInt n0(0), n1(1), n2(1);
  UnsignedInt* result = &n1;
  UnsignedInt t0(0), t1(1), t2(1), t3;
  UnsignedInt s1, s2;
  if (N == 0)
  {
    result = &n0;
  }
  else if (N >= 3)
  {
    if (N % 2 == 0)
    {
      result = &n0;
    }

    while (N >>= 1)
    {
      // A := A * A
      t3 = t2;
      t3.add(t0);
      t3.multiply(t1);

      t0.multiply(t0);
      t1.multiply(t1);
      t2.multiply(t2);

      t2.add(t1);
      t0.add(t1);
      t1.swap(t3);

      if (N % 2)
      {
        // TODO: find faster method
        // N := A * N
        s1 = n1;
        s1.multiply(t0);

        s2 = n2;
        s2.multiply(t1);
        s2.add(s1);

        n1.multiply(t1);

        n2.multiply(t2);
        n2.add(n1);

        n0.multiply(t0);
        n0.add(n1);

        n1.swap(s2);
      }
    }
  }
  // printf("%f\n", now() - start);
  return result->toDec();
};

std::string fasterFibonacci(int N)
{
  // double start = now();
  UnsignedInt n0(0), n1(1), n2(1);
  UnsignedInt* result = &n1;
  UnsignedInt t;

  if (N == 0)
  {
    result = &n0;
  }
  else if (N >= 3)
  {
    int bit = 1;
    while (bit <= N)
      bit <<= 1;
    bit >>= 1;
    assert((bit & N) > 0);

    for (bit >>= 1; bit > 0; bit >>= 1)
    {
      t = n2;
      t.add(n0);
      t.multiply(n1);

      n0.multiply(n0);
      n1.multiply(n1);
      n2.multiply(n2);

      n2.add(n1);
      n0.add(n1);
      n1.swap(t);

      if (bit & N)
      {
        n0.swap(n1);
        n1 = n2;
        n2.add(n0);
      }
    }
  }
  // printf("%f\n", now() - start);
  return result->toDec();
}

std::string linearFibonacci(int N)
{
  assert (N >= 0);
  // double start = now();
  UnsignedInt n0(0);
  UnsignedInt n1(1);
  UnsignedInt* result = NULL;
  if (N == 0)
  {
    result = &n0;
  }
  else if (N == 1)
  {
    result = &n1;
  }
  else // (N >= 2)
  {
    for (int i = 2; i <= N; ++i)
    {
      n0.swap(n1);
      n1.add(n0);
    }
    result = &n1;
  }

  assert(result != NULL);
  //printf("%f\n", now() - start);
  return result->toDec();
};

int main(int argc, char* argv[])
{
  /*
  for (int i = 0; i < 100000; ++i)
  {
    std::string result = linearFibonacci(i);
    if (fastFibonacci(i) != result || fasterFibonacci(i) != result)
    {
      printf("ERROR %d\n", i);
    }
  }
  */

  if (argc > 1)
  {
    const int N = atoi(argv[1]);
    std::string result;
    if (argc > 2)
    {
      if (argv[2][0] == 'F')
      {
        result = fasterFibonacci(N);
      }
      else
      {
        result = fastFibonacci(N);
      }
    }
    else
    {
      result = linearFibonacci(N);
    }
    printf("%s\n", result.c_str());
  }
  else
  {
    printf("Usage: %s number [fF]\n", argv[0]);
  }
}

