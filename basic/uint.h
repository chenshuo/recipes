#include <string>
#include <vector>
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
      value_.push_back(x);
    }
  }

  UnsignedInt(const std::string& x, Radix r = kDec);

  std::string toHex() const;
  std::string toDec() const;

  void swap(UnsignedInt& rhs) { value_.swap(rhs.value_); }

  void add(const uint32_t x)
  {
    if (value_.empty())
    {
      value_.push_back(x);
      return;
    }

    uint64_t sum = value_[0] + static_cast<uint64_t>(x);
    value_[0] = sum & kMask_;
    uint64_t carry = sum > kMask_;

    for (size_t i = 1; i < value_.size() && carry; ++i)
    {
      sum = value_[i] + carry;
      value_[i] = sum & kMask_;
      carry = sum > kMask_;
    }

    if (carry)
    {
      value_.push_back(carry);
    }
  }

  void add(const UnsignedInt& x)
  {
    const value_type& rhs = x.value_;
    if (rhs.size() > value_.size())
    {
      value_.resize(rhs.size());
    }
    size_t len = std::min(value_.size(), rhs.size());
    uint64_t carry = 0;
    for (size_t i = 0; i < len; ++i)
    {
      uint64_t sum = value_[i] + static_cast<uint64_t>(rhs[i]) + carry;
      value_[i] = sum & kMask_;
      carry = sum > kMask_;
    }
    if (carry)
    {
      for (size_t i = len; i < value_.size() && carry; ++i)
      {
        uint64_t sum = value_[i] + carry;
        value_[i] = sum & kMask_;
        carry = sum > kMask_;
      }
    }
    if (carry)
    {
      value_.push_back(carry);
    }
  }

  void multiply(const uint32_t x)
  {
    const uint64_t multiplier = x;
    uint64_t carry = 0;
    for (size_t i = 0; i < value_.size(); ++i)
    {
      uint64_t prod = value_[i] * multiplier + carry;
      value_[i] = prod & kMask_;
      carry = prod / 0x10000 / 0x10000;
    }
    if (carry)
    {
      value_.push_back(carry);
    }
  }

  void multiply(const UnsignedInt& x)
  {
    const value_type& rhs = x.value_;
    if (value_.empty() || rhs.empty())
    {
      value_.clear();
      return;
    }
    value_type result(value_.size() + rhs.size());
    for (size_t i = 0; i < rhs.size(); ++i)
    {
      if (rhs[i] == 0)
        continue;
      uint64_t carry = 0;
      for (size_t j = 0; j < value_.size(); ++j)
      {
        uint64_t prod = uint64_t(rhs[i]) * value_[j]
                        + result[i+j]
                        + carry;
        result[i+j] = prod & kMask_;
        carry = prod / 0x10000 / 0x10000;
      }
      result[i + value_.size()] = carry;
    }
    if (result.back() == 0)
      result.pop_back();
    result.swap(value_);
  }

  // returns remainder
  uint32_t devide(const uint32_t x)
  {
    uint64_t carry = 0;
    for (size_t i = value_.size(); i > 0; --i)
    {
      uint64_t prod = carry << 32;
      prod += value_[i-1];

      value_[i-1] = prod / x;
      carry = prod % x;
    }

    if (!value_.empty() && value_.back() == 0)
      value_.pop_back();
    return carry;
  }

  typedef std::vector<uint32_t> value_type;

  const value_type& getValue() const
  {
    return value_;
  }

  void setValue(int n, uint32_t x)
  {
    value_.assign(n, x);
  }

 private:
  void parseDec(const std::string& str);
  void parseHex(const std::string& str);

  value_type value_;

  const static uint32_t kMask_ = 0xFFFFFFFF;
};
