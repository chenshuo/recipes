#ifndef MUDUO_BASE_ASYNCLOGGINGDOUBLEBUFFERING_H
#define MUDUO_BASE_ASYNCLOGGINGDOUBLEBUFFERING_H

#include "LogStream.h"

#include "thread/BlockingQueue.h"
#include "thread/BoundedBlockingQueue.h"
#include "thread/CountDownLatch.h"
#include "thread/Mutex.h"
#include "thread/Thread.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace muduo
{

class AsyncLoggingDoubleBuffering : boost::noncopyable
{
 public:
  typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer;
  typedef boost::ptr_vector<Buffer> BufferVector;
  typedef BufferVector::auto_type BufferPtr;

  AsyncLoggingDoubleBuffering(const string& basename, // FIXME: StringPiece
                              size_t rollSize,
                              int flushInterval = 3)
    : flushInterval_(flushInterval),
      running_(false),
      basename_(basename),
      rollSize_(rollSize),
      thread_(boost::bind(&AsyncLoggingDoubleBuffering::threadFunc, this), "Logging"),
      latch_(1),
      mutex_(),
      cond_(mutex_),
      currentBuffer_(new Buffer),
      nextBuffer_(new Buffer),
      buffers_()
  {
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
  }

  ~AsyncLoggingDoubleBuffering()
  {
    if (running_)
    {
      stop();
    }
  }

  void append(const char* logline, int len)
  {
    muduo::MutexLockGuard lock(mutex_);
    if (currentBuffer_->avail() > len)
    {
      currentBuffer_->append(logline, len);
    }
    else
    {
      buffers_.push_back(currentBuffer_.release());

      if (nextBuffer_)
      {
        currentBuffer_ = boost::ptr_container::move(nextBuffer_);
      }
      else
      {
        currentBuffer_.reset(new Buffer); // Rarely happens
      }
      currentBuffer_->append(logline, len);
      cond_.notify();
    }
  }

  void start()
  {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop()
  {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

 private:

  void threadFunc()
  {
    assert(running_ == true);
    latch_.countDown();
    LogFile output(basename_, rollSize_, false);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    boost::ptr_vector<Buffer> buffersToWrite;
    buffersToWrite.reserve(16);
    while (running_)
    {
      assert(newBuffer1 && newBuffer1->length() == 0);
      assert(newBuffer2 && newBuffer2->length() == 0);
      assert(buffersToWrite.empty());

      {
        muduo::MutexLockGuard lock(mutex_);
        if (!buffers_.empty())
        {
          cond_.waitForSeconds(flushInterval_);
        }
        buffers_.push_back(currentBuffer_.release());
        currentBuffer_ = boost::ptr_container::move(newBuffer1);
        buffersToWrite.swap(buffers_);
        if (!nextBuffer_)
        {
          nextBuffer_ = boost::ptr_container::move(newBuffer2);
        }
      }

      assert(!buffersToWrite.empty());

      if (buffersToWrite.size() > 25)
      {
        const char* dropMsg = "Dropped log messages\n";
        fprintf(stderr, "%s", dropMsg);
        output.append(dropMsg, strlen(dropMsg));
        buffersToWrite.erase(buffersToWrite.begin(), buffersToWrite.end() - 2);
      }

      for (size_t i = 0; i < buffersToWrite.size(); ++i)
      {
        // FIXME: use unbuffered stdio FILE ? or use ::writev ?
        output.append(buffersToWrite[i].data(), buffersToWrite[i].length());
      }

      if (buffersToWrite.size() > 2)
      {
        // drop non-bzero-ed buffers, avoid trashing
        buffersToWrite.resize(2);
      }

      if (!newBuffer1)
      {
        assert(!buffersToWrite.empty());
        newBuffer1 = buffersToWrite.pop_back();
        newBuffer1->reset();
      }

      if (!newBuffer2)
      {
        assert(!buffersToWrite.empty());
        newBuffer2 = buffersToWrite.pop_back();
        newBuffer2->reset();
      }

      buffersToWrite.clear();
      output.flush();
    }
    output.flush();
  }

  const int flushInterval_;
  bool running_;
  string basename_;
  size_t rollSize_;
  muduo::Thread thread_;
  muduo::CountDownLatch latch_;
  muduo::MutexLock mutex_;
  muduo::Condition cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
};

}
#endif  // MUDUO_BASE_ASYNCLOGGINGDOUBLEBUFFERING_H
