/* sort word by frequency, sharding version.

  1. read input file, shard to N files:
       word
  2. assume each shard file fits in memory, read each shard file, count words and sort by count, then write to N count files:
       count \t word
  3. merge N count files using heap.

Limits: each shard must fit in memory.
*/

#include <assert.h>

#include "file.h"
#include "timer.h"

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"
#include "muduo/base/BoundedBlockingQueue.h"
#include "muduo/base/Logging.h"
#include "muduo/base/ThreadPool.h"

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/program_options.hpp>

#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

using std::string;
using std::string_view;
using std::vector;
using std::unique_ptr;

int kShards = 10, kThreads = 4;
bool g_verbose = false, g_keep = false;
const char* shard_dir = ".";
const char* g_output = "output";

class Sharder // : boost::noncopyable
{
 public:
  Sharder()
    : files_(kShards)
  {
    for (int i = 0; i < kShards; ++i)
    {
      char name[256];
      snprintf(name, sizeof name, "%s/shard-%05d-of-%05d", shard_dir, i, kShards);
      files_[i].reset(new OutputFile(name));
    }
    assert(files_.size() == static_cast<size_t>(kShards));
  }

  void output(string_view word)
  {
    size_t shard = hash(word) % files_.size();
    files_[shard]->appendRecord(word);
  }

