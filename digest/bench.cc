#include <benchmark/benchmark.h>
#include "DigestEVP.h"
#include "DigestOOP.h"

template <oop::Digest::Type type>
static void BM_Digest_OOP(benchmark::State& state)
{
  std::unique_ptr<oop::Digest> digest = oop::Digest::create(type);
  std::string str(state.range(0), 'x');
  for (auto _ : state)
  {
    digest->update(str.data(), str.size());
  }
  digest->digest();
  state.SetBytesProcessed(str.size() * state.iterations());
}

template <oop::Digest::Type type>
static void BM_Digest_OOP_short(benchmark::State& state)
{
  std::string str(state.range(0), 'x');
  for (auto _ : state)
  {
    std::unique_ptr<oop::Digest> digest = oop::Digest::create(type);
    digest->update(str.data(), str.size());
    digest->digest();
  }
  state.SetBytesProcessed(str.size() * state.iterations());
}

namespace oop
{
const Digest::Type MD5 = Digest::MD5;
BENCHMARK_TEMPLATE(BM_Digest_OOP, MD5)->Range(1, 16<<10);
const Digest::Type SHA1 = Digest::SHA1;
BENCHMARK_TEMPLATE(BM_Digest_OOP, SHA1)->Range(1, 16<<10);
const Digest::Type SHA256 = Digest::SHA256;
BENCHMARK_TEMPLATE(BM_Digest_OOP, SHA256)->Range(1, 16<<10);

BENCHMARK_TEMPLATE(BM_Digest_OOP_short, MD5)->Range(1, 16<<10);
BENCHMARK_TEMPLATE(BM_Digest_OOP_short, SHA1)->Range(1, 16<<10);
BENCHMARK_TEMPLATE(BM_Digest_OOP_short, SHA256)->Range(1, 16<<10);
}

template <evp::Digest::Type type>
static void BM_Digest_EVP(benchmark::State& state)
{
  evp::Digest digest(type);
  std::string str(state.range(0), 'x');
  for (auto _ : state)
  {
    digest.update(str.data(), str.size());
  }
  digest.digest();
  state.SetBytesProcessed(str.size() * state.iterations());
}

template <evp::Digest::Type type>
static void BM_Digest_EVP_short(benchmark::State& state)
{
  std::string str(state.range(0), 'x');
  for (auto _ : state)
  {
    evp::Digest digest(type);
    digest.update(str.data(), str.size());
    digest.digest();
  }
  state.SetBytesProcessed(str.size() * state.iterations());
}

namespace evp
{
const Digest::Type MD5 = Digest::MD5;
BENCHMARK_TEMPLATE(BM_Digest_EVP, MD5)->Range(1, 16<<10);
const Digest::Type SHA1 = Digest::SHA1;
BENCHMARK_TEMPLATE(BM_Digest_EVP, SHA1)->Range(1, 16<<10);
const Digest::Type SHA256 = Digest::SHA256;
BENCHMARK_TEMPLATE(BM_Digest_EVP, SHA256)->Range(1, 16<<10);

BENCHMARK_TEMPLATE(BM_Digest_EVP_short, MD5)->Range(1, 16<<10);
BENCHMARK_TEMPLATE(BM_Digest_EVP_short, SHA1)->Range(1, 16<<10);
BENCHMARK_TEMPLATE(BM_Digest_EVP_short, SHA256)->Range(1, 16<<10);
}

BENCHMARK_MAIN();
