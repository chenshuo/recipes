// version 03: pipeline impl to sort large files.
// overlap IO with computing using threads.

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <datetime/Timestamp.h>
#include <thread/ThreadPool.h>
#include <thread/BlockingQueue.h>

#include <algorithm>
#include <string>
//#include <ext/vstring.h>
#include <vector>

#include <assert.h>
#include <stdio.h>

#include <sys/resource.h>


typedef std::string string;
// typedef __gnu_cxx::__sso_string string;

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

void sortWithKeys(const std::vector<string>& data, std::vector<Key>* keys)
{
  Timestamp start = Timestamp::now();
  keys->clear();
  keys->reserve(data.size());

  for (size_t i = 0; i < data.size(); ++i)
  {
    keys->push_back(Key(data[i], i));
  }
  // printf("make keys %f\n", data.size(), timeDifference(Timestamp::now(), start));

  std::sort(keys->begin(), keys->end());
}

typedef std::vector<string> Data;

class Task;
typedef boost::shared_ptr<Task> TaskPtr;

class Task : public boost::enable_shared_from_this<Task>
{
 public:
  Task(muduo::BlockingQueue<TaskPtr>* queue)
    : queue_(queue),
      id_(s_created++),
      sorted_(false)
  {
    assert(muduo::CurrentThread::isMainThread());
    printf("Task %d\n", id_);
  }

  ~Task()
  {
    printf("~Task %d\n", id_);
  }

  bool read(InputFile& in)
  {
    assert(muduo::CurrentThread::isMainThread());
    sorted_ = false;
    Timestamp startRead = Timestamp::now();
    printf("task %d start read  %s\n", id_, startRead.toString().c_str());
    readInput(in, &data_);
    Timestamp readDone = Timestamp::now();
    printf("task %d read done   %s  %f  %zd \n",
        id_, readDone.toString().c_str(), timeDifference(readDone, startRead), data_.size());
    return !data_.empty();
  }

  void sort()
  {
    // assert(!muduo::CurrentThread::isMainThread());
    assert(!sorted_);
    Timestamp startSort = Timestamp::now();
    printf("task %d start sort  %s\n", id_, startSort.toString().c_str());
    sortWithKeys(data_, &keys_);
    sorted_ = true;
    Timestamp sortDone = Timestamp::now();
    printf("task %d sort done   %s  %f\n",
        id_, sortDone.toString().c_str(), timeDifference(sortDone, startSort));
    queue_->put(shared_from_this());
  }

  void write(int batch)
  {
    assert(muduo::CurrentThread::isMainThread());
    assert(sorted_);

    Timestamp startWrite = Timestamp::now();
    printf("task %d start write %s\n", id_, startWrite.toString().c_str());
    {
      char output[256];
      snprintf(output, sizeof output, "tmp%d", batch);

      OutputFile out(output);
      for (std::vector<Key>::iterator it = keys_.begin();
          it != keys_.end();
          ++it)
      {
        out.writeLine(data_[it->index]);
      }
    }
    Timestamp writeDone = Timestamp::now();
    printf("task %d write done  %s  %f\n",
        id_, writeDone.toString().c_str(), timeDifference(writeDone, startWrite));
  }

  const Data& data() const
  {
    return data_;
  }

 private:
  Data data_;
  std::vector<Key> keys_;
  muduo::BlockingQueue<TaskPtr>* queue_;
  int id_;
  bool sorted_;

  static int s_created;
};

int Task::s_created = 0;


int sortSplit(const char* filename)
{
  // read
  InputFile in(filename);
  muduo::BlockingQueue<TaskPtr> queue;
  muduo::ThreadPool threadPool;
  threadPool.start(2);
  int active = 0;

  // initialize
  {
    TaskPtr task(new Task(&queue));
    if (task->read(in))
    {
      threadPool.run(boost::bind(&Task::sort, task));
      active++;
    }
    else
    {
      return 0;
    }

    TaskPtr task2(new Task(&queue));
    if (task2->read(in))
    {
      threadPool.run(boost::bind(&Task::sort, task2));
      active++;
    }
  }

  int batch = 0;
  while (active > 0)
  {
    TaskPtr task = queue.take();
    active--;

    task->write(batch++);

    if (task->read(in))
    {
      threadPool.run(boost::bind(&Task::sort, task));
      active++;
    }
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
    snprintf(filename, sizeof filename, "tmp%d", i);
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
    // set max virtual memory to 4GB.
    size_t kOneGB = 1024*1024*1024;
    rlimit rl = { 4.0*kOneGB, 4.0*kOneGB };
    setrlimit(RLIMIT_AS, &rl);
  }

  Timestamp start = Timestamp::now();
  printf("sortSplit start %s\n", start.toString().c_str());

  // sort
  int batch = sortSplit(argv[1]);
  Timestamp sortDone = Timestamp::now();
  printf("sortSplit done %f\n", timeDifference(sortDone, start));

  if (batch == 0)
  {
  }
  else if (batch == 1)
  {
    rename("tmp0", "output");
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
      snprintf(tmp, sizeof tmp, "tmp%d", i);
      unlink(tmp);
    }
  }

  printf("total %f\n", timeDifference(Timestamp::now(), start));
}
