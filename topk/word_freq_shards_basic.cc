/* sort word by frequency, sharding version.

  1. read input file, shard to N files:
       word
  2. assume each shard file fits in memory, read each shard file, count words and sort by count, then write to N count files:
       count \t word
  3. merge N count files using heap.

Limits: each shard must fit in memory.
*/

#include <assert.h>

#include "absl/container/flat_hash_map.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>

using std::string;
using std::string_view;
using std::vector;
using std::unique_ptr;

int kShards = 10;

inline double now()
{
  struct timeval tv = { 0, 0 };
  gettimeofday(&tv, nullptr);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

struct CpuTime
{
  double userSeconds = 0.0;
  double systemSeconds = 0.0;

  double total() const { return userSeconds + systemSeconds; }
};

const int g_clockTicks = static_cast<int>(::sysconf(_SC_CLK_TCK));

CpuTime cpuTime()
{
  CpuTime t;
  struct tms tms;
  if (::times(&tms) >= 0)
  {
    const double hz = static_cast<double>(g_clockTicks);
    t.userSeconds = static_cast<double>(tms.tms_utime) / hz;
    t.systemSeconds = static_cast<double>(tms.tms_stime) / hz;
  }
  return t;
}

class Timer
{
 public:
  Timer()
    : start_(now()),
      start_cpu_(cpuTime())
  {
  }

  void report(int64_t bytes) const
  {
    CpuTime end_cpu(cpuTime());
    double end = now();
    printf("%.3f real  %.3f cpu  %.2f MiB/s  %ld bytes\n",
           end - start_, end_cpu.total() - start_cpu_.total(),
           bytes / (end - start_) / 1024 / 1024, bytes);
  }
 private:
  const double start_ = 0;
  const CpuTime start_cpu_;
};

class OutputFile // : boost::noncopyable
{
 public:
  explicit OutputFile(const string& filename)
    : file_(::fopen(filename.c_str(), "w+"))
  {
    assert(file_);
    ::setbuffer(file_, buffer_, sizeof buffer_);
  }

  ~OutputFile()
  {
    close();
  }

  void append(string_view s)
  {
    assert(s.size() < 255);
    uint8_t len = s.size();
    ::fwrite(&len, 1, sizeof len, file_);
    ::fwrite(s.data(), 1, len, file_);
    ++items_;
  }

  /*
  void append(uint64_t x)
  {
    // FIXME: htobe64(x);
    ::fwrite(&x, 1, sizeof x, file_);
  }
  */

  void flush()
  {
    ::fflush(file_);
  }

  void close()
  {
    if (file_)
      ::fclose(file_);
    file_ = nullptr;
  }

  int64_t tell()
  {
    return ::ftell(file_);
  }

  int fd()
  {
    return ::fileno(file_);
  }

  size_t items()
  {
    return items_;
  }

 private:
  FILE* file_;
  char buffer_[64 * 1024];
  size_t items_ = 0;

  OutputFile(const OutputFile&) = delete;
  void operator=(const OutputFile&) = delete;
};

class Sharder // : boost::noncopyable
{
 public:
  Sharder()
    : files_(kShards)
  {
    for (int i = 0; i < kShards; ++i)
    {
      char name[256];
      snprintf(name, sizeof name, "shard-%05d-of-%05d", i, kShards);
      files_[i].reset(new OutputFile(name));
    }
    assert(files_.size() == static_cast<size_t>(kShards));
  }

  void output(string_view word)
  {
    size_t shard = hash(word) % files_.size();
    files_[shard]->append(word);
  }

  void finish()
  {
    int shard = 0;
    for (const auto& file : files_)
    {
      printf("  shard %d: %ld bytes, %ld items\n", shard, file->tell(), file->items());
      ++shard;
      file->close();
    }
  }

 private:
  std::hash<string_view> hash;
  vector<unique_ptr<OutputFile>> files_;
};

int64_t shard_(int argc, char* argv[])
{
  Sharder sharder;
  Timer timer;
  int64_t total = 0;
  for (int i = 1; i < argc; ++i)
  {
    std::cout << "  processing input file " << argv[i] << std::endl;
    double t = now();
    char line[1024];
    FILE* fp = fopen(argv[i], "r");
    char buffer[65536];
    ::setbuffer(fp, buffer, sizeof buffer);
    while (fgets(line, sizeof line, fp))
    {
      size_t len = strlen(line);
      if (len > 0 && line[len-1] == '\n')
        line[len-1] = '\0';
      sharder.output(line);
    }
    size_t len = ftell(fp);
    fclose(fp);
    total += len;
    double sec = now() - t;
    printf("%.3f sec %.2f MiB/s\n", sec, len / sec / 1024 / 1024);
  }
  sharder.finish();
  printf("sharding done ");
  timer.report(total);
  return total;
}

// ======= count_shards =======

int64_t count_shard(int shard, int fd)
{
  const int64_t len = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  double t = now();
  printf("shard %d: file size %ld\n", shard, len);
  {
  void* mapped = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(mapped != MAP_FAILED);
  const uint8_t* const start = static_cast<const uint8_t*>(mapped);
  const uint8_t* const end = start + len;

  // std::unordered_map<string_view, uint64_t> items;
  absl::flat_hash_map<string_view, uint64_t> items;
  for (const uint8_t* p = start; p < end;)
  {
    string_view s((const char*)p+1, *p);
    items[s]++;
    p += 1 + *p;
  }
  printf("  count %.3f sec %ld items\n", now() - t, items.size());

  t = now();
  vector<std::pair<size_t, string_view>> counts;
  for (const auto& it : items)
  {
    if (it.second > 1)
      counts.push_back(make_pair(it.second, it.first));
  }
  printf("  select %.3f sec %ld\n", now() - t, counts.size());

  t = now();
  std::sort(counts.begin(), counts.end());
  printf("  sort %.3f sec\n", now() - t);

  t = now();
  {
    char buf[256];
    snprintf(buf, sizeof buf, "count-%05d-of-%05d", shard, kShards);
    std::ofstream out(buf);
    for (auto it = counts.rbegin(); it != counts.rend(); ++it)
    {
      out << it->first << '\t' << it->second << '\n';
    }
    for (const auto& it : items)
    {
      if (it.second == 1)
      {
        out << "1\t" << it.first << '\n';
      }
    }
  }
  printf("  output %.3f sec\n", now() - t);

  t = now();
  if (munmap(mapped, len))
    perror("munmap");
  }
  printf("  destruct %.3f sec\n", now() - t);
  return len;
}

void count_shards()
{
  Timer timer;
  int64_t total = 0;
  for (int shard = 0; shard < kShards; ++shard)
  {
    char buf[256];
    snprintf(buf, sizeof buf, "shard-%05d-of-%05d", shard, kShards);
    int fd = open(buf, O_RDONLY);
    double t = now();
    int64_t len = count_shard(shard, fd);
    ::close(fd);
    ::unlink(buf);
    total += len;
    printf("shard %d: %.2f MiB/s\n", shard, len / (now() - t) / 1024 / 1024);
  }
  printf("count done ");
  timer.report(total);
}

// ======= merge =======

class Source  // copyable
{
 public:
  explicit Source(std::istream* in)
    : in_(in),
      count_(0),
      word_()
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
  int64_t count_;
  string word_;
};

void merge()
{
  Timer timer;
  vector<unique_ptr<std::ifstream>> inputs;
  vector<Source> keys;

  int64_t total = 0;
  for (int i = 0; i < kShards; ++i)
  {
    char buf[256];
    snprintf(buf, sizeof buf, "count-%05d-of-%05d", i, kShards);
    struct stat st;
    ::stat(buf, &st);
    total += st.st_size;
    inputs.emplace_back(new std::ifstream(buf));
    Source rec(inputs.back().get());
    if (rec.next())
    {
      keys.push_back(rec);
    }
    ::unlink(buf);
  }

  {
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
  }
  printf("merging done ");
  timer.report(total);
}

int main(int argc, char* argv[])
{
  /*
  int fd = open("shard-00000-of-00010", O_RDONLY);
  double t = now();
  int64_t len = count_shard(0, fd);
  double sec = now() - t;
  printf("count_shard %.3f sec %.2f MB/s\n", sec, len / sec / 1e6);
  */

  Timer timer;
  int64_t total = shard_(argc, argv);
  count_shards();
  merge();
  printf("All done ");
  timer.report(total);
}
