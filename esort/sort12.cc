// version 11: sort 100M doubles
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <datetime/Timestamp.h>
#include <thread/ThreadPool.h>
#include <thread/BlockingQueue.h>

#include <algorithm>
#include <vector>

#include <assert.h>
#include <stdio.h>

#include <sys/resource.h>


using muduo::Timestamp;
typedef double ElementType;

struct Source
{
  const std::vector<ElementType>* data;
  int first, last;

  bool read(ElementType* value)
  {
    if (first < last)
    {
      *value = (*data)[first];
      ++first;
      return true;
    }
    else
    {
      return false;
    }
  }

};

struct Record
{
  ElementType value;
  Source* source;

  Record(Source& src)
    : source(&src)
  {
    next();
  }

  bool next()
  {
    return source->read(&value);
  }

  bool operator<(const Record& rhs) const
  {
    return value > rhs.value;
  }
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
    fflush(file_);
    fdatasync(fileno(file_));
    fclose(file_);
  }

  void writeRecord(ElementType x)
  {
    fwrite_unlocked(&x, 1, sizeof x, file_);
  }

  void sync()
  {
    fdatasync(fileno(file_));
  }

 private:
  FILE* file_;
  char buffer_[4*1024*1024];
};

typedef std::vector<ElementType> Batch;
typedef boost::shared_ptr<Batch> BatchPtr;

void output(muduo::BlockingQueue<BatchPtr>* queueOut,
            muduo::BlockingQueue<BatchPtr>* queueIn)
{
  OutputFile out("output");
  while (BatchPtr batch = queueOut->take())
  {
    Timestamp start = Timestamp::now();
    printf("%s write %zd records\n", start.toString().c_str(), batch->size());

    for (Batch::iterator it = batch->begin();
        it != batch->end();
        ++it)
    {
      out.writeRecord(*it);
    }
    out.sync();

    queueIn->put(batch);
  }
  printf("flushing\n");
}

void merge(const std::vector<ElementType>& data, const int kChunkSize)
{
  const int nChunks = (data.size() + kChunkSize - 1) / kChunkSize;
  printf("merge %d chunks\n", nChunks);

  std::vector<Source> sources;
  sources.reserve(nChunks);
  std::vector<Record> keys;
  keys.reserve(nChunks);
  
  size_t first = 0;
  for (first = 0; first + kChunkSize <= data.size(); first += kChunkSize)
  {
    Source src = { &data, first, first + kChunkSize };
    sources.push_back(src);
    Record rec(sources.back());
    keys.push_back(rec);
  }
  if (first < data.size())
  {
    assert(data.size() - first < kChunkSize);
    Source src = { &data, first, data.size()};
    sources.push_back(src);
    Record rec(sources.back());
    keys.push_back(rec);
  }
  assert(sources.size() == nChunks);

  muduo::BlockingQueue<BatchPtr> queueOut, queueIn;
  muduo::ThreadPool threadPool;
  threadPool.start(1);
  threadPool.run(boost::bind(output, &queueOut, &queueIn));
  const int batchSize = 1000 * 1000 * 10;

  {
    BatchPtr batch(new Batch);
    batch->reserve(batchSize);
    queueIn.put(batch);
  }

  {
    BatchPtr batch(new Batch);
    batch->reserve(batchSize);
    queueIn.put(batch);
  }

  {
    BatchPtr batch(new Batch);
    batch->reserve(batchSize);
    queueIn.put(batch);
  }

  BatchPtr batch(new Batch);
  batch->reserve(batchSize);

  std::make_heap(keys.begin(), keys.end());
  ElementType current = keys.front().value;
  while (!keys.empty())
  {
    std::pop_heap(keys.begin(), keys.end());
    assert(current <= keys.back().value);
    if (current > keys.back().value)
    {
      abort();
    }

    batch->push_back(keys.back().value);
    if (batch->size() >= batchSize)
    {
      queueOut.put(batch);
      batch = queueIn.take();
      batch->clear();
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
  queueOut.put(batch);
  queueOut.put(BatchPtr());
  printf("flushing\n");
}

int main(int argc, char* argv[])
{
  {
    // set max virtual memory to 3GB.
    size_t kOneGB = 1024*1024*1024;
    rlimit rl = { 3.0*kOneGB, 3.0*kOneGB };
    setrlimit(RLIMIT_AS, &rl);
  }

  std::vector<ElementType> data;

  Timestamp start = Timestamp::now();

  {
    data.resize(1000*1000*100);
    for (std::vector<ElementType>::iterator it = data.begin();
        it != data.end();
        ++it)
    {
      *it = drand48();
    }
  }

  Timestamp readDone = Timestamp::now();
  printf("%zd\nread  %f\n", data.size(), timeDifference(readDone, start));

  int kChunkSize = atoi(argv[1]);
  printf("sort %zd records with chunk size %d\n", data.size(), kChunkSize);

  int i = 0;
  for (i = 0; i + kChunkSize <= data.size(); i += kChunkSize)
  {
    std::sort(data.begin() + i, data.begin() + i + kChunkSize);
  }
  printf("%d\n%zd\n", i, data.size());

  std::sort(data.begin() + i, data.end());
  Timestamp sortDone = Timestamp::now();
  printf("sort  %f\n", timeDifference(sortDone, readDone));

  merge(data, kChunkSize);
  Timestamp mergeDone = Timestamp::now();
  printf("merge %f\n", timeDifference(mergeDone, sortDone));

  printf("sort & merge %f\n", timeDifference(mergeDone, readDone));
  // getchar();
}
