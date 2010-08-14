#include "Date.h"
#include <stdio.h>
#include <stdlib.h>

using muduo::Date;

int main(int argc, char* argv[])
{
  if (argc == 2)
  {
    int jdn = atoi(argv[1]);
    Date date(jdn);
    printf("%s\n", date.toIsoString().c_str());
  }
  else if (argc == 4)
  {
    int year = atoi(argv[1]);
    int month = atoi(argv[2]);
    int day = atoi(argv[3]);
    Date date(year, month, day);
    printf("%d\n", date.julianDayNumber());
  }
  else
  {
    printf("Usage: %s [julian_day_number | year month day]\n", argv[0]);
  }
}


