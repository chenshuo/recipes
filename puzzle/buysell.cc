#include <assert.h>
#include <stdio.h>
#include <vector>

int buysell(const std::vector<int>& prices)
{
  assert(!prices.empty());
  size_t lowestBuy = 0;
  size_t lowestBuySoFar = 0;
  size_t highestSell = 0;
  int maxGain = 0;
  for (size_t day = 1; day < prices.size(); ++day)
  {
    int todayPrice = prices[day];
    int gain = todayPrice - prices[lowestBuySoFar];
    if (gain > maxGain)
    {
      lowestBuy = lowestBuySoFar;
      highestSell = day;
      maxGain = gain;
    }

    if (todayPrice < prices[lowestBuySoFar])
    {
      lowestBuySoFar = day;
    }
  }
  printf("buy %zd day at %d, sell %zd day at %d\n",
      lowestBuy, prices[lowestBuy], highestSell, prices[highestSell]);
  assert(lowestBuy <= highestSell);
  return maxGain;
}

template<int N>
std::vector<int> toVec(int (&arr)[N])
{
  return std::vector<int>(arr, arr+N);
}

int main()
{
  int test1[] = { 1, 2 };
  int test2[] = { 1, 2, 3 };
  int test3[] = { 2, 1, 3 };
  int test4[] = { 3, 2, 1 };
  int test5[] = { 2, 3, 1 };
  int test6[] = { 2, 3, 1, 3 };
  int test7[] = { 4, 5, 2, 3, 2, 4 };
  printf("max gain %d\n", buysell(toVec(test1)));
  printf("max gain %d\n", buysell(toVec(test2)));
  printf("max gain %d\n", buysell(toVec(test3)));
  printf("max gain %d\n", buysell(toVec(test4)));
  printf("max gain %d\n", buysell(toVec(test5)));
  printf("max gain %d\n", buysell(toVec(test6)));
  printf("max gain %d\n", buysell(toVec(test7)));
}
