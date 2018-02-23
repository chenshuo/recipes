#include "DigestOOP.h"

#include <openssl/crypto.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

namespace oop
{

#define DEFINE(name)                                       \
  class name##Digest : public Digest                       \
  {                                                        \
   public:                                                 \
    name##Digest()                                         \
    {                                                      \
      name##_Init(&ctx_);                                  \
    }                                                      \
                                                           \
    ~name##Digest() override                               \
    {                                                      \
      OPENSSL_cleanse(&ctx_, sizeof(ctx_));                \
    }                                                      \
                                                           \
    void update(const void* data, int len) override        \
    {                                                      \
      name##_Update(&ctx_, data, len);                     \
    }                                                      \
                                                           \
    std::string digest() override                          \
    {                                                      \
      unsigned char result[name##_DIGEST_LENGTH];          \
      name##_Final(result, &ctx_);                         \
      return std::string(reinterpret_cast<char*>(result),  \
                         name##_DIGEST_LENGTH);            \
    }                                                      \
                                                           \
    int length() const override                            \
    {                                                      \
      return name##_DIGEST_LENGTH;                         \
    }                                                      \
                                                           \
   private:                                                \
    name##_CTX ctx_;                                       \
  };
#define SHA1_DIGEST_LENGTH SHA_DIGEST_LENGTH
#define SHA1_CTX SHA_CTX

DEFINE(MD5);
DEFINE(SHA1);
DEFINE(SHA256);

// static
std::unique_ptr<Digest> Digest::create(Type t)
{
  if (t == MD5)
    return std::make_unique<MD5Digest>();
  else if (t == SHA1)
    return std::make_unique<SHA1Digest>();
  else if (t == SHA256)
    return std::make_unique<SHA256Digest>();
  else
    return nullptr;
}

}
