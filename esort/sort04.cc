// version 04: pipeline impl to sort large files.
// reduce memory usage by replacing std::string with fixed-length char array
// larger IO buffer

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <datetime/Timestamp.h>
#include <thread/ThreadPool.h>
#include <thread/BlockingQueue.h>

#include <algorithm>
#include <vector>

#include <assert.h>
#include <stdio.h>

#include <sys/resource.h>


using muduo::Timestamp;

class InputFile : boost::noncopyable
{
 public:
  InputFile(const char* filename)
    : file_(fopen(filename, "rb"))
  {
    assert(file_);
    setbuffer(file_, buffer_, sizeof buffer_);
    // posix_fadvise for initial input
    // http://lwn.net/Articles/449420/
    // http://blog.mikemccandless.com/2010/06/lucene-and-fadvisemadvise.html
  }

  ~InputFile()
  {
    fclose(file_);
  }

  int read(char* buf, int size)
  {
    return fread_unlocked(buf, 1, size, file_);
  }

 private:
  FILE* file_;
  char buffer_[4*1024*1024];
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
    // posix_fadvise for final output
  }

  ~OutputFile()
  {
    // how to use OS buffers sensibly ?
    // fdatasync(fileno_unlocked(file_));
    fclose(file_);
  }

  void writeRecord(char (&record)[kRecordSize])
  {
    fwrite_unlocked(record, 1, kRecordSize, file_);
  }

 private:
  FILE* file_;
  char buffer_[4*1024*1024];
};

const bool kUseReadLine = false;
const int kBatchRecords = 10000000;

struct Record
{
  char data[kRecordSize];
};

typedef std::vector<Record> Data;

void readInput(InputFile& in, Data* data)
{
  int64_t totalSize = 0;
  data->clear();
  data->reserve(kBatchRecords);

  for (int i = 0; i < kBatchRecords; ++i)
  {
    Record record;
    if (int n = in.read(record.data, sizeof record.data))
    {
      assert (n == kRecordSize);
      totalSize += n;
      data->push_back(record);
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

  Key(const Record& record, int idx)
    : index(idx)
  {
    memcpy(key, record.data, sizeof key);
  }

  bool operator<(const Key& rhs) const
  {
    return memcmp(key, rhs.key, sizeof key) < 0;
  }
};

void sortWithKeys(const Data& data, std::vector<Key>* keys)
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
        out.writeRecord(data_[it->index].data);
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

struct Source
{
  char data[kRecordSize];
  InputFile* input;

  Source(InputFile* in)
    : input(in)
  {
  }

  bool next()
  {
    return input->read(data, sizeof data) == kRecordSize;
  }

  bool operator<(const Source& rhs) const
  {
    // make_heap to build min-heap, for merging
    return memcmp(data, rhs.data, kKeySize) > 0;
  }
};

void merge(const int batch)
{
  printf("merge %d files\n", batch);

  boost::ptr_vector<InputFile> inputs;
  std::vector<Source> keys;

  for (int i = 0; i < batch; ++i)
  {
    char filename[128];
    snprintf(filename, sizeof filename, "tmp%d", i);
    inputs.push_back(new InputFile(filename));
    Source rec(&inputs.back());
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

  printf("pid %d\n", getpid());

  {
    // set max virtual memory to 3GB.
    size_t kOneGB = 1024*1024*1024;
    rlimit rl = { 3.0*kOneGB, 3.0*kOneGB };
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
