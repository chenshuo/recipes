/*
 $ sudo bash -c "echo 1 > /proc/sys/vm/drop_caches"
 */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fstream>
#include <string>

#include "benchmark/benchmark.h"

const char* g_filename;

static void BM_read(benchmark::State& state)
{
  const int64_t buflen = state.range(0);
  void* buffer = ::malloc(buflen);
  int64_t len = 0;
  for (auto _ : state)
  {
    int fd = ::open(g_filename, O_RDONLY);
    if (fd < 0)
    {
      state.SkipWithError("Failed to read data!");
      break;
    }

    ssize_t nr = 0;
    while ( (nr = ::read(fd, buffer, buflen)) > 0)
    {
      len += nr;
    }
    ::close(fd);
  }
  state.SetBytesProcessed(len);
  ::free(buffer);
}
BENCHMARK(BM_read)->RangeMultiplier(2)->Range(512, 8 * 1024 * 1024)->UseRealTime()->Unit(benchmark::kMillisecond);


struct GetLine
{
  bool operator()(FILE* fp) const
  {
    char line[1024];
    return ::fgets(line, sizeof line, fp);
  }
};

struct GetLineAssign
{
  bool operator()(FILE* fp) const
  {
    char line[1024] = "";  // same as bzero(line, 1024)
    return ::fgets(line, sizeof line, fp);
  }
};

template<typename FUNC>
static void BM_fgets(benchmark::State& state)
{
  int64_t len = 0;
  int64_t lines = 0;
  char buffer[128 * 1024];
  FUNC func;
  for (auto _ : state)
  {
    FILE* fp = ::fopen(g_filename, "r");
    if (!fp)
    {
      state.SkipWithError("Failed to read data!");
      break;
    }
    ::setbuffer(fp, buffer, sizeof buffer);
    while (func(fp))
    {
      lines++;
    }
    len += ::ftell(fp);
    ::fclose(fp);
  }
  state.SetBytesProcessed(len);
  state.SetItemsProcessed(lines);
}
BENCHMARK_TEMPLATE(BM_fgets, GetLine)->UseRealTime()->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_fgets, GetLineAssign)->UseRealTime()->Unit(benchmark::kMillisecond);

static void BM_getline(benchmark::State& state)
{
  int64_t len = 0;
  int64_t lines = 0;
  for (auto _ : state)
  {
    std::ifstream in(g_filename);
    if (!in)
    {
      state.SkipWithError("Failed to read data!");
      break;
    }
    std::string line;
    while (getline(in, line))
    {
      len += line.size()+1;
      lines++;
    }
  }
  state.SetBytesProcessed(len);
  state.SetItemsProcessed(lines);
}
BENCHMARK(BM_getline)->UseRealTime()->Unit(benchmark::kMillisecond);

int main(int argc, char* argv[])
{
  benchmark::Initialize(&argc, argv);
  if (argc > 1)
  {
    g_filename = argv[1];
    benchmark::RunSpecifiedBenchmarks();
  }
  else
  {
    printf("Usage: %s file_to_read\n", argv[0]);
  }
}

