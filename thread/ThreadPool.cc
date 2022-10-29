// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (giantchen at gmail dot com)

#include "ThreadPool.h"
#include "Exception.h"

#include <boost/bind.hpp>
#include <assert.h>
#include <stdio.h>

using namespace muduo;

ThreadPool::ThreadPool(const std::string& name)
  : mutex_(),
    cond_(mutex_),
    name_(name),
    running_(false)
{
}

ThreadPool::~ThreadPool()
{
  if (running_)
  {
    stop();
  }
}

void ThreadPool::start(int numThreads)
{
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(numThreads);
  for (int i = 0; i < numThreads; ++i)
  {
    char id[32];
    snprintf(id, sizeof id, "%d", i);
    threads_.push_back(new muduo::Thread(
          boost::bind(&ThreadPool::runInThread, this), name_+id));
    threads_[i].start();
  }
}

void ThreadPool::stop()
{
  printf("starting to stop the pool.\n");
  {
    MutexLockGuard lock(mutex_);
    running_ = false;
  }
  cond_.notifyAll();
  for_each(threads_.begin(),
           threads_.end(),
           boost::bind(&muduo::Thread::join, _1));
  threads_.clear();
  printf("stop the pool successfully.\n");
}

void ThreadPool::run(const Task& task)
{
  if (threads_.empty())
  {
    task();
    return;
  }
  {
    MutexLockGuard lock(mutex_);
    queue_.push_back(task);
  }
  cond_.notify();
}

ThreadPool::Task ThreadPool::take()
{
  MutexLockGuard lock(mutex_);
  while (queue_.empty() && running_)
  {
    cond_.wait();
  }
  Task task;
  if(!queue_.empty())
  {
    task = queue_.front();
    queue_.pop_front();
  }
  return task;
}

void ThreadPool::runInThread()
{
  try
  {
    while (running_)
    {
      Task task(take());
      if (task)
      {
        task();
      }
    }
  }
  catch (const Exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
    abort();
  }
  catch (const std::exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch (...)
  {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
    abort();
  }
}



// // bind cores
// const int targetCore = bindcores_[i];
// cpu_set_t cpuset;
// CPU_ZERO(&cpuset);
// CPU_SET(targetCore, &cpuset);
// int ret = pthread_setaffinity_np(thread_[i], sizeof(cpu_set_t), &cpuset); // or std::thread::native_handle()
//                                                                           // or pthread_self()
// if (ret != 0)
// {
//     printf("[ThreadPool] thread %zu bind core %d  failed\n", i, targetCore);
// }
// else
// {
//     printf("[ThreadPool] thread %zu bind core %d  success\n", i, targetCore);
// }

// // 线程优先级
// sched_param sp;
// sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
// ret = pthread_setschedparam(thread_[i], SCHED_FIFO, &sp);
// if (ret != 0)
// {
//     printf("[ThreadPool] thread %zu set  priority %d  failed\n", i, sp.sched_priority);
// }
// else
// {
//     printf("[ThreadPool] thread %zu set  priority %d  success\n", i, sp.sched_priority);
// }
