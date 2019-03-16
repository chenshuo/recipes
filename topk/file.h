#pragma once

#include <stdio.h>
#include <fstream>
#include <memory>
#include <string>
#include "absl/strings/string_view.h"
#include "muduo/base/Logging.h"  // CHECK_NOTNULL

const int kBufferSize = 1024 * 1024;

// Wrappers FILE* from stdio.
class File
{
 public:
  int64_t tell() const
  {
    return ::ftell(file_);
  }

  void close()
  {
    if (file_)
      ::fclose(file_);
    file_ = nullptr;
    buffer_.reset();
  }

  const std::string& filename() const
  {
    return filename_;
  }

  // https://github.com/coreutils/coreutils/blob/master/src/ioblksize.h
  /* As of May 2014, 128KiB is determined to be the minimium
   * blksize to best minimize system call overhead.
   */

 protected:
  File(const std::string& filename, const char* mode, int bufsize=kBufferSize)
    : filename_(filename),
      file_(CHECK_NOTNULL(::fopen(filename.c_str(), mode))),
      buffer_(CHECK_NOTNULL(new char[bufsize]))
  {
    ::setbuffer(file_, buffer_.get(), bufsize);
  }

  virtual ~File()
  {
    close();
  }

 protected:
  std::string filename_;
  FILE* file_ = nullptr;

 private:
  std::unique_ptr<char[]> buffer_;

  File(const File&) = delete;
  void operator=(const File&) = delete;
};

class InputFile : public File
{
 public:
  explicit InputFile(const char* filename, int bufsize=kBufferSize)
    : File(filename, "r", bufsize)
  {
  }

  bool getline(std::string* output)
  {
    char buf[1024];  // ="" will slow down by 50%!!!
    if (::fgets(buf, sizeof buf, file_))
    {
      *output = buf;
      if (!output->empty() && output->back() == '\n')
      {
        output->resize(output->size()-1);
      }
      return true;
    }
    return false;
  }
};

/*
class InputFile2
{
 public:
  explicit InputFile2(const char* filename, int bufsize=kBufferSize)
    : filename_(filename),
      in_(filename)
  {
    // FIXME: bufsize
  }

  bool getline(std::string* output)
  {
    return static_cast<bool>(std::getline(in_, *output));
  }

  const std::string& filename() const
  {
    return filename_;
  }

 private:
  std::string filename_;
  std::ifstream in_;
};
*/

class OutputFile : public File
{
 public:
  explicit OutputFile(const std::string& filename)
    : File(filename, "w")
  {
  }

  void write(absl::string_view s)
  {
    ::fwrite(s.data(), 1, s.size(), file_);
  }

  void writeWord(int64_t count, absl::string_view word)
  {
    ::fprintf(file_, "%ld\t", count);
    ::fwrite(word.data(), 1, word.size(), file_);
    ::fwrite("\n", 1, 1, file_);
  }

  void appendRecord(absl::string_view s)
  {
    assert(s.size() < 255);
    uint8_t len = s.size();
    ::fwrite(&len, 1, sizeof len, file_);
    ::fwrite(s.data(), 1, len, file_);
    ++items_;
  }

  size_t items()
  {
    return items_;
  }

 private:
  size_t items_ = 0;
};

