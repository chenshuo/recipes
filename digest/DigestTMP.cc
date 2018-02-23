#include "DigestOOP.h"

#include <openssl/crypto.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

namespace oop
{

template<typename CTX,
    int Init(CTX*),
    int Update(CTX *c, const void *data, size_t len),
    int Final(unsigned char *md, CTX *c),
    int DIGEST_LENGTH>
class DigestT : public Digest
{
 public:
  DigestT()
  {
    Init(&ctx_);
  }

  ~DigestT() override
  {
    OPENSSL_cleanse(&ctx_, sizeof(ctx_));
  }

  void update(const void* data, int len) override
  {
    Update(&ctx_, data, len);
  }

  std::string digest() override
  {
    unsigned char result[DIGEST_LENGTH];
    Final(result, &ctx_);
    return std::string(reinterpret_cast<char*>(result), DIGEST_LENGTH);
  }

  int length() const override
  {
    return DIGEST_LENGTH;
  }

 private:
  CTX ctx_;
};

typedef DigestT<MD5_CTX, MD5_Init, MD5_Update, MD5_Final, MD5_DIGEST_LENGTH> MD5Digest;
typedef DigestT<SHA_CTX, SHA1_Init, SHA1_Update, SHA1_Final, SHA_DIGEST_LENGTH> SHA1Digest;
typedef DigestT<SHA256_CTX, SHA256_Init, SHA256_Update, SHA256_Final, SHA256_DIGEST_LENGTH> SHA256Digest;

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

}  // namespace oop
