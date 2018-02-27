#include "DigestEVP2.h"
#include <sys/time.h>

double now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void print(const std::string& d)
{
  for (int i = 0; i < d.size(); ++i)
  {
    printf("%02x", (unsigned char)d[i]);
  }
  printf("\n");
}

evp::Digest getDefaultDigest()
{
  return evp::Digest(evp::Digest::SHA1);
}

void testThroughput(evp::Digest::Type type, int nblocks, int block_size)
{
  std::string x(block_size, 'x');
  double start = now();
  evp::Digest d(type);
  for (int i = 0; i < nblocks; ++i)
    d.update(x.data(), x.size());
  d.digest();
  double seconds = now() - start;
  int64_t bytes = int64_t(nblocks) * block_size;
  printf("%-6s %7.2f MiB/s\n", d.name(), bytes / seconds / 1024 / 1024);
}

void testLatency(evp::Digest::Type type, int nblocks, int block_size)
{
  std::string x(block_size, 'x');
  double start = now();
  for (int i = 0; i < nblocks; ++i)
  {
    evp::Digest d(type);
    d.update(x.data(), x.size());
    d.digest();
  }
  double seconds = now() - start;
  int64_t bytes = int64_t(nblocks) * block_size;
  printf("%-6s %7.0f op/s %8.3f us/op %7.2f MiB/s\n", evp::Digest(type).name(),
      nblocks / seconds, seconds * 1e6 / nblocks, bytes / seconds / 1024 / 1024);
}

int main(int argc, char* argv[])
{
  // No OpenSSL_add_all_digests();
  if (argc == 4)
  {
    int nblocks = atoi(argv[2]);
    int block_size = atoi(argv[3]);
    if (argv[1][0] == 't')
    {
      testThroughput(evp::Digest::MD5, nblocks, block_size);
      testThroughput(evp::Digest::SHA1, nblocks, block_size);
      testThroughput(evp::Digest::SHA256, nblocks, block_size);
      testThroughput(evp::Digest::SHA512, nblocks, block_size);
    }
    else
    {
      testLatency(evp::Digest::MD5, nblocks, block_size);
      testLatency(evp::Digest::SHA1, nblocks, block_size);
      testLatency(evp::Digest::SHA256, nblocks, block_size);
      testLatency(evp::Digest::SHA512, nblocks, block_size);
    }
  }
  else
  {
  evp::Digest md5(evp::Digest::MD5);
  print(md5.digest());
  evp::Digest sha1(evp::Digest::SHA1);
  print(sha1.digest());
  evp::Digest sha256(evp::Digest::SHA256);
  print(sha256.digest());

  evp::Digest md = getDefaultDigest();
  md.update("hello\n", 6);
  print(md.digest());
  md = getDefaultDigest();
  md = std::move(md);
  }
}

