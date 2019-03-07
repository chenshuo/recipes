#include <string.h>
#include "benchmark/benchmark.h"

static void BM_memset(benchmark::State& state) {
  int64_t len = state.range(0);
  void* buf = ::malloc(len);
  for (auto _ : state)
    ::memset(buf, 0, len);
  state.SetBytesProcessed(len * state.iterations());
  ::free(buf);
}
BENCHMARK(BM_memset)->Range(8, 1 << 30);

BENCHMARK_MAIN();

