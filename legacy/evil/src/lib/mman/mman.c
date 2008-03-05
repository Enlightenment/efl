#include <sys/types.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#ifndef __CEGCC__
# include <io.h>
#endif /* ! __CEGCC__ */

#include <sys/mman.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE___ATTRIBUTE__
# define __UNUSED__ __attribute__((unused))
#else
# define __UNUSED__
#endif /* HAVE___ATTRIBUTE__ */

#if defined(__CEGCC__)
# define CreateFileMapping CreateFileMappingW
# define _get_osfhandle get_osfhandle
elif defined (__MINGW32CE__)
# define _get_osfhandle(FILEDES) ((long)FILEDES)
#endif /* ! __CEGCC__ && ! __MINGW32CE__ */


void *
mmap(void  *addr __UNUSED__,
     size_t len,
     int    prot,
     int    flags,
     int    fd,
     off_t  offset)
{
   void  *data;
   HANDLE fm;
   DWORD  protect = PAGE_NOACCESS;
   DWORD  access = 0;

   /* support only MAP_SHARED */
   if (!(flags & MAP_SHARED)) return (void *)~0;

   if (prot & PROT_EXEC)
     {
        if (prot & PROT_READ)
          {
             if (prot & PROT_WRITE)
               protect = PAGE_EXECUTE_READWRITE;
             else
               protect = PAGE_EXECUTE_READ;
          }
        else
          {
             if (prot & PROT_WRITE)
               protect = PAGE_EXECUTE_WRITECOPY;
             else
               protect = PAGE_EXECUTE;
          }
     }
   else
     {
        if (prot & PROT_READ)
          {
             if (prot & PROT_WRITE)
               protect = PAGE_READWRITE;
             else
               protect = PAGE_READONLY;
          }
        else if (prot & PROT_WRITE)
          protect = PAGE_WRITECOPY;
     }

   fm = CreateFileMapping((HANDLE)_get_osfhandle (fd),
                          NULL,
                          protect,
                          0,
                          0,
                          NULL);
   if (!fm) return (void *)~0;

   if (protect & PAGE_READWRITE)
     access = FILE_MAP_ALL_ACCESS;
   if (protect & PAGE_WRITECOPY)
     access = FILE_MAP_COPY;
#if 0
   if (protect & (PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_READ))
     access = FILE_MAP_EXECUTE;
#endif
   if (protect & (PAGE_READWRITE | PAGE_READONLY))
     access = FILE_MAP_READ;
   else
     {
        if (protect & PAGE_READWRITE)
          access = FILE_MAP_WRITE;
     }

   data = MapViewOfFile(fm,
                        access,
                        offset & 0xffff0000,
                        offset & 0x0000ffff,
                        len);
   CloseHandle(fm);

   if (!data) return (void *)~0;
   else return data;
}

int
munmap(void  *addr,
       size_t len __UNUSED__)
{
   BOOL res;

   res = UnmapViewOfFile(addr);

   return (res == 0) ? -1 : 0;
}
