#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#ifdef STD_STRING
#warning "STD STRING"
#include <string>
using std::string;
#else
#include <ext/vstring.h>
typedef __gnu_cxx::__sso_string string;
#endif

#include <unistd.h>

const size_t kMaxSize = 10 * 1000 * 1000;

int input(int argc, char* argv[])
{
  int count = 0;
  for (int i = 1; i < argc; ++i)
  {
    std::cout << "  processing input file " << argv[i] << std::endl;
    std::map<string, int64_t> counts;
    std::ifstream in(argv[i]);
    while (in && !in.eof())
    {
      counts.clear();
      string word;
      while (in >> word)
      {
        counts[word]++;
        if (counts.size() > kMaxSize)
        {
          std::cout << "    split" << std::endl;
          break;
        }
      }

      char buf[256];
      snprintf(buf, sizeof buf, "segment-%05d", count);
      std::ofstream out(buf);
      ++count;
      std::cout << "  writing " << buf << std::endl;
      for (const auto& kv : counts)
      {
        out << kv.first << '\t' << kv.second << '\n';
      }
    }
  }
  std::cout << "reading done, " << count << std::endl;
  return count;
}

// ======= combine =======

class Segment  // copyable
{
 public:
  string word;
  int64_t count = 0;

  explicit Segment(std::istream* in)
    : in_(in)
  {
  }

  bool next()
  {
    string line;
    if (getline(*in_, line))
    {
      size_t tab = line.find('\t');
      if (tab != string::npos)
      {
        word = line.substr(0, tab);
        count = strtol(line.c_str() + tab, NULL, 10);
        return true;
      }
    }
    return false;
  }

  bool operator<(const Segment& rhs) const
  {
    return word > rhs.word;
  }

 private:
  std::istream* in_;
};

void output(int i, const std::unordered_map<string, int64_t>& counts)
{
  std::vector<std::pair<int64_t, string>> freq;
  for (const auto& entry : counts)
  {
    freq.push_back(make_pair(entry.second, entry.first));
  }
  std::sort(freq.begin(), freq.end());

  char buf[256];
  snprintf(buf, sizeof buf, "count-%05d", i);
  std::ofstream out(buf);
  std::cout << "  writing " << buf << std::endl;
  for (auto it = freq.rbegin(); it != freq.rend(); ++it)
  {
    out << it->first << '\t' << it->second << '\n';
  }
}

int combine(int count)
{
  std::vector<std::unique_ptr<std::ifstream>> inputs;
  std::vector<Segment> keys;

  for (int i = 0; i < count; ++i)
  {
    char buf[256];
    snprintf(buf, sizeof buf, "segment-%05d", i);
    inputs.emplace_back(new std::ifstream(buf));
    Segment rec(inputs.back().get());
    if (rec.next())
    {
      keys.push_back(rec);
    }
    ::unlink(buf);
  }

  // std::cout << keys.size() << '\n';
  int m = 0;
  string last;
  std::unordered_map<string, int64_t> counts;
  std::make_heap(keys.begin(), keys.end());
  while (!keys.empty())
  {
    std::pop_heap(keys.begin(), keys.end());

    if (keys.back().word != last)
    {
      last = keys.back().word;
      if (counts.size() > kMaxSize)
      {
        std::cout << "    split" << std::endl;
        output(m++, counts);
        counts.clear();
      }
    }

    counts[keys.back().word] += keys.back().count;

    if (keys.back().next())
    {
      std::push_heap(keys.begin(), keys.end());
    }
    else
    {
      keys.pop_back();
    }
  }

  if (!counts.empty())
  {
    output(m++, counts);
  }
  std::cout << "combining done, " << m << std::endl;
  return m;
}

// ======= merge  =======

class Source  // copyable
{
 public:
  explicit Source(std::istream* in)
    : in_(in)
  {
  }

  bool next()
  {
    string line;
    if (getline(*in_, line))
    {
      size_t tab = line.find('\t');
      if (tab != string::npos)
      {
        count_ = strtol(line.c_str(), NULL, 10);
        if (count_ > 0)
        {
          word_ = line.substr(tab+1);
          return true;
        }
      }
    }
    return false;
  }

  bool operator<(const Source& rhs) const
  {
    return count_ < rhs.count_;
  }

  void outputTo(std::ostream& out) const
  {
    out << count_ << '\t' << word_ << '\n';
  }

 private:
  std::istream* in_;
  int64_t count_ = 0;
  string word_;
};

void merge(int m)
{
  std::vector<std::unique_ptr<std::ifstream>> inputs;
  std::vector<Source> keys;

  for (int i = 0; i < m; ++i)
  {
    char buf[256];
    snprintf(buf, sizeof buf, "count-%05d", i);
    inputs.emplace_back(new std::ifstream(buf));
    Source rec(inputs.back().get());
    if (rec.next())
    {
      keys.push_back(rec);
    }
    ::unlink(buf);
  }

  std::ofstream out("output");
  std::make_heap(keys.begin(), keys.end());
  while (!keys.empty())
  {
    std::pop_heap(keys.begin(), keys.end());
    keys.back().outputTo(out);

    if (keys.back().next())
    {
      std::push_heap(keys.begin(), keys.end());
    }
    else
    {
      keys.pop_back();
    }
  }
  std::cout << "merging done\n";
}

int main(int argc, char* argv[])
{
  int count = input(argc, argv);
  int m = combine(count);
  merge (m);
}
