#include <string.h>
#include <memory>
#include <string>
#include <unordered_set>

#include "benchmark/benchmark.h"
#include <brotli/encode.h>
#include "huf.h"
#include "lz4.h"
#include <snappy.h>
#include "zlib.h"
#include "zstd.h"

enum Compress
{
  Brotli,
  Huff0,  // only for input size <= 128KiB
  LZ4,
  None,   // memcpy
  Snappy,
  Zlib,
  Zstd,   // Zstd is much faster on Haswell with BMI2 instructions.
};


template<Compress c>
static void compress(const std::string& input, benchmark::State& state)
{
  size_t output_len = 0;
  int level = state.range(0);
  if (c == Brotli)
  {
    output_len = BrotliEncoderMaxCompressedSize(input.size());
  }
  else if (c == Huff0)
  {
    output_len = HUF_compressBound(std::min<size_t>(input.size(), HUF_BLOCKSIZE_MAX));
  }
  else if (c == None)
  {
    output_len = input.size();
  }
  else if (c == Snappy)
  {
    output_len = snappy::MaxCompressedLength(input.size());
  }
  else if (c == LZ4)
  {
    output_len = LZ4_compressBound(input.size());
  }
  else if (c == Zlib)
  {
    output_len = compressBound(input.size());
  }
  else if (c == Zstd)
  {
    output_len = ZSTD_compressBound(input.size());
  }
  // printf("%zd %zd %.5f\n", input.size(), output_len, double(output_len) / input.size());
  std::unique_ptr<char[]> output(new char[output_len]);

  size_t compressed_len = 0;
  for (auto _ : state)
  {
    if (c == Brotli)
    {
      compressed_len = output_len;
      BrotliEncoderCompress(level, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE,
                            input.size(), reinterpret_cast<const unsigned char*>(input.c_str()),
                            &compressed_len, reinterpret_cast<unsigned char*>(output.get()));
    }
    else if (c == Huff0)
    {
      std::string_view in(input);
      compressed_len = 0;
      while (!in.empty())
      {
        size_t src = std::min<size_t>(in.size(), HUF_BLOCKSIZE_MAX);
        output_len = HUF_compressBound(src);
        //printf("%zd %zd\n", src, output_len);
        size_t compressed = HUF_compress(output.get(), output_len, in.data(), src);
        if (compressed == 0)
        {
          // not compressible
          compressed_len += output_len;
        }
        else if (HUF_isError(compressed))
        {
          printf("%s\n", HUF_getErrorName(compressed));
          state.SetLabel(HUF_getErrorName(compressed));
          return;
          break;
        }
        else
        {
          compressed_len += compressed;
        }
        in.remove_prefix(src);
      }
    }
    else if (c == LZ4)
    {
      compressed_len = LZ4_compress_default(input.c_str(), output.get(), input.size(), output_len);
    }
    else if (c == None)
    {
      memcpy(output.get(), input.c_str(), input.size());
      compressed_len = output_len;
    }
    else if (c == Snappy)
    {
      snappy::RawCompress(input.c_str(), input.size(), output.get(), &compressed_len);
    }
    else if (c == Zlib)
    {
      compressed_len = output_len;
      compress2(reinterpret_cast<unsigned char*>(output.get()), &compressed_len,
                reinterpret_cast<const unsigned char*>(input.c_str()), input.size(),
                level);
    }
    else if (c == Zstd)
    {
      compressed_len = ZSTD_compress(output.get(), output_len, input.c_str(), input.size(),
                                     level);
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
BENCHMARK_TEMPLATE(BM_highest, None)->Arg(0)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_highest, Snappy)->Arg(0)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_highest, LZ4)->Arg(0)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_highest, Zlib)->Arg(1)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_highest, Brotli)->DenseRange(0, 7)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_highest, Zstd)->DenseRange(1, 9)->Unit(benchmark::kMillisecond);


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

std::string getIncompressibleZstd()
{
  printf("Generating incompressible data for Zstd...");
  fflush(stdout);
  std::set<unsigned char> avails[256*256];
  std::set<unsigned char> all;
  for (int i = 0; i < 256; ++i)
    all.insert(i);
  for (auto& x : avails)
    x = all;
  std::string data;
  data.reserve(256*256*256);
  data.push_back('\0');
  data.push_back('\1');
  bool popped = false;

  while (true)
  {
    assert(data.size() >= 2);
    unsigned char prev = data[data.size()-2];
    unsigned char curr = data.back();

    unsigned key = prev * 256 + curr;
    assert(key < 256*256);
    auto& avail = avails[key];
    
    if (avail.empty())
    {
      if (popped)
        break;
      popped = true;
      data.pop_back();
      continue;
    }
    unsigned char next = data.back() + 1;
    auto it = avail.lower_bound(next);
    if (it != avail.end())
    {
      // next avail
      next = *it;
    }
    else
    {
      // wrap to front
      next = *avail.begin();
    }
    size_t c = avail.erase(next);
    assert(c == 1);
    data.push_back(next);
  }
  printf(" done %zd\n", data.size());
  return data;
}

std::string incompressible;

template<Compress c>
static void BM_lowest(benchmark::State& state)
{
  if (incompressible.empty())
    incompressible = getIncompressibleZstd();
  std::string input = incompressible;
  // printf("incompressible %zd\n", input.size());
  while (input.size() < 100*1024*1024)
    input += input;

  compress<c>(input, state);
}
BENCHMARK_TEMPLATE(BM_lowest, Snappy)->Arg(0)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_lowest, LZ4)->Arg(0)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_lowest, Zlib)->Arg(1)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_lowest, Brotli)->DenseRange(0, 7)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_lowest, Zstd)->DenseRange(1, 9)->Unit(benchmark::kMillisecond);

const char* g_file = "input";

template<Compress c>
static void BM_file(benchmark::State& state)
{
  std::string input;
  FILE* fp = fopen(g_file, "r");
  if (!fp)
  {
    state.SkipWithError(strerror(errno));
    return;
  }
  assert(fp);
  size_t nr = 0;
  char buf[64*1024];
  while ( (nr = fread(buf, 1, sizeof buf, fp)) > 0)
    input.append(buf, nr);
  fclose(fp);

  compress<c>(input, state);
}
BENCHMARK_TEMPLATE(BM_file, Huff0)->Arg(0)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_file, Snappy)->Arg(0)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_file, LZ4)->Arg(0)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_file, Zlib)->DenseRange(1, 7)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_file, Brotli)->DenseRange(BROTLI_MIN_QUALITY, 7)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_file, Zstd)->DenseRange(1, 12)->Unit(benchmark::kMillisecond);
