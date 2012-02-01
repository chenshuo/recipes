// version 01: only be able to sort 1GB file
// sort with keys
// 30% faster than sort(1) for 1GB file.

#include <boost/noncopyable.hpp>

#include <datetime/Timestamp.h>

#include <algorithm>
#include <string>
#include <ext/vstring.h>
#include <vector>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <sys/resource.h>


// typedef std::string string;
typedef __gnu_cxx::__sso_string string;

using muduo::Timestamp;

class InputFile : boost::noncopyable
{
 public:
  InputFile(const char* filename)
    : file_(fopen(filename, "rb"))
  {
    assert(file_);
    setbuffer(file_, buffer_, sizeof buffer_);
  }

  ~InputFile()
  {
    fclose(file_);
  }

  bool readLine(string* line)
  {
    char buf[256];
    if (fgets_unlocked(buf, sizeof buf, file_))
    {
      line->assign(buf);
      return true;
    }
    else
    {
      return false;
    }
  }

  int read(char* buf, int size)
  {
    return fread_unlocked(buf, 1, size, file_);
  }

 private:
  FILE* file_;
  char buffer_[64*1024];
};

class OutputFile : boost::noncopyable
{
 public:
  OutputFile(const char* filename)
    : file_(fopen(filename, "wb"))
  {
    assert(file_);
    setbuffer(file_, buffer_, sizeof buffer_);
  }

  ~OutputFile()
  {
    fclose(file_);
  }

  void writeLine(const string& line)
  {
    if (line.empty())
    {
      fwrite_unlocked("\n", 1, 1, file_);
    }
    else if (line[line.size() - 1] == '\n')
    {
      fwrite_unlocked(line.c_str(), 1, line.size(), file_);
    }
    else
    {
      fwrite_unlocked(line.c_str(), 1, line.size(), file_);
      fwrite_unlocked("\n", 1, 1, file_);
    }
  }

 private:
  FILE* file_;
  char buffer_[64*1024];
};

const int kRecordSize = 100;
const int kKeySize = 10;
const bool kUseReadLine = false;

void readInput(const char* filename, std::vector<string>* data)
{
  InputFile in(filename);
  string line;
  int64_t totalSize = 0;
  data->reserve(10000000);

  if (kUseReadLine)
  {
    while (in.readLine(&line))
    {
      totalSize += line.size();
      data->push_back(line);
    }
  }
  else
  {
    char buf[kRecordSize];
    while (int n = in.read(buf, sizeof buf))
    {
      totalSize += n;
      line.assign(buf, n);
      data->push_back(line);
    }
  }
}

struct Key
{
  char key[kKeySize];
  int index;

  Key(const string& record, int idx)
    : index(idx)
  {
    memcpy(key, record.data(), sizeof key);
  }

  bool operator<(const Key& rhs) const
  {
    return memcmp(key, rhs.key, sizeof key) < 0;
  }
};

void sort(const std::vector<string>& data, std::vector<Key>* keys)
{
  Timestamp start = Timestamp::now();
  keys->reserve(data.size());

  for (size_t i = 0; i < data.size(); ++i)
  {
    keys->push_back(Key(data[i], i));
  }
  printf("make keys %f\n", timeDifference(Timestamp::now(), start));

  std::sort(keys->begin(), keys->end());
}

int main(int argc, char* argv[])
{
  bool kSortDummyData = false;

  {
    // set max virtual memory to 3GB.
    size_t kOneGB = 1024*1024*1024;
    rlimit rl = { 3.0*kOneGB, 3.0*kOneGB };
    setrlimit(RLIMIT_AS, &rl);
  }

  std::vector<int> dummyData;

  if (kSortDummyData)
  {
    dummyData.resize(10000000);
    for (std::vector<int>::iterator it = dummyData.begin();
        it != dummyData.end();
        ++it)
    {
      *it = rand();
    }
  }

  Timestamp start = Timestamp::now();

  std::vector<string> data;

  // read
  readInput(argv[1], &data);

  Timestamp readDone = Timestamp::now();
  printf("%zd\nread  %f\n", data.size(), timeDifference(readDone, start));

  // sort
  std::vector<Key> keys;
  sort(data, &keys);

  Timestamp sortDone = Timestamp::now();
  printf("sort  %f\n", timeDifference(sortDone, readDone));

  // output
  // char dummyBuf[256];
  {
    OutputFile out("output");
    for (std::vector<Key>::iterator it = keys.begin();
        it != keys.end();
        ++it)
    {
      // memcpy(dummyBuf, data[it->index].data(), 100);

      out.writeLine(data[it->index]);
    }
  }
  Timestamp writeDone = Timestamp::now();
  printf("write %f\n", timeDifference(writeDone, sortDone));

  printf("total %f\n", timeDifference(writeDone, start));
}
