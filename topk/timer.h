#pragma once

#include <sys/time.h>

#include "absl/strings/str_format.h"
#include "muduo/base/ProcessInfo.h"
#include "muduo/base/Timestamp.h"

class Timer
{
 public:
  Timer()
    : start_(now()),
      start_cpu_(muduo::ProcessInfo::cpuTime())
  {
  }

  std::string report(int64_t bytes) const
  {
    muduo::ProcessInfo::CpuTime end_cpu(muduo::ProcessInfo::cpuTime());
    double seconds = now() - start_;
    char buf[64];
    snprintf(buf, sizeof buf, "%'zd", bytes);
    return absl::StrFormat("%.2fs real  %.2fs cpu  %6.2f MiB/s  %s bytes",
                           seconds, end_cpu.total() - start_cpu_.total(),
                           bytes / seconds / 1024 / 1024, buf);
  }

  static double now()
  {
    struct timeval tv = { 0, 0 };
    gettimeofday(&tv, nullptr);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
  }

 private:
  const double start_;
  const muduo::ProcessInfo::CpuTime start_cpu_;
};
