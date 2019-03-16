#pragma once

#include <string>
#include <fcntl.h>

class SegmentInput
{
 public:
  explicit SegmentInput(const char* filename, int bufsize=kBufferSize)
    : filename_(filename),
      fd_(::open(filename, O_RDONLY)),
      buffer_size_(bufsize),
      data_(new char[buffer_size_])
  {
    refill();
  }

  const std::string& filename() const { return filename_; }
  int64_t tell() const { return offset_; }
  const std::string& current_word() const { return word_; }
  int64_t current_count() const { return count_; }

  bool next()
  {
    if (avail_ <= 0)
      return false;
    char* nl = static_cast<char*>(::memchr(start_, '\n', avail_));
    if (nl)
    {
      char* tab = static_cast<char*>(::memchr(start_, '\t', nl - start_));
      if (tab)
      {
        count_ = strtol(tab+1, NULL, 10);
        word_ = std::string_view(start_, tab-start_);

        int len = nl - start_ + 1;
        avail_ -= len;
        offset_ += len;
        assert(avail_ >= 0);
        if (avail_ == 0)
        {
          refill();
        }
        else
        {
          start_ += len;
        }
        return true;
      }
      else
      {
        avail_ = 0;
        assert(0);
        return false;
      }
    }
    else
    {
      refill();
      return next();
    }
  }

 private:
  void refill()
  {
    start_ = data_.get();
    avail_ = ::pread(fd_, start_, buffer_size_, offset_);
  }

  const std::string filename_;
  const int fd_;
  const int buffer_size_;
  int64_t offset_ = 0;  // file position

  char* start_ = nullptr;
  int avail_ = 0;
  std::unique_ptr<char[]> data_;

  std::string word_;
  int64_t count_ = 0;

  SegmentInput(const SegmentInput&) = delete;
  void operator=(const SegmentInput&) = delete;
};


