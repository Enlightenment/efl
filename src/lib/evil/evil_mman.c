#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <io.h>

#include "evil_private.h"

/*
 * Possible values
 * PAGE_EXECUTE_READ (equivalent to PAGE_EXECUTE_WRITECOPY)
 * PAGE_EXECUTE_READWRITE
 * PAGE_READONLY (equivalent to PAGE_WRITECOPY)
 * PAGE_READWRITE
 */
static DWORD
_evil_mmap_protection_get(int prot)
{
   if (prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC))
     return 0xffffffff;

   if (prot & PROT_WRITE)
     {
        if (prot & PROT_EXEC)
          return PAGE_EXECUTE_READWRITE;
        else
          return PAGE_READWRITE;
     }
   else
     {
        if (prot & PROT_EXEC)
          return PAGE_EXECUTE_READ;
        else
          return PAGE_READONLY;
     }
}


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
   DWORD  protect;
   DWORD  acs = 0;
   HANDLE handle;
   void  *data;
   DWORD low;
   DWORD high;

   /* get protection */
   protect = _evil_mmap_protection_get(prot);
   if (protect == 0xffffffff)
     return MAP_FAILED;

   /* check if the mapping is backed by a file or not */
   if (fd == -1)
     {
        /* shared memory */
        if (!(flags & MAP_ANON) || offset)
          return MAP_FAILED;
     }
   else
     {
        if (flags & MAP_ANON)
          return MAP_FAILED;
     }

   if (fd == -1)
     handle = INVALID_HANDLE_VALUE;
   else
     {
        handle = (HANDLE)_get_osfhandle(fd);
        if ((errno == EBADF) && (handle == INVALID_HANDLE_VALUE))
          {
             fprintf(stderr, "[Evil] [mmap] _get_osfhandle failed\n");
             return MAP_FAILED;
          }
     }

#ifdef _WIN64
   low = (DWORD)((len >> 32) & 0x00000000ffffffff);
   low = (DWORD)(len & 0x00000000ffffffff);
#else
   high = 0L;
   low = len;
#endif

   fm = CreateFileMapping(handle, NULL, protect, high, low, NULL);
   if (!fm)
     {
        fprintf(stderr, "[Evil] [mmap] CreateFileMapping failed: %s\n",
                evil_last_error_get());
        return MAP_FAILED;
     }

   if (prot & PROT_WRITE)
     acs = FILE_MAP_WRITE;
   else
     acs = FILE_MAP_READ;
   if (prot & PROT_EXEC)
     acs |= FILE_MAP_EXECUTE;
   if (prot & MAP_PRIVATE)
     acs |= FILE_MAP_COPY;

   data = MapViewOfFile(fm,
                        acs,
                        offset & 0xffff0000,
                        offset & 0x0000ffff,
                        len);
   CloseHandle(fm);

   if (!data)
     {
        fprintf(stderr, "[Evil] [mmap] MapViewOfFile failed: %s\n",
                evil_last_error_get());
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
     fprintf(stderr, "[Evil] [munmap] UnmapViewOfFile failed: %s\n",
             evil_last_error_get());

   return (res == 0) ? -1 : 0;
}

int
mprotect(void *addr, size_t len, int prot)
{
   DWORD old;
   return VirtualProtect(addr, len, _evil_mmap_protection_get(prot), &old) ? 0 : -1;
}
