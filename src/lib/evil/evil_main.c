#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <windows.h>

#include "evil_private.h"

static UINT     _evil_time_period = 1;

static int      _evil_init_count = 0;

extern LONGLONG _evil_time_freq;
extern LONGLONG _evil_time_count;

extern DWORD    _evil_tls_index;

int
evil_init(void)
{
   LARGE_INTEGER freq;
   LARGE_INTEGER count;
   TIMECAPS tc;
   MMRESULT res;

   if (++_evil_init_count != 1)
     return _evil_init_count;

   res = timeGetDevCaps(&tc, sizeof(TIMECAPS));
   if (res  != MMSYSERR_NOERROR)
     return --_evil_init_count;

   _evil_time_period = tc.wPeriodMin;
   res = timeBeginPeriod(_evil_time_period);
   if (res  != TIMERR_NOERROR)
     return --_evil_init_count;

   QueryPerformanceFrequency(&freq);

   _evil_time_freq = freq.QuadPart;

   QueryPerformanceCounter(&count);

   _evil_time_count = count.QuadPart;

   if (!evil_sockets_init())
     return --_evil_init_count;

   return _evil_init_count;
}

int
evil_shutdown(void)
{
   /* _evil_init_count should not go below zero. */
   if (_evil_init_count < 1)
     {
        printf("Evil shutdown called without calling evil init.\n");
        return 0;
     }

   if (--_evil_init_count != 0)
     return _evil_init_count;

   evil_sockets_shutdown();

   timeEndPeriod(_evil_time_period);

   return _evil_init_count;
}

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved)
{
   LPVOID data;

   switch (reason)
     {
     case DLL_PROCESS_ATTACH:
       _evil_tls_index = TlsAlloc();
       if (_evil_tls_index == TLS_OUT_OF_INDEXES)
         return FALSE;
       /* No break: Initialize the index for first thread. */
       /* fall through */
     case DLL_THREAD_ATTACH:
       data = (LPVOID)LocalAlloc(LPTR, 4096);
       if (!data)
         return FALSE;
       if (!TlsSetValue(_evil_tls_index, data))
         return FALSE;
       break;
     case DLL_THREAD_DETACH:
       data = TlsGetValue(_evil_tls_index);
       if (data)
         LocalFree((HLOCAL)data);
       break;
     case DLL_PROCESS_DETACH:
       data = TlsGetValue(_evil_tls_index);
       if (data)
         LocalFree((HLOCAL)data);
       TlsFree(_evil_tls_index);
       break;
     default:
       break;
     }

   return TRUE;

   (void)inst;
   (void)reserved;
}
