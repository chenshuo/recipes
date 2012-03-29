#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

int main()
{
  char elements[] = { 'a', 'a', 'b', 'b', 'b' };
  const size_t N = sizeof(elements)/sizeof(elements[0]);
  std::vector<char> vec(elements, elements + N);

  int count = 0;
  do
  {
    std::cout << ++count << ": ";
    std::copy(vec.begin(), vec.end(),
              std::ostream_iterator<char>(std::cout, ", "));
    std::cout << std::endl;
  } while (next_permutation(vec.begin(), vec.end()));
}
