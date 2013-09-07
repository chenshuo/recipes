#include "../Condition.h"
#include "../Mutex.h"
#include "../Thread.h"

#include <boost/bind.hpp>

#include <signal.h>

// Interface of Waiter, also takes care of mutex_ & cond_ init and destroy.
class Waiter : boost::noncopyable
{
 public:
  virtual void wait() = 0;
  virtual void signal() = 0;

 protected:
  Waiter()
  {
    pthread_mutex_init(&mutex_, NULL);
    pthread_cond_init(&cond_, NULL);
  }

  ~Waiter()
  {
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&cond_);
  }

  pthread_mutex_t mutex_;
  pthread_cond_t cond_;
};

// Version 1: orininal from the book
// Incorrect, could lose signal
class Waiter1 : public Waiter
{
 public:
  void wait() override
  {
    pthread_mutex_lock(&mutex_);
    pthread_cond_wait(&cond_, &mutex_);
    pthread_mutex_unlock(&mutex_);
  }

  void signal() override
  {
    pthread_cond_signal(&cond_);
  }
};

// Version 2: signal in lock
// Incorrect, could lose signal
class Waiter2 : public Waiter
{
 public:
  void wait() override
  {
    pthread_mutex_lock(&mutex_);
    pthread_cond_wait(&cond_, &mutex_);
    pthread_mutex_unlock(&mutex_);
  }

  void signal() override
  {
    pthread_mutex_lock(&mutex_);
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&mutex_);
  }
};

// Version 3: add a boolean member
// Incorrect, spurious wakeup
class Waiter3 : public Waiter
{
 public:
  void wait() override
  {
    pthread_mutex_lock(&mutex_);
    if (!signaled_)
    {
      pthread_cond_wait(&cond_, &mutex_);
    }
    pthread_mutex_unlock(&mutex_);
  }

  void signal() override
  {
    pthread_mutex_lock(&mutex_);
    signaled_ = true;
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&mutex_);
  }

 private:
  bool signaled_ = false;
};

// Version 4: wait in while-loop
// Correct, signal before unlock
class Waiter4 : public Waiter
{
 public:
  void wait() override
  {
    pthread_mutex_lock(&mutex_);
    while (!signaled_)
    {
      pthread_cond_wait(&cond_, &mutex_);
    }
    pthread_mutex_unlock(&mutex_);
  }

  void signal() override
  {
    pthread_mutex_lock(&mutex_);
    signaled_ = true;
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&mutex_);
  }

 private:
  bool signaled_ = false;
};

// Version 5: wait in while-loop
// Correct, signal after unlock
class Waiter5 : public Waiter
{
 public:
  void wait() override
  {
    pthread_mutex_lock(&mutex_);
    while (!signaled_)
    {
      pthread_cond_wait(&cond_, &mutex_);
    }
    pthread_mutex_unlock(&mutex_);
  }

  void signal() override
  {
    pthread_mutex_lock(&mutex_);
    signaled_ = true;
    pthread_mutex_unlock(&mutex_);
    pthread_cond_signal(&cond_);
  }

 private:
  bool signaled_ = false;
};

// Note: version 4 is as efficient as version 5 because of "wait morphing"

// Version 6: signal before set boolean flag
// Correct or not?
class Waiter6 : public Waiter
{
 public:
  void wait() override
  {
    pthread_mutex_lock(&mutex_);
    while (!signaled_)
    {
      pthread_cond_wait(&cond_, &mutex_);
    }
    pthread_mutex_unlock(&mutex_);
  }

  void signal() override
  {
    pthread_mutex_lock(&mutex_);
    pthread_cond_signal(&cond_);
    signaled_ = true;
    pthread_mutex_unlock(&mutex_);
  }

 private:
  bool signaled_ = false;
};

// Version 8: modify signaled_ without lock
// Incorrect, data-race and could lose signal
class Waiter8 : public Waiter
{
 public:
  void wait() override
  {
    pthread_mutex_lock(&mutex_);
    while (!signaled_)
    {
      pthread_cond_wait(&cond_, &mutex_);
    }
    pthread_mutex_unlock(&mutex_);
  }

  void signal() override
  {
    signaled_ = true;
    pthread_cond_signal(&cond_);
  }

 private:
  bool signaled_ = false;
};

// Version 7: broadcast to wakeup multiple waiting threads
// Probably the best version among above.
class Waiter7 : public Waiter
{
 public:
  void wait() override
  {
    pthread_mutex_lock(&mutex_);
    while (!signaled_)
    {
      pthread_cond_wait(&cond_, &mutex_);
    }
    pthread_mutex_unlock(&mutex_);
  }

  void signal() override // Sorry, bad name in base class, poor OOP
  {
    broadcast();
  }

  void broadcast()
  {
    pthread_mutex_lock(&mutex_);
    pthread_cond_broadcast(&cond_);
    signaled_ = true;
    pthread_mutex_unlock(&mutex_);
  }

 private:
  bool signaled_ = false;
};

// Same as version 7, with muduo library
class WaiterInMuduo : boost::noncopyable
{
 public:
  WaiterInMuduo()
    : cond_(mutex_)
  {
  }

  void wait()
  {
    muduo::MutexLockGuard lock(mutex_);
    while (!signaled_)
    {
      cond_.wait();
    }
  }

  void broadcast()
  {
    muduo::MutexLockGuard lock(mutex_);
    signaled_ = true;
    cond_.notifyAll();
  }

 private:
  muduo::MutexLock mutex_;
  muduo::Condition cond_;
  bool signaled_ = false;
};

void initAndRefresh(Waiter* waiter)
{
  printf("init: running\n");

  struct timespec ts = { 0, 500*1000*1000 };
  nanosleep(&ts, NULL);  // initialization takes 500ms

  waiter->signal();
  printf("init: signaled\n");
}

void runServer(Waiter* waiter, int sec)
{
  muduo::Thread initThread(boost::bind(initAndRefresh, waiter));
  initThread.start();
  printf("main: init thread started\n");

  struct timespec ts = { sec, 0 };
  nanosleep(&ts, NULL); // do some work before calling wait()

  printf("main: waiting\n");
  waiter->wait();
  printf("main: done\n");
}

void sigalarm(int)
{
  write(1, "\nFAILED\n", 8);
  exit(1);
}

int main(int argc, char* argv[])
{
  signal(SIGALRM, sigalarm);
  alarm(5);

  Waiter1 w1;
  Waiter2 w2;
  Waiter3 w3;
  Waiter4 w4;
  Waiter5 w5;
  Waiter6 w6;
  Waiter7 w7;
  Waiter8 w8;
  WaiterInMuduo waiterInMuduo;

  Waiter* waiters[] = { NULL, &w1, &w2, &w3, &w4, &w5, &w6, &w7, &w8 };
  int whichWaiter = argc > 1 ? atoi(argv[1]) : 5;
  if (!(whichWaiter > 0 && whichWaiter <= 8))
  {
    printf("Unknown waiter, must between 1 and 8, inclusive.\n");
    return 1;
  }
  Waiter* waiter = waiters[whichWaiter];

  int sec = 0;
  printf("test 1: wait() before signal().\n");
  runServer(waiter, sec);

  printf("\ntest 2: wait() after signal().\n");
  sec = 1;
  runServer(waiter, sec);

  printf("\nPASSED!\n");
}
