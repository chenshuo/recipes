/* sort word by frequency, sorting version.

   1. read input files, sort every 1GB to segment files
      word \t count  -- sorted by word
   2. read all segment files, do merging & counting, when count map > 10M keys, output to count files, each word goes to one count file only.
      count \t word  -- sorted by count
   3. read all count files, do merging and output
*/

#include "timer.h"

#include "muduo/base/Logging.h"

#include <assert.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

using std::pair;
using std::string;
using std::string_view;
using std::vector;

const size_t kMaxSize = 10 * 1000 * 1000;
bool g_verbose = false, g_keep = false;
const char* segment_dir = ".";
const char* g_output = "output";

inline double now()
{
  struct timeval tv = { 0, 0 };
  gettimeofday(&tv, nullptr);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int sort_segments(int count, int fd)
{
  Timer timer;
  const int64_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  printf("  file size %ld\n", file_size);
  int64_t offset = 0;
  while (offset < file_size)
  {
    double t = now();
    const int64_t len = std::min(file_size - offset, 1024 * 1000 * 1000L);
    printf("    segment %d reading range: offset %ld len %ld", count, offset, len);
    char* const buf = (char*)malloc(len);
    const ssize_t nr = ::pread(fd, buf, len, offset);
    double sec = now() - t;
    printf(" %.3f sec %.3f MB/s\n", sec, nr / sec / 1000 / 1000);

    t = now();
    const char* const start = buf;
    const char* const end = start + nr;
    vector<string_view> items;
    const char* p = start;
    while (p < end)
    {
      const char* nl = static_cast<const char*>(memchr(p, '\n', end - p));
      if (nl)
      {
        string_view s(p, nl - p);
        items.push_back(s);
        p = nl + 1;
      }
      else
      {
        break;
      }
    }
    offset += p - start;
    printf("    parse %.3f sec %ld items %ld bytes\n", now() - t, items.size(), p - start);

    t = now();
    std::sort(items.begin(), items.end());
    printf("    sort %.3f sec\n", now() - t);

    t = now();
    char name[256];
    snprintf(name, sizeof name, "%s/segment-%05d", segment_dir, count);
    ++count;
    int unique = 0;
    {
    std::ofstream out(name);
    string_view curr;
    int cnt = 0;
    for (auto it = items.begin(); it != items.end(); ++it)
    {
      if (*it != curr)
      {
        if (cnt)
        {
          out << curr << '\t' << cnt << '\n';
          ++unique;
        }
        curr = *it;
        cnt = 1;
      }
      else
        ++cnt;
    }
    if (cnt)
    {
      out << curr << '\t' << cnt << '\n';
      ++unique;
    }
    }
    printf("    unique %.3f sec %d\n", now() - t, unique);

    free(buf);
  }
  LOG_INFO << "done file " << timer.report(file_size);
  return count;
}

int input(int argc, char* argv[])
{
  int count = 0;
  double t = now();
  for (int i = optind; i < argc; ++i)
  {
    std::cout << "processing input file " << argv[i] << std::endl;

    int fd = open(argv[i], O_RDONLY);
    if (fd >= 0)
    {
      double t = now();
      count = sort_segments(count, fd);
      printf("  sort segments %.3f sec\n", now() - t);
      ::close(fd);
    }
    else
      perror("open");
  }
  std::cout << "reading done " << count << " segments " << now() - t << " sec" << std::endl;
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

void output(int i, std::vector<pair<string, int64_t>>* counts)
{
  double t = now();

  std::sort(counts->begin(), counts->end(), [](const auto& lhs, const auto& rhs) {
    return lhs.second > rhs.second;
  });
  std::cout << "  sorting " << now() - t << " sec" << std::endl;

  t = now();
  char buf[256];
  snprintf(buf, sizeof buf, "count-%05d", i);
  std::ofstream out(buf);
  for (const auto& it : *counts)
  {
    out << it.second << '\t' << it.first << '\n';
  }
  std::cout << "  writing " << buf << " " << now() - t << " sec" << std::endl;
}

int combine(int count)
{
  Timer timer;
  std::vector<std::unique_ptr<std::ifstream>> inputs;
  std::vector<Segment> keys;

  double t = now();

  int64_t total = 0;
  for (int i = 0; i < count; ++i)
  {
    char buf[256];
    snprintf(buf, sizeof buf, "%s/segment-%05d", segment_dir, i);
    struct stat st;
    if (::stat(buf, &st) == 0)
    {
      total += st.st_size;
      inputs.emplace_back(new std::ifstream(buf));
      Segment rec(inputs.back().get());
      if (rec.next())
      {
        keys.push_back(rec);
      }
      if (!g_keep)
        ::unlink(buf);
    }
    else
    {
      perror("Cannot open segment");
    }
  }
  LOG_INFO << "Combining " << count << " files " << total << " bytes";

  // std::cout << keys.size() << '\n';
  int m = 0;
  string last = "Chen Shuo";
  // std::unordered_map<string, int64_t> counts;
  vector<pair<string, int64_t>> counts;
  std::make_heap(keys.begin(), keys.end());
  int64_t lines_in = 0, lines_out = 0;

  double tt = now();
  while (!keys.empty())
  {
    std::pop_heap(keys.begin(), keys.end());
    lines_in++;

    if (keys.back().word != last)
    {
      last = keys.back().word;
      if (counts.size() > kMaxSize)
      {
        LOG_INFO << "    split " << now() - tt << " sec";
        output(m++, &counts);
        tt = now();
        counts.clear();
        // LOG_INFO << "cleared " << lines_in << " " << lines_out;
      }
      lines_out++;
      counts.push_back(make_pair(keys.back().word, keys.back().count));
    }
    else
    {
      assert(counts.back().first == last);
      counts.back().second += keys.back().count;
    }

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
    std::cout << "    split " << now() - tt << " sec" << std::endl;
    output(m++, &counts);
  }
  std::cout << "combining done " << m << " count files " << now() - t << " sec" << std::endl;
  LOG_INFO << timer.report(total);
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

  double t = now();
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
    if (!g_keep)
      ::unlink(buf);
  }

  std::ofstream out(g_output);
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
  std::cout << "merging done " << now() - t << " sec\n";
}

int main(int argc, char* argv[])
{
  setlocale(LC_NUMERIC, "");
  int opt;
  bool sort_only = false;
  int count_only = 0;
  int merge_only = 0;
  while ((opt = getopt(argc, argv, "c:d:km:o:sv")) != -1)
  {
    switch (opt)
    {
      case 'c':
        count_only = atoi(optarg);
        break;
      case 'd':
        segment_dir = optarg;
        break;
      case 'k':
        g_keep = true;
        break;
      case 'm':
        merge_only = atoi(optarg);
        break;
      case 'o':
        g_output = optarg;
        break;
      case 's':
        sort_only = true;
        break;
      case 'v':
        g_verbose = true;
        break;
    }
  }

  if (sort_only || count_only > 0 || merge_only > 0)
  {
    g_keep = true;
    if (sort_only)
    {
      int count = input(argc, argv);
    }
    if (count_only)
    {
      int m = combine(count_only);
    }
    if (merge_only)
    {
      merge(merge_only);
    }
  }
  else
  {
    int count = input(argc, argv);
    int m = combine(count);
    merge(m);
  }
}
