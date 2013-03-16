#include <assert.h>
#include <stdio.h>

#include <vector>

// each element < 10000
// LSB first, ie. little endian
typedef std::vector<int> BigInt;

BigInt factorial(int n)
{
  // 9999 * 9999 < 10000 * 10000 = 100000000 < 1073741824  = 2 ** 30
  assert(n >= 0 && n <= 10000);

  BigInt result;
  result.push_back(1);
  for (int factor = 1; factor <= n; ++factor) {
    int carry = 0;
    for (auto& item : result) {
      int product = item * factor + carry;
      item = product % 10000;
      carry = product / 10000;
    }
    if (carry > 0) {
      result.push_back(carry);
    }
  }
  return result;
}

void printBigInt(const BigInt& number)
{
  if (number.empty())
  {
    printf("0\n");  // compiles to puts()
  }
  else
  {
    printf("%d", number.back());
    for (auto it = number.rbegin()+1; it != number.rend(); ++it)
      printf("%04d", *it);
    printf("\n");
  }
}

int main()
{
  /*
  for (int i = 0; i <= 10000; ++i)
  {
    BigInt result = factorial(i);
    printf("%d: ", i);
    printBigInt(result);
  }
  */
  BigInt result = factorial(10000);
  printBigInt(result);
}
