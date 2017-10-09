#include "uint.h"

#include <float.h>
#include <stdio.h>
#include <iostream>
#include <ios>

// convert a floating point number to exact decimal number
class Float
{
 public:
  Float(int exponent, uint64_t mantissa)
    : E(exponent), M(mantissa)
  {
  }

  std::string decimal() const
  {
    std::string dec = integer();
    std::pair<std::string, int> frac = fraction();
    dec += '.';
    dec.append(frac.second, '0');
    dec.append(frac.first);
    return dec;
  }

  static Float fromDouble(double x)
  {
    union
    {
      double f;
      uint64_t u;
    };
    f = x;
    // CAUTION: only works on 64-bit platforms?
    return fromDouble((u & 0x7ff0000000000000) >> 52, u & 0xFFFFFFFFFFFFF);
  }

  static Float fromDouble(int exponent, uint64_t fraction)
  {
    // FIXME: subnormal
    return Float(exponent-1023-52, fraction | 0x10000000000000);
  }

 private:

  std::string integer() const
  {
    int e = E;
    uint64_t m = M;
    while (e < 0)
    {
      m >>= 1;
      ++e;
    }
    uint32_t lo = m & 0xFFFFFFFF;
    uint32_t hi = m >> 32;
    UnsignedInt u(lo);
    if (hi)
    {
      UnsignedInt x(hi);
      x.multiply(0x10000);
      x.multiply(0x10000);
      u.add(x);
    }
    if (e)
    {
      UnsignedInt y(2);
      y.power(e);
      u.multiply(y);
    }
    return u.toDec();
  }

  // 0.1111111111
  //   ||||||||||
  // 0.5|||||||||
  // 0.25||||||||
  // 0.125|||||||
  // 0.0625||||||
  // 0.03125|||||
  // 0.015625||||
  // 0.0078125|||
  // 0.00390625||
  // 0.001953125|
  // 0.0009765625
  std::pair<std::string, int> fraction() const
  {
    int e = E;
    uint64_t m = M;
    UnsignedInt f(0);
    UnsignedInt d(5);
    while (e < 0 && (m & 1) == 0)
    {
      m >>= 1;
      ++e;
    }
    const int digits = (e < 0) ? -e : 0;
    while (e < 0)
    {
      f.multiply(5);
      if (m & 1)
      {
        f.add(d);
      }
      m >>= 1;
      ++e;
      d.multiply(10);
    }
    std::string x = f.toDec();
    int zeros = x.size() < digits ? digits - x.size() : 0;
    return std::make_pair(x, zeros);
  }

 private:
  const int E;
  const uint64_t M;
};

using namespace std;

int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    for (int i = 1; i < argc; ++i)
    {
      double x = strtod(argv[i], NULL);
      printf("%s = %a\n", argv[i], x);
      cout << Float::fromDouble(x).decimal() << endl;
      if (i != argc - 1)
	printf("\n");
    }
  }
  else
  {
    Float f1 = Float::fromDouble(1);
    cout << f1.decimal() << endl;
    Float pi = Float::fromDouble(0x400, 0x921fb54442d18);
    cout << pi.decimal() << endl;
    Float f3 = Float::fromDouble(0x3fd, 0x5555555555555);
    cout << f3.decimal() << endl;
    Float f01 = Float::fromDouble(0.1);
    cout << f01.decimal() << endl;
    Float fy(1023, 2);  // 2 ** 1024
    cout << fy.decimal() << endl;

    assert(strtod("0x1.FffffFFFFffffP1023", NULL) == DBL_MAX);
    Float fx = Float::fromDouble(DBL_MAX);
    cout << fx.decimal() << endl;
    Float fm = Float::fromDouble(DBL_MIN);
    cout << fm.decimal() << endl;
    Float fe = Float::fromDouble(DBL_EPSILON);
    cout << fe.decimal() << endl;
    printf("%a\n", DBL_MAX);
    printf("%a\n", DBL_MIN);
    printf("%a\n", DBL_EPSILON);
  }
}

