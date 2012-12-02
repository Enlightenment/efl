#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "sys/mman.h"

/***** API *****/

void *
mmap(void  *addr __UNUSED__,
     size_t len,
     int    prot,
     int    flags,
     int    fd,
     off_t  offset)
{
   void *data;
   size_t size;

   data = malloc(len);
   if (!data)
     {
        fprintf (stderr, "[Escape] [mmap] malloc failed\n");
        return MAP_FAILED;
     }

   size = read(fd, data, len);
   if (size != len)
     {
        fprintf (stderr, "[Escape] [mmap] read failed\n");
        free(data);
        return MAP_FAILED;
     }

   if (lseek(fd, -len, SEEK_CUR) == -1)
     {
        fprintf (stderr, "[Escape] [mmap] lseek failed\n");
        free(data);
        return MAP_FAILED;
     }

   return data;
}

int
munmap(void  *addr,
       size_t len __UNUSED__)
{
   if (addr && (addr != MAP_FAILED))
     free(addr);

   return 0;
}

int
madvise(void  *addr __UNUSED__,
        size_t length __UNUSED__,
        int    advice __UNUSED__)
{
   return 0;
}

