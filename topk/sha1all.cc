#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <openssl/sha.h>

inline double now()
{
  struct timeval tv = { 0, 0 };
  gettimeofday(&tv, nullptr);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main(int argc, char* argv[])
{
  int64_t total = 0;
  double start = now();
  SHA_CTX ctx_;
  SHA1_Init(&ctx_);
  for (int i = 1; i < argc; ++i)
  {
    int fd = open(argv[i], O_RDONLY);
    struct stat st;
    fstat(fd, &st);
    size_t len = st.st_size;
    total += len;
    if (len < 1024*1024*1024)
    {
    void* mapped = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    assert(mapped != MAP_FAILED);
    SHA1_Update(&ctx_, mapped, len);
    munmap(mapped, len);
    }
    else
    {
      char buf[128*1024];
      ssize_t nr;
      while ( (nr = read(fd, buf, sizeof buf)) > 0)
      {
        SHA1_Update(&ctx_, buf, nr);
      }
    }
    ::close(fd);
  }
  unsigned char result[SHA_DIGEST_LENGTH];
  SHA1_Final(result, &ctx_);
  for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
  {
    printf("%02x", result[i]);
  }
  printf("\n");
  double sec = now() - start;
  printf("%ld bytes %.3f sec %.2f MiB/s\n", total, sec, total / sec / 1024 / 1024);
}

