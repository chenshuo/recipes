#include "DigestOOP.h"

#include <openssl/crypto.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

namespace oop
{

class MD5Digest : public Digest
{
 public:
  MD5Digest()
  {
    MD5_Init(&ctx_);
  }

  ~MD5Digest() override
  {
    OPENSSL_cleanse(&ctx_, sizeof(ctx_));
  }

  void update(const void* data, int len) override
  {
    MD5_Update(&ctx_, data, len);
  }

  std::string digest() override
  {
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5_Final(result, &ctx_);
    return std::string(reinterpret_cast<char*>(result), MD5_DIGEST_LENGTH);
  }

  int length() const override
  {
    return MD5_DIGEST_LENGTH;
  }

 private:
  MD5_CTX ctx_;
};

class SHA1Digest : public Digest
{
 public:
  SHA1Digest()
  {
    SHA1_Init(&ctx_);
  }

  ~SHA1Digest() override
  {
    OPENSSL_cleanse(&ctx_, sizeof(ctx_));
  }

  void update(const void* data, int len) override
  {
    SHA1_Update(&ctx_, data, len);
  }

  std::string digest() override
  {
    unsigned char result[SHA_DIGEST_LENGTH];
    SHA1_Final(result, &ctx_);
    return std::string(reinterpret_cast<char*>(result), SHA_DIGEST_LENGTH);
  }

  int length() const override
  {
    return SHA_DIGEST_LENGTH;
  }

 private:
  SHA_CTX ctx_;
};

class SHA256Digest : public Digest
{
 public:
  SHA256Digest()
  {
    SHA256_Init(&ctx_);
  }

  ~SHA256Digest() override
  {
    OPENSSL_cleanse(&ctx_, sizeof(ctx_));
  }

  void update(const void* data, int len) override
  {
    SHA256_Update(&ctx_, data, len);
  }

  std::string digest() override
  {
    unsigned char result[SHA256_DIGEST_LENGTH];
    SHA256_Final(result, &ctx_);
    return std::string(reinterpret_cast<char*>(result), SHA256_DIGEST_LENGTH);
  }

  int length() const override
  {
    return SHA256_DIGEST_LENGTH;
  }

 private:
  SHA256_CTX ctx_;
};

// static
std::unique_ptr<Digest> Digest::create(Type t)
{
  std::unique_ptr<Digest> result;
  // TODO: std::make_unique
  if (t == MD5)
    result.reset(new MD5Digest);
  else if (t == SHA1)
    result.reset(new SHA1Digest);
  else if (t == SHA256)
    result.reset(new SHA256Digest);
  return result;
}

}  // namespace oop
