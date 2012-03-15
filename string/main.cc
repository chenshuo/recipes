#include "StringEager.h"
#include "StringSso.h"
#include <stdio.h>

char muduo::StringEager::kEmpty_[1] = { '\0' };

int main()
{
  printf("%zd\n", sizeof(muduo::StringEager));
  printf("%zd\n", sizeof(muduo::StringSso));
  muduo::StringEager x;
}
