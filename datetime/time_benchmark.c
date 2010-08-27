#include <stdio.h>
#include <time.h>
#include <sys/time.h>

double time_diff(struct timeval* high, struct timeval* low)
{
  return (high->tv_sec-low->tv_sec)+(high->tv_usec-low->tv_usec)/1000000.0;
}

int main(int argc, char* argv[])
{
  int i = 0;
  int n = atoi(argv[1]);
  int which = atoi(argv[2]);
  struct timeval tv;
  gettimeofday(&tv, NULL);
  struct timeval start;
  gettimeofday(&start, NULL);
  if (which == 0)
  {
    for (i = 0; i < n; ++i)
    {
      gettimeofday(&tv, NULL);
    }
  }
  else if (which == 1)
  {
    for (i = 0; i < n; ++i)
    {
      time(NULL);
    }
  }
  else if (which == 2)
  {
    for (i = 0; i < n; ++i)
    {
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
    }
  }
  else
  {
    for (i = 0; i < n; ++i)
    {
      getppid();
    }
  }


  struct timeval end;
  gettimeofday(&end, NULL);

  double d = time_diff(&end, &start);
  printf("%f seconds, %f micro seconds per call\n", d, d * 1000000 / n);
}

