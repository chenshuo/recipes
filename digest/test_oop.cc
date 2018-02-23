#include "DigestOOP.h"

void print(const std::string& d)
{
  for (int i = 0; i < d.size(); ++i)
  {
    printf("%02x", (unsigned char)d[i]);
  }
  printf("\n");
}

int main(int argc, char* argv[])
{
  auto md5 = oop::Digest::create(oop::Digest::MD5);
  print(md5->digest());
  auto sha1 = oop::Digest::create(oop::Digest::SHA1);
  print(sha1->digest());
  auto sha256 = oop::Digest::create(oop::Digest::SHA256);
  print(sha256->digest());
}
