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

  std::sort(freq.begin(), freq.end(), [](const auto& lhs, const auto& rhs) {
    return lhs.first > rhs.first;
  });
  // printf("%zd\n", sizeof(freq[0]));
  for (auto item : freq)
  {
    std::cout << item.first << '\t' << item.second->first << '\n';
  }
}
