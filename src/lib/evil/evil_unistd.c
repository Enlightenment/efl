#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <errno.h>
#include <direct.h>
# include <sys/time.h>

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#undef WIN32_LEAN_AND_MEAN

#include "evil_private.h"

#ifdef _MSC_VER
EVIL_API int
execvp(const char *file, char *const argv[])
{
   return _execvp(file, (const char *const *)argv);
}
#endif

LONGLONG _evil_time_freq;
LONGLONG _evil_time_count;

EVIL_API int
ftruncate(int fd, off_t size)
{
   HANDLE file = (HANDLE)_get_osfhandle(fd);

   if (SetFilePointer(file, (LONG)size, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
     {
       _set_errno(EINVAL);
       return -1;
     }

   if (!SetEndOfFile(file))
     {
       _set_errno(EIO);
       return -1;
     }

    return 0;
}

/*
 * Time related functions
 *
 */

EVIL_API double
evil_time_get(void)
{
   LARGE_INTEGER count;

   QueryPerformanceCounter(&count);

   return (double)(count.QuadPart - _evil_time_count)/ (double)_evil_time_freq;
}

