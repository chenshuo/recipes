#include "uint.h"

#include <algorithm>
#include <boost/static_assert.hpp>
#include <assert.h>

const char kDigits[] = "0123456789abcdef";
BOOST_STATIC_ASSERT(sizeof kDigits == 17);

void UnsignedInt::sub(const UnsignedInt& x)
{
  if (lessThan(x))
  {
    assert(0 && "Underflow");
    abort();
  }
  const value_type& rhs = x.limbs_;
  assert (rhs.size() <= limbs_.size());
  size_t len = std::min(limbs_.size(), rhs.size());
  uint64_t carry = 0;

  for (size_t i = 0; i < len; ++i)
  {
    uint64_t sum = limbs_[i] - static_cast<uint64_t>(rhs[i]) - carry;
    limbs_[i] = sum & kMask_;
    carry = sum > kMask_;
  }
  if (carry)
  {
    for (size_t i = len; i < limbs_.size() && carry; ++i)
    {
      uint64_t sum = limbs_[i] - carry;
      limbs_[i] = sum & kMask_;
      carry = sum > kMask_;
    }
  }
  if (carry)
  {
    assert(0 && "Underflow");
    abort();
  }
  while (!limbs_.empty() && limbs_.back() == 0)
    limbs_.pop_back();
}

std::string UnsignedInt::toHex() const
{
  std::string result;
  result.reserve(limbs_.size()*8);

  if (limbs_.empty())
  {
    result.push_back('0');
    return result;
  }

  assert(limbs_.size() > 0);
  for (size_t i = 0; i < limbs_.size()-1; ++i)
  {
    uint32_t x = limbs_[i];
    for (int j = 0; j < 8; ++j)
    {
      int lsd = x % 16;
      x /= 16;
      result.push_back(kDigits[lsd]);
    }
  }

  uint32_t x = limbs_.back();
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
  if (limbs_.empty())
  {
    result.push_back('0');
    return result;
  }

  result.reserve(9*limbs_.size() + 7*limbs_.size()/11 + 1);
  // log10(2**32) = 32*log10(2) = 9.633 digits per word
  UnsignedInt copy = *this;
  while (copy.limbs_.size() > 1)
  {
    uint32_t x = copy.devide(segment);
    for (int i = 0; i < 9; ++i)
    {
      int lsd = x  % 10;
      x /= 10;
      result.push_back(kDigits[lsd]);
    }
  }

  uint32_t x = copy.limbs_[0];
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
  if (r == kDec)
  {
    parseDec(x);
  }
  else if (r == kHex)
  {
    parseHex(x);
  }
  else
  {
    assert(0 && "Radix invalid");
    abort();
  }
}

uint32_t fromHex(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

void UnsignedInt::parseHex(const std::string& str)
{
  limbs_.reserve((str.size()+7) / 8);
  for (size_t i = 0; i < str.size(); ++i)
  {
    if (i % 8 == 0)
      limbs_.push_back(0);
    uint32_t digit = fromHex(str[str.size() - i - 1]);
    uint32_t shift = 4 * (i % 8);
    limbs_.back() |= (digit << shift);
  }
  if (limbs_.size() == 1 && limbs_[0] == 0)
  {
    limbs_.clear();
  }
}

uint32_t fromDec(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  return -1;
}

uint32_t parseSegment(const char* str, int len)
{
  uint32_t seg = 0;
  for (int i = 0; i < len; ++i)
  {
    seg *= 10;
    seg += fromDec(str[i]);
  }
  return seg;
}

void UnsignedInt::parseDec(const std::string& str)
{
  const uint32_t kSegment = 1000 * 1000 * 1000;
  const uint32_t kSegmentDigits = 9;
  // log2(10)/32 = 0.10381025296523
  // 8/77 = 0.103896103896104
  limbs_.reserve(1 + str.size() * 8 / 77);

  int first = str.size() % kSegmentDigits;
  if (first)
  {
    uint32_t seg = parseSegment(str.c_str(), first);
    add(seg);
  }
  for (size_t i = first; i < str.size(); i += kSegmentDigits)
  {
    assert(i + kSegmentDigits <= str.size());
    uint32_t seg = parseSegment(str.c_str() + i, kSegmentDigits);
    multiply(kSegment);
    add(seg);
  }
}
