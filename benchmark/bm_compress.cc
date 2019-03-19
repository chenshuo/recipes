#include <string.h>
#include <memory>
#include <unordered_set>

#include "benchmark/benchmark.h"
#include "zlib.h"
#include "zstd.h"

enum Compress
{
  Zlib,
  Zstd,
};

// Zstd is much faster on Haswell with BMI2 instructions.

template<Compress c>
static void compress(const std::string& input, benchmark::State& state)
{
  size_t output_len = 0;
  if (c == Zlib)
  {
    output_len = compressBound(input.size());
  }
  else if (c == Zstd)
  {
    output_len = ZSTD_compressBound(input.size());
  }
  // printf("%zd %zd %.5f\n", input.size(), output_len, double(output_len) / input.size());
  std::unique_ptr<char[]> output(new char[output_len]);

  size_t compressed_len = output_len;
  for (auto _ : state)
  {
    if (c == Zlib)
    {
      compressed_len = output_len;
      compress2(reinterpret_cast<unsigned char*>(output.get()), &compressed_len,
                reinterpret_cast<const unsigned char*>(input.c_str()), input.size(),
                Z_DEFAULT_COMPRESSION);
    }
    else if (c == Zstd)
    {
      compressed_len = ZSTD_compress(output.get(), output_len, input.c_str(), input.size(),
                                     ZSTD_CLEVEL_DEFAULT);
      if (ZSTD_isError(compressed_len))
      {
        state.SkipWithError(ZSTD_getErrorName(compressed_len));
      }
    }
  }
  state.SetItemsProcessed(state.iterations());
  state.SetBytesProcessed(input.size() * state.iterations());
  char buf[256];
  // printf("%zd %zd\n", input.size(), output_len);

  if (ZSTD_isError(compressed_len))
  {
    snprintf(buf, sizeof buf, "%s", ZSTD_getErrorName(compressed_len));
  }
  else
  {
    snprintf(buf, sizeof buf, "compress: %.8f", input.size() / double(compressed_len));
  }
  state.SetLabel(buf);
}

template<Compress c>
static void BM_highest(benchmark::State& state)
{
  std::string input;
  while (input.size() < 100*1024*1024)
    input += "aaaaaaaaaa";

  compress<c>(input, state);
}
BENCHMARK_TEMPLATE(BM_highest, Zlib)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_highest, Zstd)->Unit(benchmark::kMillisecond);


// zlib's max window size is 32768, and minimal match length is 3,
// so we generate non-repetitive two-byte tuples with total length > 64k.
// this algorithm is not efficient, we should probably use 'notseen' instead.
// TODO: generate incompressible data for zstd.
// zstd's minimal match length is 3, but window size can be as large as 8M,
// so this doesn't work.
std::string getIncompressible()
{
  std::unordered_set<unsigned char> seen[256];
  unsigned char current = 0;
  std::string result;
  while (true)
  {
    result.push_back(current);
    if (result.size() >= 256*255)
      break;

    if (seen[current].size() >= 256)
      break;
    unsigned char next = current + 1;
    while (seen[current].count(next) > 0)
    {
      next++;
    }
    seen[current].insert(next);
    current = next;
  }
  return result;
}

template<Compress c>
static void BM_lowest(benchmark::State& state)
{
  std::string input = getIncompressible();
  // printf("incompressible %zd\n", input.size());
  if (c == Zlib)
  {
    while (input.size() < 50*1024*1024)
      input += input;
  }
  // TODO: how to generate longer incompressible for Zstd?

  compress<c>(input, state);
}
BENCHMARK_TEMPLATE(BM_lowest, Zlib)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_lowest, Zstd)->Unit(benchmark::kMillisecond);

