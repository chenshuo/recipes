// sort word by frequency, in-memory version.
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>

typedef std::unordered_map<std::string, int> WordCount;

int main()
{
  WordCount counts;
  std::string word;
  while (std::cin >> word)
  {
    counts[word]++;
  }

  std::vector<std::pair<int, WordCount::const_iterator>> freq;
  freq.reserve(counts.size());
  for (auto it = counts.cbegin(); it != counts.cend(); ++it)
  {
    freq.push_back(make_pair(it->second, it));
  }

  std::sort(freq.begin(), freq.end(), [](const std::pair<int, WordCount::const_iterator>& lhs,  // const auto& lhs in C++14
                                         const std::pair<int, WordCount::const_iterator>& rhs) {
    return lhs.first > rhs.first;
  });
  // printf("%zd\n", sizeof(freq[0]));
  for (auto item : freq)
  {
    std::cout << item.first << '\t' << item.second->first << '\n';
  }
}
