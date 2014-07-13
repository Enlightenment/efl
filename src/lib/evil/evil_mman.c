#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#ifndef _MSC_VER
# include <unistd.h>
#endif

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

# include <io.h>

#include "evil_macro.h"
#include "sys/mman.h"
#include "evil_util.h"
#include "evil_private.h"


/***** API *****/


void *
mmap(void  *addr EVIL_UNUSED,
     size_t len,
     int    prot,
     int    flags,
     int    fd,
     off_t  offset)
{
   HANDLE fm;
   DWORD  protect = PAGE_NOACCESS;
   DWORD  acs = 0;
   HANDLE handle;
   void  *data;

   /* support only MAP_SHARED */
   if (!(flags & MAP_SHARED))
     return MAP_FAILED;

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

   handle = (HANDLE)_get_osfhandle(fd);
   if (handle == INVALID_HANDLE_VALUE)
     {
        fprintf(stderr, "[Evil] [mmap] _get_osfhandle failed\n");

        return MAP_FAILED;
     }

   fm = CreateFileMapping(handle, NULL, protect, 0, 0, NULL);
   if (!fm)
     {
        char *str;

        str = evil_last_error_get();
        fprintf(stderr, "[Evil] [mmap] CreateFileMapping failed: %s\n", str);
        free(str);

        return MAP_FAILED;
     }

   if ((protect & PAGE_READWRITE) == PAGE_READWRITE)
     acs = FILE_MAP_ALL_ACCESS;
   else if ((protect & PAGE_WRITECOPY) == PAGE_WRITECOPY)
     acs = FILE_MAP_COPY;
#if 0
   if (protect & (PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_READ))
     acs = FILE_MAP_EXECUTE;
#endif
   else if ((protect & (PAGE_READWRITE | PAGE_READONLY)) == (PAGE_READWRITE | PAGE_READONLY))
     acs = FILE_MAP_READ;
   else
     {
        if ((protect & PAGE_WRITECOPY) == PAGE_WRITECOPY)
          acs = FILE_MAP_WRITE;
     }

   data = MapViewOfFile(fm,
                        acs,
                        offset & 0xffff0000,
                        offset & 0x0000ffff,
                        len);
   CloseHandle(fm);

   if (!data)
     {
        char *str;

        str = evil_last_error_get();
        fprintf(stderr, "[Evil] [mmap] MapViewOfFile failed: %s\n", str);
        free(str);

        return MAP_FAILED;
     }

   return data;
}

int
munmap(void  *addr,
       size_t len EVIL_UNUSED)
{
   BOOL res;

   res = UnmapViewOfFile(addr);
   if (!res)
     {
        char *str;

        str = evil_last_error_get();
        fprintf(stderr, "[Evil] [munmap] UnmapViewOfFile failed: %s\n", str);
        free(str);
     }

   return (res == 0) ? -1 : 0;
}
