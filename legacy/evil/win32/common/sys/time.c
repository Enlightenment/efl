#include "time.h"

#include <sys/timeb.h>

int __cdecl gettimeofday(struct timeval *tv, void *tzp)
{
   struct _timeb timebuffer;

   _ftime(&timebuffer);
   tv->tv_sec = timebuffer.time;
   tv->tv_usec = timebuffer.millitm * 1000;
   return 0;
}

