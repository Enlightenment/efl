#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include <errno.h>

#include "edje_cc.h"

void *
mem_alloc(size_t size)
{
   void *mem;

   mem = calloc(1, size);
   if (mem) return mem;
   ERR("%s:%i memory allocation of %zu bytes failed. %s",
       file_in, line, size, strerror(errno));
   exit(-1);
   return NULL;
}

char *
mem_strdup(const char *s)
{
   void *str;

   str = strdup(s);
   if (str) return str;
   ERR("%s:%i memory allocation of %zu bytes failed. %s. string being duplicated: \"%s\"",
       file_in, line, strlen(s) + 1, strerror(errno), s);
   exit(-1);
   return NULL;
}