  void finish()
  {
    int shard = 0;
    for (const auto& file : files_)
    {
      // if (g_verbose)
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
  for (int i = optind; i < argc; ++i)
  {
    LOG_INFO << "Processing input file " << argv[i];
    double t = Timer::now();
    string line;
    InputFile input(argv[i]);
    while (input.getline(&line))
    {
      sharder.output(line);
    }
    size_t len = input.tell();
    total += len;
    double sec = Timer::now() - t;
    LOG_INFO << "Done file " << argv[i] << absl::StrFormat(" %.3f sec %.2f MiB/s", sec, len / sec / 1024 / 1024);
  }
  sharder.finish();
  LOG_INFO << "Sharding done " << timer.report(total);
  return total;
}

// ======= count_shards =======

void count_shard(int shard, int fd, size_t len)
{
  Timer timer;

  double t = Timer::now();
  LOG_INFO << absl::StrFormat("counting shard %d: input file size %ld", shard, len);
  {
  void* mapped = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(mapped != MAP_FAILED);
  const uint8_t* const start = static_cast<const uint8_t*>(mapped);
  const uint8_t* const end = start + len;

  // std::unordered_map<string_view, uint64_t> items;
  absl::flat_hash_map<string_view, uint64_t> items;
  int64_t count = 0;
  for (const uint8_t* p = start; p < end;)
  {
    string_view s((const char*)p+1, *p);
    items[s]++;
    p += 1 + *p;
    ++count;
  }
  LOG_INFO << "items " << count << " unique " << items.size();
  if (g_verbose)
  printf("  count %.3f sec %ld items\n", Timer::now() - t, items.size());

  t = Timer::now();
  vector<std::pair<size_t, string_view>> counts;
  for (const auto& it : items)
  {
    if (it.second > 1)
      counts.push_back(make_pair(it.second, it.first));
  }
  if (g_verbose)
  printf("  select %.3f sec %ld\n", Timer::now() - t, counts.size());

  t = Timer::now();
  std::sort(counts.begin(), counts.end());
  if (g_verbose)
  printf("  sort %.3f sec\n", Timer::now() - t);

  t = Timer::now();
  {
    char buf[256];
    snprintf(buf, sizeof buf, "count-%05d-of-%05d", shard, kShards);
    OutputFile output(buf);

    for (auto it = counts.rbegin(); it != counts.rend(); ++it)
    {
      string s(it->second);
      output.write(absl::StrFormat("%d\t%s\n", it->first, s));  // FIXME %s with string_view doesn't work in C++17
      /*
      char buf[1024];
      snprintf(buf, sizeof buf, "%zd\t%s\n",
      out.write(buf);
      */
    }

    for (const auto& it : items)
    {
      if (it.second == 1)
      {
        string s(it.first);
        // FIXME: bug of absl?
        // out.write(absl::StrCat("1\t", s, "\n"));
        output.write(absl::StrFormat("1\t%s\n", s));
      }
    }
  }
  //if (g_verbose)
  //printf("  output %.3f sec %lu\n", Timer::now() - t, st.st_size);

  if (munmap(mapped, len))
    perror("munmap");
  }
  ::close(fd);
  LOG_INFO << "shard " << shard << " done " << timer.report(len);
}

void count_shards(int shards)
{
  assert(shards <= kShards);
  Timer timer;
  int64_t total = 0;
  muduo::ThreadPool threadPool;
  threadPool.setMaxQueueSize(2*kThreads);
  threadPool.start(kThreads);

  for (int shard = 0; shard < shards; ++shard)
  {
    char buf[256];
    snprintf(buf, sizeof buf, "%s/shard-%05d-of-%05d", shard_dir, shard, kShards);
    int fd = open(buf, O_RDONLY);
    assert(fd >= 0);
    if (!g_keep)
      ::unlink(buf);

    struct stat st;
    if (::fstat(fd, &st) == 0)
    {
      size_t len = st.st_size;
      total += len;
      threadPool.run([shard, fd, len]{ count_shard(shard, fd, len); });
    }
  }
  while (threadPool.queueSize() > 0)
  {
    LOG_DEBUG << "waiting for ThreadPool " << threadPool.queueSize();
    muduo::CurrentThread::sleepUsec(1000*1000);
  }
  threadPool.stop();
  LOG_INFO << "Counting done "<< timer.report(total);
}

// ======= merge =======

class Source  // copyable
{
 public:
  explicit Source(InputFile* in)
    : in_(in),
      count_(0),
      word_()
  {
  }

  bool next()
  {
    string line;
    if (in_->getline(&line))
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

  void outputTo(OutputFile* out) const
  {
    //char buf[1024];
    //snprintf(buf, sizeof buf, "%ld\t%s\n", count_, word_.c_str());
    //out->write(buf);
    out->write(absl::StrFormat("%d\t%s\n", count_, word_));
  }

  std::pair<int64_t, string> item()
  {
    return make_pair(count_, std::move(word_));
  }

 private:
  InputFile* in_;  // not owned
  int64_t count_;
  string word_;
};

int64_t merge()
{
  Timer timer;
  vector<unique_ptr<InputFile>> inputs;
  vector<Source> keys;

  int64_t total = 0;
  for (int i = 0; i < kShards; ++i)
  {
    char buf[256];
    snprintf(buf, sizeof buf, "count-%05d-of-%05d", i, kShards);
    struct stat st;
    if (::stat(buf, &st) == 0)
    {
      total += st.st_size;
      // TODO: select buffer size based on kShards.
      inputs.push_back(std::make_unique<InputFile>(buf, 32 * 1024 * 1024));
      Source rec(inputs.back().get());
      if (rec.next())
      {
        keys.push_back(rec);
      }
      if (!g_keep)
        ::unlink(buf);
    }
    else
    {
      perror("Unable to stat file:");
    }
  }
  LOG_INFO << "merging " << inputs.size() << " files of " << total << " bytes in total";

  {
  OutputFile out(g_output);
  /*
  muduo::BoundedBlockingQueue<vector<std::pair<int64_t, string>>> queue(1024);
  muduo::Thread thr([&queue] {
    OutputFile out(g_output);
    while (true) {
      auto vec = queue.take();
      if (vec.size() == 0)
        break;
      for (const auto& x : vec)
        out.write(absl::StrFormat("%d\t%s\n", x.first, x.second));
    }
  });
  thr.start();

  vector<std::pair<int64_t, string>> batch;
  */
  std::make_heap(keys.begin(), keys.end());
  while (!keys.empty())
  {
    std::pop_heap(keys.begin(), keys.end());
    keys.back().outputTo(&out);
    /*
    batch.push_back(std::move(keys.back().item()));
    if (batch.size() >= 10*1024*1024)
    {
      queue.put(std::move(batch));
      batch.clear();
    }
    */

    if (keys.back().next())
    {
      std::push_heap(keys.begin(), keys.end());
    }
    else
    {
      keys.pop_back();
    }
  }
  /*
  queue.put(batch);
  batch.clear();
  queue.put(batch);
  thr.join();
  */
  }
  LOG_INFO << "Merging done " << timer.report(total);
  return total;
}

int main(int argc, char* argv[])
{
  /*
  int fd = open("shard-00000-of-00010", O_RDONLY);
  double t = Timer::now();
  int64_t len = count_shard(0, fd);
  double sec = Timer::now() - t;
  printf("count_shard %.3f sec %.2f MB/s\n", sec, len / sec / 1e6);
  */

  int opt;
  int count_only = 0;
  bool merge_only = false;
  while ((opt = getopt(argc, argv, "c:kmo:p:s:t:v")) != -1)
  {
    switch (opt)
    {
      case 'c':
        count_only = atoi(optarg);
        break;
      case 'k':
        g_keep = true;
        break;
      case 'm':
        merge_only = true;
        break;
      case 'o':
        g_output = optarg;
        break;
      case 'p':  // Path for temp shard files
        shard_dir = optarg;
        break;
      case 's':
        kShards = atoi(optarg);
        break;
      case 't':
        kThreads = atoi(optarg);
        break;
      case 'v':
        g_verbose = true;
        break;
    }
  }

  if (count_only > 0 || merge_only)
  {
    g_keep = true;
    g_verbose = true;
    count_only = std::min(count_only, kShards);

    if (count_only > 0)
    {
      count_shards(count_only);
    }

    if (merge_only)
    {
      merge();
    }
  }
  else
  {
    // Run all three steps
    Timer timer;
    LOG_INFO << argc - optind << " input files, " << kShards << " shards, "
             << "output " << g_output <<" , temp " << shard_dir;
    int64_t input = 0;
    input = shard_(argc, argv);
    count_shards(kShards);
    int64_t output_size = merge();
    LOG_INFO << "All done " << timer.report(input) << " output " << output_size;
  }
}
