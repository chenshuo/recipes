#include <assert.h>
#include <stdint.h>

#include <algorithm>
#include <vector>

struct IPrange
{
  uint32_t startIp;  // inclusive
  uint32_t endIp;    // inclusive
  int value;         // >= 0

  bool operator<(const IPrange& rhs) const
  {
    return startIp < rhs.startIp;
  }
};

// REQUIRE: ranges is sorted.
int findIpValue(const std::vector<IPrange>& ranges, uint32_t ip)
{
  int result = -1;

  if (!ranges.empty()) {
    IPrange needle = { ip, 0, 0 };
    std::vector<IPrange>::const_iterator it
        = std::lower_bound(ranges.begin(), ranges.end(), needle);
    if (it == ranges.end()) {
      --it;
    } else if (it != ranges.begin() && it->startIp > ip) {
      --it;
    }

    if (it->startIp <= ip && it->endIp >= ip) {
      result = it->value;
    }
  }

  return result;
}

bool operator==(const IPrange& lhs, const IPrange& rhs)
{
  return lhs.startIp == rhs.startIp;
}

int main()
{
  std::vector<IPrange> ranges;
  IPrange r1 = { 123, 234, 999 };
  ranges.push_back(r1);

  std::sort(ranges.begin(), ranges.end());
  assert(std::adjacent_find(ranges.begin(), ranges.end()) == ranges.end());

  int v = findIpValue(ranges, 0);
  assert(v == -1);

  v = findIpValue(ranges, 122);
  assert(v == -1);

  v = findIpValue(ranges, 123);
  assert(v == 999);

  v = findIpValue(ranges, 234);
  assert(v == 999);

  v = findIpValue(ranges, 235);
  assert(v == -1);

  IPrange r2 = { 1000, 2000, 7777 };
  ranges.push_back(r2);

  sort(ranges.begin(), ranges.end());
  assert(adjacent_find(ranges.begin(), ranges.end()) == ranges.end());

  v = findIpValue(ranges, 0);
  assert(v == -1);

  v = findIpValue(ranges, 122);
  assert(v == -1);

  v = findIpValue(ranges, 123);
  assert(v == 999);

  v = findIpValue(ranges, 234);
  assert(v == 999);

  v = findIpValue(ranges, 235);
  assert(v == -1);

  v = findIpValue(ranges, 999);
  assert(v == -1);

  v = findIpValue(ranges, 1000);
  assert(v == 7777);

  v = findIpValue(ranges, 1500);
  assert(v == 7777);

  v = findIpValue(ranges, 2000);
  assert(v == 7777);

  v = findIpValue(ranges, 2001);
  assert(v == -1);

  v = findIpValue(ranges, 1073741824);
  assert(v == -1);

  IPrange r3 = { 1073741824U*3, 1073741824U*3+1073741823U, 5555 };
  ranges.push_back(r3);

  v = findIpValue(ranges, 1073741824U*2);
  assert(v == -1);

  v = findIpValue(ranges, 1073741824U*3);
  assert(v == 5555);

  v = findIpValue(ranges, 1073741824U*3+1073741822U);
  assert(v == 5555);

  v = findIpValue(ranges, 1073741824U*3+1073741823U);
  assert(v == 5555);

  v = findIpValue(ranges, 1073741824U*3+1073741824U);
  assert(v == -1);

  IPrange r4 = { 1073741824U*3+1073741823U, 1073741824U*3+1073741823U, 3333 };
  ranges.push_back(r4);

  v = findIpValue(ranges, 1073741824U*3+1073741822U);
  assert(v == 5555);

  v = findIpValue(ranges, 1073741824U*3+1073741823U);
  assert(v == 3333);
}
