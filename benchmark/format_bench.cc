#include "format.h"
#include "benchmark/benchmark.h"
#include <random>

static void BM_formatSI(benchmark::State& state)
{
  std::mt19937_64 gen(43);
  for (auto _ : state)
  {
    formatSI(gen());
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_formatSI);

static void BM_formatIEC(benchmark::State& state)
{
  std::mt19937_64 gen(43);
  for (auto _ : state)
  {
    formatIEC(gen());
  }
  state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_formatIEC);
