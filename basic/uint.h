#include <algorithm>
#include <string>
#include <vector>
#include <ext/numeric>
#include <assert.h>
#include <stdint.h>

class UnsignedInt // copyable
{
 public:
  enum Radix
  {
    kDec,
    kHex,
  };

  UnsignedInt(uint32_t x = 0)
  {
    if (x > 0)
    {
      limbs_.push_back(x);
    }
  }

  UnsignedInt(const std::string& x, Radix r = kDec);

  std::string toHex() const;
  std::string toDec() const;
  bool isZero() const { return limbs_.empty(); }
  bool isNormal() const { return isZero() || limbs_.back() != 0; }
  bool lessThan(const UnsignedInt& x) const
  {
    const value_type& rhs = x.limbs_;
    return rhs.size() > limbs_.size() ||
           (rhs.size() == limbs_.size() &&
            std::lexicographical_compare(limbs_.rbegin(),
                                         limbs_.rend(),
                                         rhs.rbegin(),
                                         rhs.rend()));
  }

  void swap(UnsignedInt& rhs) { limbs_.swap(rhs.limbs_); }

  void assign(const uint32_t x)
  {
    if (x == 0)
    {
      limbs_.clear();
    }
    else
    {
      limbs_.resize(1);
      limbs_[0] = x;
    }
  }

  void add(const uint32_t x)
  {
    if (limbs_.empty())
    {
      limbs_.push_back(x);
      return;
    }

    uint64_t sum = limbs_[0] + static_cast<uint64_t>(x);
    limbs_[0] = sum & kMask_;
    uint64_t carry = sum > kMask_;

    for (size_t i = 1; i < limbs_.size() && carry; ++i)
    {
      sum = limbs_[i] + carry;
      limbs_[i] = sum & kMask_;
      carry = sum > kMask_;
    }

    if (carry)
    {
      limbs_.push_back(carry);
    }
  }

  void add(const UnsignedInt& x)
  {
    const value_type& rhs = x.limbs_;
    if (rhs.size() > limbs_.size())
    {
      limbs_.resize(rhs.size());
    }
    size_t len = std::min(limbs_.size(), rhs.size());
    uint64_t carry = 0;
    for (size_t i = 0; i < len; ++i)
    {
      uint64_t sum = limbs_[i] + static_cast<uint64_t>(rhs[i]) + carry;
      limbs_[i] = sum & kMask_;
      carry = sum > kMask_;
    }
    if (carry)
    {
      for (size_t i = len; i < limbs_.size() && carry; ++i)
      {
        uint64_t sum = limbs_[i] + carry;
        limbs_[i] = sum & kMask_;
        carry = sum > kMask_;
      }
    }
    if (carry)
    {
      limbs_.push_back(carry);
    }
  }

  void sub(const UnsignedInt& x);

  void multiply(const uint32_t x)
  {
    const uint64_t multiplier = x;
    if (multiplier == 0)
    {
      limbs_.clear();
      return;
    }
    uint64_t carry = 0;
    for (size_t i = 0; i < limbs_.size(); ++i)
    {
      uint64_t prod = limbs_[i] * multiplier + carry;
      limbs_[i] = prod & kMask_;
      carry = prod / 0x10000 / 0x10000;
    }
    if (carry)
    {
      limbs_.push_back(carry);
    }
  }

  void multiply(const UnsignedInt& x)
  {
    const value_type& rhs = x.limbs_;
    if (limbs_.empty() || rhs.empty())
    {
      limbs_.clear();
      return;
    }
    value_type result(limbs_.size() + rhs.size());
    for (size_t i = 0; i < rhs.size(); ++i)
    {
      if (rhs[i] == 0)
        continue;
      uint64_t carry = 0;
      for (size_t j = 0; j < limbs_.size(); ++j)
      {
        uint64_t prod = uint64_t(rhs[i]) * limbs_[j]
                        + result[i+j]
                        + carry;
        result[i+j] = prod & kMask_;
        carry = prod / 0x10000 / 0x10000;
      }
      result[i + limbs_.size()] = carry;
    }
    if (result.back() == 0)
      result.pop_back();
    result.swap(limbs_);
  }

  // returns remainder
  uint32_t devide(const uint32_t x)
  {
    uint64_t carry = 0;
    for (size_t i = limbs_.size(); i > 0; --i)
    {
      uint64_t prod = carry << 32;
      prod += limbs_[i-1];

      limbs_[i-1] = prod / x;
      carry = prod % x;
    }

    if (!limbs_.empty() && limbs_.back() == 0)
      limbs_.pop_back();
    assert(isNormal());
    return carry;
  }

  // pow(this, n)
  void power(uint32_t n)
  {
    if (n == 0)
    {
      assign(1);
    }
    else if (n > 1)
    {
      // FIXME: if n is a power of 2, we can do this in-place.
      UnsignedInt result(1);
      while (n)
      {
        if (n & 1)
        {
          result.multiply(*this);
        }
        multiply(*this);
        n /= 2;
      }
      swap(result);
    }
  }

  typedef std::vector<uint32_t> value_type;

  const value_type& getValue() const
  {
    return limbs_;
  }

  void setValue(int n, uint32_t x)
  {
    limbs_.assign(n, x);
  }

 private:
  void parseDec(const std::string& str);
  void parseHex(const std::string& str);

  uint32_t highest() const
  {
    return limbs_.empty() ? 0 : limbs_.back();
  }

  value_type limbs_;

  const static uint32_t kMask_ = 0xFFFFFFFF;
};
