#include "TimeZone.h"
#include <stdio.h>

using muduo::TimeZone;

int main(int argc, char* argv[])
{
  const char* zonefile = argc == 1 ? "/etc/localtime" : argv[1];
  TimeZone tz(zonefile);
}
