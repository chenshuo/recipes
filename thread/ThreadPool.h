// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (giantchen at gmail dot com)

#ifndef MUDUO_BASE_THREADPOOL_H
#define MUDUO_BASE_THREADPOOL_H

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <deque>
#include <future>

namespace muduo
{

class ThreadPool : boost::noncopyable
{
 public:
  typedef boost::function<void ()> Task;

  explicit ThreadPool(const std::string& name = std::string());
  ~ThreadPool();

  void start(int numThreads);
  void stop();

  void run(const Task& f);

  template<typename Func, typename... Args>
  inline auto run_future(Func&& func, Args&&... args) -> std::future<typename std::result_of<Func(Args...)>::type>
  {
    if (threads_.empty()) {
      func(args...);
    }
    using ret_type = typename std::result_of<Func(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<ret_type()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    auto ret = task->get_future();
    do {
      MutexLockGuard lock(mutex_);
      if (!running_) {
        throw std::runtime_error("enqueue on stopped ThreadPool");
      }
      queue_.push_back([task]() { (*task)(); });
    } while (0);
    cond_.notify();
    return std::move(ret);
  }

 private:
  void runInThread();
  Task take();

  MutexLock mutex_;
  Condition cond_;
  std::string name_;
  boost::ptr_vector<muduo::Thread> threads_;
  std::deque<Task> queue_;
  bool running_;
};

}

#endif
