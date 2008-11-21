
#if defined (_WIN32_WCE) && ! defined (__CEGCC__)

# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# undef WIN32_LEAN_AND_MEAN

#include "Evil.h"
#include "evil_private.h"

extern long _evil_time_second;
extern long _evil_time_millisecond;

#endif /* _WIN32_WCE && ! __CEGCC__ */

static int  _evil_init_count = 0;

int
evil_init()
{
#if defined (_WIN32_WCE) && ! defined (__CEGCC__)
   SYSTEMTIME st;
   DWORD      tick;
   WORD       second = 59;
#endif /* _WIN32_WCE && ! __CEGCC__ */

   if (_evil_init_count > 0)
     {
	_evil_init_count++;
	return _evil_init_count;
     }

#if defined (_WIN32_WCE) && ! defined (__CEGCC__)

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
        tick = GetTickCount();
        if (st.wSecond == second + 1)
          break;
     }

   _evil_time_second = _evil_systemtime_to_time(st);
   if (_evil_time_second < 0)
     return 0;
   _evil_time_millisecond = tick;

#endif /* _WIN32_WCE && ! __CEGCC__ */

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
