#pragma once

#include <stdint.h>

// __int128 is a built-in type on amd64, though.

struct uint128
{
  uint64_t low, high;

  uint128(int32_t l)
      : low(l), high(l < 0 ? -1 : 0)
  { }

  uint128(uint64_t l = 0)
      : low(l), high(0)
  { }

  uint128(uint64_t h, uint64_t l)
      : low(l), high(h)
  { }

  uint128& add(uint128 rhs)
  {
#if __x86_64__
    asm ("addq\t%2, %0\n\t"
         "adcq\t%3, %1"
         : "+r"(low), "+r"(high)
         : "r"(rhs.low), "r"(rhs.high)
         );
#else
    // http://stackoverflow.com/questions/6659414/efficient-128-bit-addition-using-carry-flag
    low += rhs.low;
    high += rhs.high + (low < rhs.low);
    // GCC needs to fix http://gcc.gnu.org/bugzilla/show_bug.cgi?id=51839
#endif
    return *this;
  }

  uint128& multiply(uint64_t rhs)
  {
    uint64_t h = high * rhs;
    *this = multiply128(low, rhs);
    high += h;
    return *this;
  }

  uint128& multiply(uint128 rhs) // __attribute__ ((noinline))
  {
    uint64_t h = low * rhs.high + high * rhs.low;
    *this = multiply128(low, rhs.low);
    high += h;
    return *this;
  }

  static uint128 multiply128(uint64_t x, uint64_t y) // __attribute__ ((noinline))
  {
#if __x86_64__
    uint128 prod(x);
    // MULX is only available in Haswell and later, use old MUL
    // RAX = prod.low   # in-out
    // MULQ y           # RDX:RAX = RAX * y
    // prod.high = RDX  # out
    asm ("mulq\t%2\n\t"
         : "+a"(prod.low), "=d"(prod.high)
         : "r"(y)
         );
#else
    uint32_t a = x & 0xFFFFFFFF;
    uint32_t c = x >> 32;
    uint32_t b = y & 0xFFFFFFFF;
    uint32_t d = y >> 32;
    uint64_t ab = (uint64_t)a * b;
    uint64_t bc = (uint64_t)b * c;
    uint64_t ad = (uint64_t)a * d;
    uint64_t cd = (uint64_t)c * d;
    uint64_t low = ab + (bc << 32);
    uint64_t high = cd + (bc >> 32) + (ad >> 32) + (low < ab);
    low += (ad << 32);
    high += (low < (ad << 32));
    uint128 prod(high, low);
#endif
    return prod;
  }
} __attribute__ ((aligned (16)));

