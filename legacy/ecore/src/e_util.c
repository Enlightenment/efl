#include "Ecore.h"

double
ecore_get_time(void)
{
  struct timeval      timev;

  gettimeofday(&timev, NULL);
  return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}
