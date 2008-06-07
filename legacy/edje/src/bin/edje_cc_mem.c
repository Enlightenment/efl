/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_cc.h"

void *
mem_alloc(size_t size)
{
   void *mem;

   mem = calloc(1, size);
   if (mem) return mem;
   fprintf(stderr, "%s: Error. %s:%i memory allocation of %i bytes failed. %s\n",
	   progname, file_in, line, size, strerror(errno));
   exit(-1);
   return NULL;
}

char *
mem_strdup(const char *s)
{
   void *str;

   str = strdup(s);
   if (str) return str;
   fprintf(stderr, "%s: Error. %s:%i memory allocation of %i bytes failed. %s. string being duplicated: \"%s\"\n",
	   progname, file_in, line, strlen(s) + 1, strerror(errno), s);
   exit(-1);
   return NULL;
}
