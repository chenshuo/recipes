#include <algorithm>
#include <vector>

#include <iostream>
#include <iterator>

#include <assert.h>

using namespace std;

vector<int> selectBySorting(const vector<int>& input, int k1, int k2)
{
  assert(0 <= k1 && k1 < input.size());
  assert(0 <= k2 && k2 < input.size());
  assert(k1 <= k2);

  vector<int> temp(input);
  sort(temp.begin(), temp.end());
  return vector<int>(temp.begin() + k1, temp.begin() + k2 + 1);
}

vector<int> selectByNthElement(const vector<int>& input, int k1, int k2)
{
  assert(0 <= k1 && k1 < input.size());
  assert(0 <= k2 && k2 < input.size());
  assert(k1 <= k2);

  vector<int> temp(input);
  nth_element(temp.begin(), temp.begin() + k2 + 1, temp.end());
  nth_element(temp.begin(), temp.begin() + k1, temp.begin() + k2 + 1);
  sort(temp.begin() + k1, temp.begin() + k2 + 1);
  return vector<int>(temp.begin() + k1, temp.begin() + k2 + 1);
}

vector<int> selectByPartialSort(const vector<int>& input, int k1, int k2)
{
  assert(0 <= k1 && k1 < input.size());
  assert(0 <= k2 && k2 < input.size());
  assert(k1 <= k2);

  vector<int> temp(input);
  nth_element(temp.begin(), temp.begin() + k1, temp.end());
  partial_sort(temp.begin() + k1, temp.begin() + k2 + 1, temp.end());
  return vector<int>(temp.begin() + k1, temp.begin() + k2 + 1);
}

void print(const vector<int>& vec)
{
  copy(vec.begin(), vec.end(), ostream_iterator<int>(cout, ", "));
  cout << endl;
}

int main()
{
  vector<int> input;
  for (int i = 0; i < 100; ++i)
  {
    input.push_back(i);
  }
  random_shuffle(input.begin(), input.end());
  print(input);
  print(selectBySorting(input, 10, 20));
  print(selectByNthElement(input, 10, 20));
  print(selectByPartialSort(input, 10, 20));
}
