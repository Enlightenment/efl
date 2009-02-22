#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "Evil.h"
#include "evil_private.h"

# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# undef WIN32_LEAN_AND_MEAN


static int           _evil_init_count = 0;

extern LONGLONG _evil_time_freq;
extern LONGLONG _evil_time_count;
extern long     _evil_time_second;

int
evil_init()
{
   SYSTEMTIME    st;
   LARGE_INTEGER freq;
   LARGE_INTEGER count;
   WORD          second = 59;

   if (_evil_init_count > 0)
     {
	_evil_init_count++;
	return _evil_init_count;
     }

   if (!QueryPerformanceFrequency(&freq))
     {
       return 0;
     }
   _evil_time_freq = freq.QuadPart;

   /* be sure that second + 1 != 0 */
   while (second == 59)
     {
        GetSystemTime(&st);
        second = st.wSecond;
     }

   /* retrieve the tick corresponding to the time we retrieve above */
   while (1)
     {
        GetSystemTime(&st);
        QueryPerformanceCounter(&count);
        if (st.wSecond == second + 1)
          break;
     }

   _evil_time_second = _evil_systemtime_to_time(st);
   if (_evil_time_second < 0)
     return 0;
   _evil_time_count = count.QuadPart;

   _evil_init_count++;

   return _evil_init_count;
}

int
evil_shutdown()
{
   _evil_init_count--;
   if (_evil_init_count > 0) return _evil_init_count;

   if (_evil_init_count < 0) _evil_init_count = 0;

   return _evil_init_count;
}
