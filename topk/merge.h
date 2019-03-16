#pragma once

#include "timer.h"

#include <vector>

#include <fcntl.h>
#include <sys/stat.h>

extern bool g_keep;
extern const char* g_output;

class TextInput
{
 public:
  explicit TextInput(const char* filename, int buffer_size = 8 * 1024 * 1024)
    : fd_(::open(filename, O_RDONLY)),
      buffer_size_(buffer_size),
      block_(new Block)
  {
    assert(fd_ >= 0);
    block_->data.reset(new char[buffer_size_]);
    refill();
  }

  ~TextInput()
  {
    ::close(fd_);
  }

  absl::string_view line() const { return line_; }

  bool next(int64_t* count)
  {
    // EOF
    if (block_->records.empty())
    {
      return false;
    }

    if (index_ < block_->records.size())
    {
      const Record& rec = block_->records[index_];
      *count = rec.count;
      line_ = absl::string_view(block_->data.get() + rec.offset, rec.len);
      ++index_;
      return true;
    }
    else
    {
      refill();
      index_ = 0;
      return next(count);
    }
  }

 private:

  struct Record
  {
    int64_t count = 0;
    int32_t offset = 0, len = 0;
  };

  struct Block
  {
    std::unique_ptr<char[]> data;
    std::vector<Record> records;
  };

  void refill()
  {
    block_->records.clear();
    char* data = block_->data.get();
    ssize_t nr = ::pread(fd_, data, buffer_size_, pos_);
    if (nr > 0)
    {
      char* start = data;
      size_t len = nr;
      char* nl = static_cast<char*>(::memchr(start, '\n', len));
      while (nl)
      {
        Record rec;
        rec.count = strtol(start, NULL, 10);
        rec.offset = start - data;
        rec.len = nl - start + 1;
        block_->records.push_back(rec);
        start = nl+1;
        len -= rec.len;
        nl = static_cast<char*>(::memchr(start, '\n', len));
      }
      pos_ += start - data;
    }
  }

  const int fd_;
  const int buffer_size_;
  int64_t pos_ = 0;  // file position
  size_t index_ = 0; // index into block_
  std::unique_ptr<Block> block_;
  absl::string_view line_;

  TextInput(const TextInput&) = delete;
  void operator=(const TextInput&) = delete;
};

class Source  // copyable
{
 public:
  explicit Source(TextInput* in)
    : input_(in)
  {
  }

  bool next()
  {
    return input_->next(&count_);
  }

  bool operator<(const Source& rhs) const
  {
    return count_ < rhs.count_;
  }

  absl::string_view line() const { return input_->line(); }

 private:
  TextInput* input_;  // not owned
  int64_t count_ = 0;
};

int64_t merge(int n)
{
  Timer timer;
  std::vector<std::unique_ptr<TextInput>> inputs;
  std::vector<Source> keys;

  int64_t total = 0;
  for (int i = 0; i < n; ++i)
  {
    char buf[256];
    snprintf(buf, sizeof buf, "count-%05d", i);
    struct stat st;
    if (::stat(buf, &st) == 0)
    {
      total += st.st_size;
      // TODO: select buffer size based on kShards.
      inputs.push_back(std::make_unique<TextInput>(buf));
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

  int64_t lines = 0;
  {
  OutputFile out(g_output);

  std::make_heap(keys.begin(), keys.end());
  while (!keys.empty())
  {
    std::pop_heap(keys.begin(), keys.end());
    out.write(keys.back().line());
    ++lines;

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
  LOG_INFO << "Merging done " << timer.report(total) << " lines " << lines;
  return total;
}


