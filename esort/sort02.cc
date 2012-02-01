// version 02: first impl to sort large files.
// sort and merge
// 30% faster than sort(1) for all 1GB 5GB 10GB files

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

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

#define TMP_DIR "/tmp/"

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

const int kRecordSize = 100;
const int kKeySize = 10;

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

  void writeRecord(char (&record)[kRecordSize])
  {
    fwrite_unlocked(record, 1, kRecordSize, file_);
  }


 private:
  FILE* file_;
  char buffer_[64*1024];
};

const bool kUseReadLine = false;
const int kBatchRecords = 10000000;

void readInput(InputFile& in, std::vector<string>* data)
{
  int64_t totalSize = 0;
  data->clear();
  data->reserve(kBatchRecords);

  for (int i = 0; i < kBatchRecords; ++i)
  {
    char buf[kRecordSize];
    if (int n = in.read(buf, sizeof buf))
    {
      assert (n == kRecordSize);
      totalSize += n;
      data->push_back(string(buf, n));
    }
    else
    {
      break;
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
  // printf("make keys %f\n", data.size(), timeDifference(Timestamp::now(), start));

  std::sort(keys->begin(), keys->end());
}

int sortSplit(const char* filename)
{
  std::vector<string> data;
  // read
  InputFile in(filename);
  int batch = 0;
  while(true)
  {
    Timestamp startThis = Timestamp::now();
    readInput(in, &data);

    Timestamp readDone = Timestamp::now();
    printf("%zd\nread  %f\n", data.size(), timeDifference(readDone, startThis));
    if (data.empty())
    {
      break;
    }

    std::vector<Key> keys;
    sort(data, &keys);

    Timestamp sortDone = Timestamp::now();
    printf("sort  %f\n", timeDifference(sortDone, readDone));

    // output
    {
      char output[256];
      snprintf(output, sizeof output, TMP_DIR "tmp%d", batch++);

      OutputFile out(output);
      for (std::vector<Key>::iterator it = keys.begin();
          it != keys.end();
          ++it)
      {
        out.writeLine(data[it->index]);
      }
    }
    Timestamp writeDone = Timestamp::now();
    printf("write %f\n", timeDifference(writeDone, sortDone));
  }
  return batch;
}

struct Record
{
  char data[kRecordSize];
  InputFile* input;

  Record(InputFile* in)
    : input(in)
  {
  }

  bool next()
  {
    return input->read(data, sizeof data) == kRecordSize;
  }

  bool operator<(const Record& rhs) const
  {
    // make_heap to build min-heap, for merging
    return memcmp(data, rhs.data, kKeySize) > 0;
  }
};

void merge(const int batch)
{
  printf("merge %d files\n", batch);

  boost::ptr_vector<InputFile> inputs;
  std::vector<Record> keys;

  for (int i = 0; i < batch; ++i)
  {
    char filename[128];
    snprintf(filename, sizeof filename, TMP_DIR "tmp%d", i);
    inputs.push_back(new InputFile(filename));
    Record rec(&inputs.back());
    if (rec.next())
    {
      keys.push_back(rec);
    }
  }

  OutputFile out("output");
  std::make_heap(keys.begin(), keys.end());
  while (!keys.empty())
  {
    std::pop_heap(keys.begin(), keys.end());
    out.writeRecord(keys.back().data);

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

int main(int argc, char* argv[])
{
  bool kKeepIntermediateFiles = false;

  {
    // set max virtual memory to 3GB.
    size_t kOneGB = 1024*1024*1024;
    rlimit rl = { 3.0*kOneGB, 3.0*kOneGB };
    setrlimit(RLIMIT_AS, &rl);
  }

  Timestamp start = Timestamp::now();

  // sort
  int batch = sortSplit(argv[1]);
  Timestamp sortDone = Timestamp::now();
  printf("sortSplit %f\n", timeDifference(sortDone, start));

  if (batch == 1)
  {
    unlink("output");
    rename(TMP_DIR "tmp0", "output");
  }
  else
  {
    // merge
    merge(batch);
    Timestamp mergeDone = Timestamp::now();
    printf("mergeSplit %f\n", timeDifference(mergeDone, sortDone));
  }

  if (!kKeepIntermediateFiles)
  {
    for (int i = 0; i < batch; ++i)
    {
      char tmp[256];
      snprintf(tmp, sizeof tmp, TMP_DIR "tmp%d", i);
      unlink(tmp);
    }
  }

  printf("total %f\n", timeDifference(Timestamp::now(), start));
}
