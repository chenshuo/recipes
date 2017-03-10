#pragma once
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

inline double now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1e6;
}


struct Timer
{
  Timer()
      : start_(0), total_(0)
  {
  }

  void start()
  {
    start_ = gettime();
  }

  void stop()
  {
    total_ += gettime() - start_;
  }

  void reset()
  {
    start_ = 0;
    total_ = 0;
  }

  double seconds() const
  {
    return total_ / 1e9;
  }

  static int64_t gettime()
  {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1e9 + ts.tv_nsec;
  }

 private:
  int64_t start_, total_;
};
