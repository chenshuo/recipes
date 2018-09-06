// g++ -Wall fibonacci.cc -O2 -lgmp -o fib

// Set skip_get_str = true, N = 1,000,000 measured on i7-7600U laptop
//   linear :  2.60 s
//   fast   : 18.73 ms
//   faster :  8.12 ms
//   fastest:  2.78 ms

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
    cache_[3] = 2;
    /*
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
      if (N & 1)
      {
        calc(N/2);
        calc(N/2 + 1);
        const mpz_class& a = cache_[N/2];
        const mpz_class& b = cache_[N/2 + 1];
        cache_[N] = a*a + b*b;
      }
      else
      {
        calc(N/2 - 1);
        calc(N/2);
        const mpz_class& a = cache_[N/2 - 1];
        const mpz_class& b = cache_[N/2];
        cache_[N/2 + 1] = a + b;
        const mpz_class& c = cache_[N/2 + 1];
        cache_[N] = b * (a + c);
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

std::string floatFibonacci(int N, int prec)
{
  // prec / N = log2(1+sqrt(5)) - 1 = 0.694242
  mpf_class a(5, prec), b;
  mpf_class s5 = sqrt(a);
  assert(s5.get_prec() == prec);
  a = (1 + s5) / 2;
  b = (1 - s5) / 2;
  mpf_pow_ui(a.get_mpf_t(), a.get_mpf_t(), N);
  mpf_pow_ui(b.get_mpf_t(), b.get_mpf_t(), N);
  mpf_class c = s5 / 5 * (a - b) + 0.5;
  assert(c.get_prec() == prec);
  mp_exp_t expo;
  std::string str = c.get_str(expo);
  return str.substr(0, expo);
}

int get_precision(int N, int prec, const std::string& expected)
{
  while (floatFibonacci(N, prec) != expected)
  {
    prec += 64;
  }
  return prec;
}

void find_precision()
{
  FibonacciIterator iter;
  int prec = 64;
  for (int i = 1; i <= 50000; ++i)
  {
    int newprec = get_precision(i, prec, *iter);
    bool print = (i % 10000 == 0) || newprec != prec;
    prec = newprec;
    if (print)
      printf("%d %d %.5f\n", i, prec, double(prec) / i);
    ++iter;
  }
}

int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    const int N = atoi(argv[1]);
    std::string result;
    double start = now();
    const int kRepeat = 100;
    for (int i = 0; i < (skip_get_str ? kRepeat : 1); ++i)
    {
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
    }
    if (skip_get_str)
    {
      printf("%.3f ms\n", (now() - start) * 1000.0 / kRepeat);
    }
    else
    {
      printf("%s\n", result.c_str());
    }
  }
  else
  {
    printf("Usage: %s number [fFx]\n", argv[0]);
    FibonacciIterator iter;
    for (int i = 1; i < 10000; ++i)
    {
      if (fastFibonacci(i) != *iter ||
          fasterFibonacci(i) != *iter ||
          fastestFibonacci(i) != *iter ||
          floatFibonacci(i, 6976) != *iter)
      {
        printf("ERROR %d\n", i);
        break;
      }
      ++iter;
    }
    // find_precision();
    // floatFibonacci(1000000, 694272);
  }
}

