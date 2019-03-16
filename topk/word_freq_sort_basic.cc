/* sort word by frequency, sorting version.

   1. read input files, sort every 1GB to segment files
      word \t count  -- sorted by word
   2. read all segment files, do merging & counting, when count map > 10M keys, output to count files, each word goes to one count file only.
      count \t word  -- sorted by count
   3. read all count files, do merging and output
*/

#include "file.h"
#include "input.h"
#include "merge.h"
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

int64_t sort_segments(int* count, int fd)
{
  Timer timer;
  const int64_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  if (g_verbose)
    printf("  file size %ld\n", file_size);
  int64_t offset = 0;
  while (offset < file_size)
  {
    double t = now();
    const int64_t len = std::min(file_size - offset, 1024 * 1000 * 1000L);
    if (g_verbose)
      printf("    reading segment %d: offset %ld len %ld", *count, offset, len);
    std::unique_ptr<char[]> buf(new char[len]);
    const ssize_t nr = ::pread(fd, buf.get(), len, offset);
    double sec = now() - t;
    if (g_verbose)
    printf(" %.3f sec %.3f MB/s\n", sec, nr / sec / 1000 / 1000);

    // TODO: move to another thread
    t = now();
    const char* const start = buf.get();
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
    if (g_verbose)
    printf("    parse %.3f sec %ld items %ld bytes\n", now() - t, items.size(), p - start);

    t = now();
    std::sort(items.begin(), items.end());
    if (g_verbose)
    printf("    sort %.3f sec\n", now() - t);

    t = now();
    char name[256];
    snprintf(name, sizeof name, "%s/segment-%05d", segment_dir, *count);
    ++*count;
    int unique = 0;
    {
    // TODO: replace with OutputFile
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
    if (g_verbose)
    printf("    unique %.3f sec %d\n", now() - t, unique);
    LOG_INFO << "  wrote " << name;
  }
  LOG_INFO << "  file done " << timer.report(file_size);
  return file_size;
}

int input(int argc, char* argv[], int64_t* total_in = nullptr)
{
  int count = 0;
  int64_t total = 0;
  Timer timer;
  for (int i = optind; i < argc; ++i)
  {
    LOG_INFO << "Reading input file " << argv[i];

    int fd = open(argv[i], O_RDONLY);
    if (fd >= 0)
    {
      total += sort_segments(&count, fd);
      ::close(fd);
    }
    else
      perror("open");
  }
  LOG_INFO << "Reading done " << count << " segments " << timer.report(total);
  if (total_in)
    *total_in = total;
  return count;
}

// ======= combine =======

class Segment  // copyable
{
 public:
  string_view word;

  explicit Segment(SegmentInput* in)
    : in_(in)
  {
  }

  bool next()
  {
    if (in_->next())
    {
      word = in_->current_word();
      return true;
    }
    else
      return false;
  }

  bool operator<(const Segment& rhs) const
  {
    return word > rhs.word;
  }

  int64_t count() const { return in_->current_count(); }

 private:
  SegmentInput* in_;
};

class CountOutput
{
 public:
  CountOutput();

  void add(string_view word, int64_t count)
  {
    if (block_->add(word, count))
    {
    }
    else
    {
      // TODO: Move to another thread.
      block_->output(merge_count_);
      ++merge_count_;
      block_.reset(new Block);
      if (!block_->add(word, count))
      {
        abort();
      }
    }
  }

  int finish()
  {
    block_->output(merge_count_);
    ++merge_count_;
    return merge_count_;
  }

 private:
  struct Count
  {
    int64_t count = 0;
    int32_t offset = 0, len = 0;
  };

  struct Block
  {
    std::unique_ptr<char[]> data { new char[kSize] };
    vector<Count> counts;
    int start = 0;
    static const int kSize = 512 * 1000 * 1000;


    bool add(string_view word, int64_t count)
    {
      if (static_cast<size_t>(kSize - start) >= word.size())
      {
        memcpy(data.get() + start, word.data(), word.size());
        Count c;
        c.count = count;
        c.offset = start;
        c.len = word.size();
        counts.push_back(c);
        start += word.size();
        return true;
      }
      else
        return false;
    }

    void output(int n)
    {
      Timer t;
      char buf[256];
      snprintf(buf, sizeof buf, "count-%05d", n);
      LOG_INFO << "  writing " << buf << " of " << counts.size() << " words";
      std::sort(counts.begin(), counts.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.count > rhs.count;
      });
      int64_t file_size = 0;
      {
      OutputFile out(buf);

      for (const auto& c : counts)
      {
        out.writeWord(c.count, string_view(data.get() + c.offset, c.len));
      }
      file_size = out.tell();
      }
      LOG_DEBUG << "  done " << t.report(file_size);
    }
  };

  std::unique_ptr<Block> block_;
  int merge_count_ = 0;
};

CountOutput::CountOutput()
  : block_(new Block)
{
}

int combine(int count)
{
  Timer timer;
  std::vector<std::unique_ptr<SegmentInput>> inputs;
  std::vector<Segment> keys;

  int64_t total = 0;
  for (int i = 0; i < count; ++i)
  {
    char buf[256];
    snprintf(buf, sizeof buf, "%s/segment-%05d", segment_dir, i);
    struct stat st;
    if (::stat(buf, &st) == 0)
    {
      total += st.st_size;
      inputs.emplace_back(new SegmentInput(buf));
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
  string last = "Chen Shuo";
  int64_t last_count = 0, total_count = 0;
  int64_t lines_in = 0, lines_out = 0;
  CountOutput out;
  std::make_heap(keys.begin(), keys.end());

  while (!keys.empty())
  {
    std::pop_heap(keys.begin(), keys.end());
    lines_in++;
    total_count += keys.back().count();

    if (keys.back().word != last)
    {
      if (last_count > 0)
      {
        assert(last > keys.back().word);
        lines_out++;
        out.add(last, last_count);
      }

      last = keys.back().word;
      last_count = keys.back().count();
    }
    else
    {
      last_count += keys.back().count();
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

  if (last_count > 0)
  {
    lines_out++;
    out.add(last, last_count);
  }
  int m = out.finish();

  LOG_INFO << "total count " << total_count << ", lines in " << lines_in << " out " << lines_out;
  LOG_INFO << "Combine done " << timer.report(total);
  return m;
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
      LOG_INFO << "wrote " << count << " segments";
    }
    if (count_only)
    {
      int m = combine(count_only);
      LOG_INFO << "wrote " << m << " counts";
    }
    if (merge_only)
    {
      merge(merge_only);
    }
  }
  else
  {
    int64_t total = 0;
    Timer timer;
    int count = input(argc, argv, &total);
    int m = combine(count);
    merge(m);
    LOG_INFO << "All done " << timer.report(total);
  }
}
