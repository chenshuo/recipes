#include <assert.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

template<typename C>
class MergedIterator
{
 public:
  explicit MergedIterator(const std::vector<C>& input)
  {
    heap_.reserve(input.size());
    int i = 0;
    for (const C& c : input)
    {
      if (!c.empty())
        heap_.push_back({.iter = c.cbegin(), .end = c.cend(), .idx = i++});
    }
    std::make_heap(heap_.begin(), heap_.end());
  }

  bool done() const
  {
    return heap_.empty();
  }

  const typename C::value_type& get() const
  {
    assert(!done());
    return *heap_.front().iter;
  }

  void next()
  {
    assert(!done());
    std::pop_heap(heap_.begin(), heap_.end());
    if (++heap_.back().iter == heap_.back().end)
      heap_.pop_back();
    else
      std::push_heap(heap_.begin(), heap_.end());
  }

 private:
  typedef typename C::const_iterator Iterator;

  struct Item
  {
    Iterator iter, end;
    int idx = 0;

    // TODO: generalize this using C::value_comp ?
    bool operator<(const Item& rhs) const
    {
      if (iter->first == rhs.iter->first)
        return idx > rhs.idx;
      return iter->first > rhs.iter->first;
    }
  };

  std::vector<Item> heap_;
};

int main()
{
  using Map = std::map<int, std::string>;
  std::vector<Map> maps(4);
  maps[0][1] = "1.a";
  maps[1][1] = "1.b";
  maps[2][1] = "1.c";
  maps[0][2] = "2.a";
  maps[1][3] = "3.b";
  maps[2][4] = "4.c";
  maps[0][9] = "9.a";
  maps[1][8] = "8.b";
  maps[2][7] = "7.c";

  for (MergedIterator<Map> s(maps); !s.done(); s.next())
  {
    std::cout << s.get().first << " " << s.get().second << "\n";
  }
}
