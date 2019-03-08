#include <assert.h>

#include <fstream>
#include <map>
#include <random>
#include <unordered_map>

#include "absl/container/flat_hash_map.h"
#include "absl/container/node_hash_map.h"
#include "benchmark/benchmark.h"

const int kMaxSize = 10'000'000;

std::vector<std::string> g_keys;  // of size kMaxSize

int64_t getTotalLength(const std::vector<std::string>& keys)
{
  return std::accumulate(
      keys.begin(), keys.end(), 0LL,
      [](int64_t len, const auto& key) {
        return len + key.size();
      });
}

template<class Map>
static void BM_wordcount(benchmark::State& state)
{
  assert(state.range(0) <= g_keys.size());
  std::vector<std::string> keys(g_keys.begin(), g_keys.begin() + state.range(0));
  int64_t total_length = getTotalLength(keys);
  for (auto _ : state)
  {
    Map counts;
    for (const auto& key : keys)
      counts[key]++;
  }
  state.SetItemsProcessed(keys.size() * state.iterations());
  state.SetBytesProcessed(total_length * state.iterations());
}
using treemap = std::map<std::string, int64_t>;
BENCHMARK_TEMPLATE(BM_wordcount, treemap)
  ->RangeMultiplier(10)->Range(10, kMaxSize)
  ->Unit(benchmark::kMillisecond);

using hashmap = std::unordered_map<std::string, int64_t>;
BENCHMARK_TEMPLATE(BM_wordcount, hashmap)
  ->RangeMultiplier(10)->Range(10, kMaxSize)
  ->Unit(benchmark::kMillisecond);

using nodemap = absl::node_hash_map<std::string, int64_t>;
BENCHMARK_TEMPLATE(BM_wordcount, nodemap)
  ->RangeMultiplier(10)->Range(10, kMaxSize)
  ->Unit(benchmark::kMillisecond);

using flatmap = absl::flat_hash_map<std::string, int64_t>;
BENCHMARK_TEMPLATE(BM_wordcount, flatmap)
  ->RangeMultiplier(10)->Range(10, kMaxSize)
  ->Unit(benchmark::kMillisecond);

template<class STR>
static void BM_wordsort(benchmark::State& state)
{
  assert(state.range(0) <= g_keys.size());
  std::vector<std::string> keys(g_keys.begin(), g_keys.begin() + state.range(0));
  int64_t total_length = getTotalLength(keys);
  for (auto _ : state)
  {
    state.PauseTiming();
    std::vector<STR> copy(keys.begin(), keys.end());
    state.ResumeTiming();
    std::sort(copy.begin(), copy.end());
  }
  state.SetItemsProcessed(keys.size() * state.iterations());
  state.SetBytesProcessed(total_length * state.iterations());
}
BENCHMARK_TEMPLATE(BM_wordsort, std::string)
  ->RangeMultiplier(10)->Range(10, kMaxSize)
  ->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_wordsort, std::string_view)
  ->RangeMultiplier(10)->Range(10, kMaxSize)
  ->Unit(benchmark::kMillisecond);

std::vector<std::string> getKeys(int n, bool truncate)
{
  std::vector<std::string> keys;
  std::mt19937_64 gen(43);
  for (int i = 0; i < n; ++i)
  {
    char buf[64];
    if (truncate)
    {
      snprintf(buf, sizeof buf, "%lu", gen() & 0xffffffff);
    }
    else
    {
      snprintf(buf, sizeof buf, "%lu", gen());
    }
    keys.push_back(buf);
  }
  return keys;
}

int main(int argc, char* argv[])
{
  benchmark::Initialize(&argc, argv);
  if (argc > 1 && ::strcmp(argv[1], "--short") == 0)
  {
    g_keys = getKeys(kMaxSize, /*truncate=*/true);
  }
  else if (argc > 1)
  {
    std::ifstream in(argv[1]);
    std::string line;
    while (getline(in, line))
    {
      g_keys.push_back(line);
      if (g_keys.size() >= kMaxSize)
        break;
    }
  }
  else
  {
    g_keys = getKeys(kMaxSize, /*truncate=*/false);
  }

  assert(g_keys.size() >= kMaxSize);

  int64_t len = 0;
  for (const auto& key : g_keys)
    len += key.size();
  printf("Average key length = %.2f\n", double(len) / g_keys.size());

  benchmark::RunSpecifiedBenchmarks();

  printf("Average key length = %.2f\n", double(len) / g_keys.size());
}

