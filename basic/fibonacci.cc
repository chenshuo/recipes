// Set skip_to_string = true, N = 1,000,000 measured on i7-7600U laptop
//   linear : 11.45  s
//   fast   :  0.639 s
//   faster :  0.437 s
//   fastest:  0.215 s

#include "uint.h"

#include <unordered_map>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

const bool skip_to_string = false;

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
  return skip_to_string ? "" : result->toDec();
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

    // http://en.wikipedia.org/wiki/Exponentiation_by_squaring#Runtime_example:_compute_310_2
    for (bit >>= 1; bit > 0; bit >>= 1)
    {
      // A = A * A
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
        // A = [ 1 1; 1 0] * A
        n0.swap(n1);
        n1 = n2;
        n2.add(n0);
      }
    }
  }
  // printf("%f\n", now() - start);
  return skip_to_string ? "" : result->toDec();
}

// f(2n) = f(n) * (f(n-1) + f(n+1))
// f(2n+1) = f(n)^2 + f(n+1)^2
class Fib
{
 public:
  explicit Fib(int N)
  {
    cache_[0] = UnsignedInt(0);
    cache_[1] = UnsignedInt(1);
    cache_[2] = UnsignedInt(1);
    /*
    cache_[3] = UnsignedInt(2);
    cache_[4] = UnsignedInt(3);
    cache_[5] = UnsignedInt(5);
    cache_[6] = UnsignedInt(8);
    cache_[7] = UnsignedInt(13);
    cache_[8] = UnsignedInt(21);
    cache_[9] = UnsignedInt(34);
    */
    calc(N);
    result_ = &cache_[N];
  }

  const UnsignedInt& get() const
  {
    return *result_;
  }

 private:
  void calc(int N)
  {
    assert(N >= 0);
    auto it = cache_.find(N);
    if (it == cache_.end())
    {
      calc(N/2);
      calc(N/2 + 1);
      if (N & 1)
      {
        UnsignedInt a = cache_[N/2];
        a.multiply(a);
        UnsignedInt b = cache_[N/2 + 1];
        b.multiply(b);
        a.add(b);
        cache_[N] = std::move(a);
      }
      else
      {
        calc(N/2 - 1);
        UnsignedInt a = cache_[N/2 - 1];
        a.add(cache_[N/2 + 1]);
        a.multiply(cache_[N/2]);
        cache_[N] = std::move(a);
      }
    }
  }

  std::unordered_map<int, UnsignedInt> cache_;
  const UnsignedInt* result_ = nullptr;
};

std::string fastestFibonacci(int N)
{
  Fib f(N);
  return skip_to_string ? "" : f.get().toDec();
}

class FibonacciIterator
{
 public:
  std::string operator*() const
  {
    return fn.toDec();
  }

  FibonacciIterator& operator++()
  {
    prev.swap(fn);
    fn.add(prev);
    return *this;
  }

 private:
  UnsignedInt fn = 1, prev = 0;
};

std::string linearFibonacci(int N)
{
  assert(N >= 0);
  // double start = now();
  FibonacciIterator iter;  // F(1)
  for (int i = 1; i < N; ++i)
  {
    ++iter;
  }
  //printf("%f\n", now() - start);
  return skip_to_string ? "" : (N == 0 ? "0" : *iter);
};

int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    const int N = atoi(argv[1]);
    std::string result;
    if (argc > 2)
    {
      if (argv[2][0] == 'F')
      {
        result = fastestFibonacci(N);
      }
      else if (argv[2][0] == 'f')
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
    printf("Usage: %s number [fFx]\n", argv[0]);
    FibonacciIterator iter;
    for (int i = 1; i < 10000; ++i)
    {
      if (fastFibonacci(i) != *iter || fasterFibonacci(i) != *iter || fastestFibonacci(i) != *iter)
      {
        printf("ERROR %d\n", i);
      }
      ++iter;
    }
  }
}

