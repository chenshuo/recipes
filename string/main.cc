#include "StringEager.h"
#include "StringSso.h"
#include <stdio.h>

int main()
{
  printf("%zd\n", sizeof(muduo::StringEager));
  printf("%zd\n", sizeof(muduo::StringSso));
  muduo::StringEager x;
}
