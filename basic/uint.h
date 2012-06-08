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

  UnsignedInt(const std::string& x, Radix r = kDec)
  {
    // FIXME
  }

  std::string toHex() const;
  std::string toDec() const;

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

  // returns reminder
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

 private:
  value_type value_;

  const static uint32_t kMask_ = 0xFFFFFFFF;
};

