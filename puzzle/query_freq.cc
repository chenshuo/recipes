// answer to http://weibo.com/1915548291/z2UtyzuvQ
// see also http://www.cnblogs.com/baiyanhuang/archive/2012/11/11/2764914.html

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <fstream>
#include <iostream>
#include <unordered_map>

#ifdef STD_STRING
#warning "STD STRING"
#include <string>
using std::string;
#else
#include <ext/vstring.h>
typedef __gnu_cxx::__sso_string string;
#endif

typedef boost::function<void(const string& query, int64_t count)> Callback;

const size_t kMaxSize = 10 * 1000 * 1000;

void processOneInput(const char* file, const Callback& cb)
{
  std::unordered_map<string, int64_t> queries;
  std::ifstream in(file);
  string query;
  while (in && !in.eof())
  {
    queries.clear();
    while (getline(in, query))
    {
      queries[query] += 1;
      if (queries.size() > kMaxSize)
      {
        std::cout << "    split\n";
        break;
      }
    }

    for (auto kv : queries)
    {
      cb(kv.first, kv.second);
    }
  }
}

class LocalSink : boost::noncopyable
{
 public:
  explicit LocalSink(int idx, int nbuckets)
  {
    char buf[256];
    snprintf(buf, sizeof buf, "shard-%05d-of-%05d", idx, nbuckets);
    out.open(buf);
  }

  void output(const string& query, int64_t count)
  {
    out << query << '\t' << count << '\n';
  }

 private:
  std::ofstream out;
};

// class RemoteSink;

// template<typename SINK>
class Shuffler : boost::noncopyable
{
 public:
  explicit Shuffler(int nbuckets)
    : buckets_(nbuckets)
  {
    for (int i = 0; i < nbuckets; ++i)
    {
      buckets_.push_back(new LocalSink(i, nbuckets));
    }
    assert(buckets_.size() == static_cast<size_t>(nbuckets));
  }

  void output(const string& query, int64_t count)
  {
    int idx = std::hash<string>()(query) % buckets_.size();
    buckets_[idx].output(query, count);
  }

 protected:
  boost::ptr_vector<LocalSink> buckets_;
};

void shuffle(int nbuckets, int argc, char* argv[])
{
  Shuffler shuffler(nbuckets);
  for (int i = 1; i < argc; ++i)
  {
    std::cout << "  processing " << argv[i] << std::endl;
    processOneInput(argv[i],
                    boost::bind(&Shuffler::output, &shuffler, _1, _2));
  }
  std::cout << "shuffling done" << std::endl;
}

// ======= reduce =======

std::unordered_map<string, int64_t> read_shard(int idx, int nbuckets)
{
  std::unordered_map<string, int64_t> queries;

  char buf[256];
  snprintf(buf, sizeof buf, "shard-%05d-of-%05d", idx, nbuckets);
  std::cout << "  reading " << buf << std::endl;
  {
    std::ifstream in(buf);
    string line;

    while (getline(in, line))
    {
      size_t tab = line.find('\t');
      if (tab != string::npos)
      {
        int64_t count = strtol(line.c_str() + tab, NULL, 10);
        if (count > 0)
        {
          queries[line.substr(0, tab)] += count;
        }
      }
    }
  }

  ::unlink(buf);
  return queries;
}

void reduce(const int nbuckets)
{
  for (int i = 0; i < nbuckets; ++i)
  {
    std::unordered_map<string, int64_t> queries(read_shard(i, nbuckets));

    // std::cout << "  sorting " << std::endl;
    std::vector<std::pair<int64_t, string>> counts;
    for (auto entry : queries)
    {
      counts.push_back(make_pair(entry.second, entry.first));
    }
    std::sort(counts.begin(), counts.end());

    char buf[256];
    snprintf(buf, sizeof buf, "count-%05d-of-%05d", i, nbuckets);
    std::ofstream out(buf);
    std::cout << "  writing " << buf << std::endl;
    for (auto it = counts.rbegin(); it != counts.rend(); ++it)
    {
      out << it->first << '\t' << it->second << '\n';
    }
  }

  std::cout << "reducing done" << std::endl;
}

// ======= merge =======
class Source
{
 public:
  explicit Source(std::ifstream* in)
    : in_(in),
      count_(0),
      query_()
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
          query_ = line.substr(tab+1);
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

  void output(std::ostream& out)
  {
    out << count_ << '\t' << query_ << '\n';
  }

 private:
  std::ifstream* in_;
  int64_t count_;
  string query_;
};

void merge(const int nbuckets)
{
  boost::ptr_vector<std::ifstream> inputs;
  std::vector<Source> keys;

  for (int i = 0; i < nbuckets; ++i)
  {
    char buf[256];
    snprintf(buf, sizeof buf, "count-%05d-of-%05d", i, nbuckets);
    inputs.push_back(new std::ifstream(buf));
    Source rec(&inputs.back());
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
    keys.back().output(out);
    // out.writeRecord(keys.back().data);

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
  int nbuckets = 10;
  shuffle(nbuckets, argc, argv);
  reduce(nbuckets);
  merge(nbuckets);
}
