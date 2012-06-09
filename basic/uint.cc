#include "uint.h"

#include <algorithm>
#include <boost/static_assert.hpp>
#include <assert.h>

const char kDigits[] = "0123456789abcdef";
BOOST_STATIC_ASSERT(sizeof kDigits == 17);

std::string UnsignedInt::toHex() const
{
  std::string result;
  result.reserve(value_.size()*8);

  if (value_.empty())
  {
    result.push_back('0');
    return result;
  }

  assert(value_.size() > 0);
  for (size_t i = 0; i < value_.size()-1; ++i)
  {
    uint32_t x = value_[i];
    for (int j = 0; j < 8; ++j)
    {
      int lsd = x % 16;
      x /= 16;
      result.push_back(kDigits[lsd]);
    }
  }

  uint32_t x = value_.back();
  do
  {
    int lsd = x % 16;
    x /= 16;
    result.push_back(kDigits[lsd]);
  } while (x != 0);

  std::reverse(result.begin(), result.end());

  return result;
}

std::string UnsignedInt::toDec() const
{
  const uint32_t segment = 1000000000;
  std::string result;
  if (value_.empty())
  {
    result.push_back('0');
    return result;
  }

  result.reserve(9*value_.size() + 7*value_.size()/11 + 1);
  // log10(2**32) = 32*log10(2) = 9.633 digits per word
  UnsignedInt copy = *this;
  while (copy.value_.size() > 1)
  {
    uint32_t x = copy.devide(segment);
    for (int i = 0; i < 9; ++i)
    {
      int lsd = x  % 10;
      x /= 10;
      result.push_back(kDigits[lsd]);
    }
  }

  uint32_t x = copy.value_[0];
  do
  {
    int lsd = x % 10;
    x /= 10;
    result.push_back(kDigits[lsd]);
  } while (x != 0);

  std::reverse(result.begin(), result.end());

  return result;
}

UnsignedInt::UnsignedInt(const std::string& x, Radix r)
{
  (void)x;(void)r;
}
