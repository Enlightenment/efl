/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* azundris */

#include <sys/types.h>
#include <stdlib.h>		/* malloc(), free() */
#include <stdio.h>
#include <string.h>		/* str...() */

#include <stdarg.h>		/* varargs in sprintf/appendf */

#include "ecore_private.h"

#include "Ecore_Config.h"
#include "ecore_config_util.h"

#include "ecore_config_private.h"

#define CHUNKLEN 4096

/*****************************************************************************/
/* STRINGS */
/***********/

estring            *
estring_new(int size)
{
   estring            *e = malloc(sizeof(estring));

   if (e)
     {
	memset(e, 0, sizeof(estring));
	if ((size > 0) && (e->str = malloc(size)))
	   e->alloc = size;
     }
   return e;
}

char               *
estring_disown(estring * e)
{
   if (e)
     {
	char               *r = e->str;

	free(e);
	return r;
     }
   return NULL;
}

int
estring_appendf(estring * e, const char *fmt, ...)
{
   int      need;
   va_list  ap;
   char    *p;

   if (!e)
      return ECORE_CONFIG_ERR_FAIL;

   if (!e->str)
     {
	e->used = e->alloc = 0;
	if (!(e->str = (char *)malloc(e->alloc = CHUNKLEN)))
	   return ECORE_CONFIG_ERR_OOM;
     }

   va_start(ap, fmt);
   need = vsnprintf(NULL, 0, fmt, ap);
   va_end(ap);

   if (need >= (e->alloc - e->used))
     {
	e->alloc = e->used + need + (CHUNKLEN - (need % CHUNKLEN));

	if (!(p = (char *)realloc(e->str, e->alloc)))
	   {
	     free(e->str);
	     e->alloc = e->used = 0;
	     return ECORE_CONFIG_ERR_OOM;
	   }
	e->str = p;
     }

   va_start(ap, fmt);
   need = vsnprintf(e->str + e->used, e->alloc - e->used, fmt, ap);
   va_end(ap);

   return e->used += need;
}

int
esprintf(char **result, const char *fmt, ...)
{
   va_list   ap;
   size_t    need;
   char     *n;

   if (!result)
      return ECORE_CONFIG_ERR_FAIL;

   va_start(ap, fmt);
   need = vsnprintf(NULL, 0, fmt, ap) + 1;
   va_end(ap);
   n = malloc(need + 1);

   if (n)
     {
	va_start(ap, fmt);
	need = vsnprintf(n, need, fmt, ap);
	va_end(ap);

	n[need] = 0;

	if(*result)
	   free(result);
	*result = n;

	return need;
     }

   return ECORE_CONFIG_ERR_OOM;
}

/*****************************************************************************/
