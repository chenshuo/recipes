// g++ -Wall fibonacci.cc -O2 -lgmp -o fib

// Set skip_get_str = true, N = 1,000,000 measured on i7-7600U laptop
//   linear :  2.60 s
//   fast   : 19.09 ms
//   faster :  8.12 ms
//   fastest:  3.73 ms

#include <unordered_map>
#include <assert.h>
#include <gmpxx.h>
#include <sys/time.h>

const bool skip_get_str = false;

double now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

class FibonacciIterator
{
 public:
  std::string operator*() const
  {
    return fn.get_str();
  }

  FibonacciIterator& operator++()
  {
    prev.swap(fn);
    fn += prev;
    return *this;
  }

 private:
  mpz_class fn = 1, prev = 0;
};

std::string linearFibonacci(int N)
{
  assert(N >= 1);
  FibonacciIterator iter; // F(1)
  for (int i = 1; i < N; ++i)
  {
    ++iter;
  }

  return skip_get_str ? "" : *iter;
}

// [ a  b ]
// [ c  d ]
struct Matrix
{
  Matrix(int x11, int x12, int x21, int x22)
    : a(x11), b(x12), c(x21), d(x22)
  {
  }

  void operator*=(const Matrix& rhs)
  {
    mpz_class x11 = a * rhs.a + b * rhs.c;
    mpz_class x12 = a * rhs.b + b * rhs.d;
    mpz_class x21 = c * rhs.a + d * rhs.c;
    mpz_class x22 = c * rhs.b + d * rhs.d;
    a.swap(x11);
    b.swap(x12);
    c.swap(x21);
    d.swap(x22);
  }

  void square()
  {
    operator*=(*this);
    // TODO: there is one slight faster square algorithm, saving 1 multiplication by reusing b*c.
  }

  mpz_class a, b, c, d;
};

/*
 [ 1  1 ] ^ n   [F(n+1)  F(n)  ]
 [ 1  0 ]     = [F(n)    F(n-1)]
 */
std::string fastFibonacci(int N)
{
  assert(N >= 1);
  Matrix base(1, 1, 1, 0), result(1, 0, 0, 1);
  int n = N-1;
  while (n > 1)
  {
    if (n % 2 == 1)
    {
      result *= base;
    }
    base.square();
    n /= 2;
  }
  result *= base;  // TODO: only need to calculate 'a'
  return skip_get_str ? "" : result.a.get_str();
}

/*
  consider symmetry, the Matrix [a b; c d] has only
  three independent elements: a b c, because b==c.
 */
std::string fasterFibonacci(int N)
{
  assert(N >= 1);
  mpz_class a(1), b(1), d(0), x(1), y(0), z(1), tmp;
  int n = N-1;
  while (n > 1)
  {
    if (n % 2 == 1)
    {
      // [ x  y ]    [ a  b ]
      // [ y  z ] *= [ b  d ]
      tmp = y*b;
      z = tmp + z*d;
      y = x*b + y*d;
      x = x*a + tmp;
    }
    tmp = b*b;
    b = a*b + b*d;
    d = tmp + d*d;
    a = a*a + tmp;
    n /= 2;
  }
  tmp = x*a + y*b;
  return skip_get_str ? "" : tmp.get_str();
}

// f(2n) = f(n) * (f(n-1) + f(n+1))
// f(2n+1) = f(n)^2 + f(n+1)^2
class Fib
{
 public:
  explicit Fib(int N)
  {
    cache_[0] = 0;
    cache_[1] = 1;
    cache_[2] = 1;
    /*
    cache_[3] = 2;
    cache_[4] = 3;
    cache_[5] = 5;
    cache_[6] = 8;
    cache_[7] = 13;
    cache_[8] = 21;
    cache_[9] = 34;
    */
    calc(N);
    result_ = &cache_[N];
  }

  const mpz_class& get() const
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
        const mpz_class& a = cache_[N/2];
        const mpz_class& b = cache_[N/2 + 1];
        cache_[N] = a*a + b*b;
      }
      else
      {
        calc(N/2 - 1);
        const mpz_class& a = cache_[N/2 - 1];
        const mpz_class& b = cache_[N/2 + 1];
        cache_[N] = cache_[N/2] * (a + b);
      }
    }
  }

  std::unordered_map<int, mpz_class> cache_;
  const mpz_class* result_ = nullptr;
};

std::string fastestFibonacci(int N)
{
  Fib f(N);
  return skip_get_str ? "" : f.get().get_str();
}

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
        break;
      }
      ++iter;
    }
  }
}

