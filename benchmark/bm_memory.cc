#include <string.h>
#include "benchmark/benchmark.h"

/*
// memory read bandwidth
static void BM_memchr(benchmark::State& state)
{
  int64_t len = state.range(0);
  void* buf = ::malloc(len);
  ::memset(buf, 0, len);
  benchmark::DoNotOptimize(buf);
  for (auto _ : state)
    ::memchr(buf, 1, len);
  state.SetBytesProcessed(len * state.iterations());
  ::free(buf);
}
BENCHMARK(BM_memchr)->Range(8, 1 << 30);
*/

// memory write bandwidth
static void BM_memset(benchmark::State& state)
{
  int64_t len = state.range(0);
  void* buf = ::malloc(len);
  for (auto _ : state)
  {
    ::memset(buf, 0, len);
    benchmark::ClobberMemory();
  }
  state.SetBytesProcessed(len * state.iterations());
  ::free(buf);
}
BENCHMARK(BM_memset)->Range(8, 1 << 30);
