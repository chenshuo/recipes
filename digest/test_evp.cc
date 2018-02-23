#include "DigestEVP.h"

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

int main(int argc, char* argv[])
{
  OpenSSL_add_all_digests();

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

  EVP_cleanup();
}

