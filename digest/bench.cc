#include <benchmark/benchmark.h>
#include "DigestEVP.h"
#include "DigestOOP.h"

static void BM_MD5_OOP(benchmark::State& state)
{
  std::unique_ptr<oop::Digest> digest = oop::Digest::create(oop::Digest::MD5);
  std::string str(state.range(0), 'x');
  for (auto _ : state)
  {
    digest->update(str.data(), str.size());
  }
  digest->digest();
  state.SetBytesProcessed(str.size() * state.iterations());
}
BENCHMARK(BM_MD5_OOP)->Range(1, 16<<10);

static void BM_MD5_EVP(benchmark::State& state)
{
  evp::Digest digest(evp::Digest::MD5);
  std::string str(state.range(0), 'x');
  for (auto _ : state)
  {
    digest.update(str.data(), str.size());
  }
  digest->digest();
  state.SetBytesProcessed(str.size() * state.iterations());
}
BENCHMARK(BM_MD5_EVP)->Range(1, 16<<10);

static void BM_SHA1_OOP(benchmark::State& state)
{
  std::unique_ptr<oop::Digest> digest = oop::Digest::create(oop::Digest::SHA1);
  std::string str(state.range(0), 'x');
  for (auto _ : state)
  {
    digest->update(str.data(), str.size());
  }
  digest->digest();
  state.SetBytesProcessed(str.size() * state.iterations());
}
BENCHMARK(BM_SHA1_OOP)->Range(1, 16<<10);

static void BM_SHA1_EVP(benchmark::State& state)
{
  evp::Digest digest(evp::Digest::SHA1);
  std::string str(state.range(0), 'x');
  for (auto _ : state)
  {
    digest.update(str.data(), str.size());
  }
  digest.digest();
  state.SetBytesProcessed(str.size() * state.iterations());
}
BENCHMARK(BM_SHA1_EVP)->Range(1, 16<<10);

static void BM_SHA256_OOP(benchmark::State& state)
{
  std::unique_ptr<oop::Digest> digest = oop::Digest::create(oop::Digest::SHA256);
  std::string str(state.range(0), 'x');
  for (auto _ : state)
  {
    digest->update(str.data(), str.size());
  }
  digest.digest();
  state.SetBytesProcessed(str.size() * state.iterations());
}
BENCHMARK(BM_SHA256_OOP)->Range(1, 16<<10);

static void BM_SHA256_EVP(benchmark::State& state)
{
  evp::Digest digest(evp::Digest::SHA256);
  std::string str(state.range(0), 'x');
  for (auto _ : state)
  {
    digest.update(str.data(), str.size());
  }
  digest.digest();
  state.SetBytesProcessed(str.size() * state.iterations());
}
BENCHMARK(BM_SHA256_EVP)->Range(1, 16<<10);

BENCHMARK_MAIN();
