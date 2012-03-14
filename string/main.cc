#include "StringEager.h"
#include <stdio.h>

char muduo::StringEager::kEmpty_[1] = { '\0' };

int main()
{
  printf("%zd\n", sizeof(muduo::StringEager));
  muduo::StringEager x;
}
