/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>

#include "eina_array.h"
#include "eina_inline_array.x"

EAPI void
eina_array_append(Eina_Array *array, void *data)
{
   _eina_array_append(array, data);
}

EAPI void *
eina_array_get(Eina_Array *array, unsigned int index)
{
   return _eina_array_get(array, index);
}

EAPI void
eina_array_clean(Eina_Array *array)
{
   array->count = 0;
}

EAPI void
eina_array_setup(Eina_Array *array, unsigned int step)
{
   array->step = step;
}

EAPI void
eina_array_flush(Eina_Array *array)
{
   array->count = 0;
   array->total = 0;

   if (array->data) free(array->data);
   array->data = NULL;
}

EAPI Eina_Array *
eina_array_new(unsigned int step)
{
   Eina_Array *array;

   array = malloc(sizeof (Eina_Array));
   if (!array) return NULL;

   array->data = NULL;
   array->total = 0;
   array->count = 0;
   array->step = step;

   return array;
}

EAPI void
eina_array_free(Eina_Array *array)
{
   eina_array_flush(array);
   free(array);
}

EAPI void
eina_array_remove(Eina_Array *array, Eina_Bool (*keep)(void *data, void *gdata), void *gdata)
{
   void **tmp;
   unsigned int total = 0;
   unsigned int i;

   if (array->total == 0) return ;

   tmp = malloc(sizeof (void*) * array->total);
   if (!tmp) return ;

   for (i = 0; i < array->count; i++)
     {
	void *data;

	data = _eina_array_get(array, i);

	if (keep(data, gdata))
	  {
	     tmp[total] = data;
	     total++;
	  }
     }

   free(array->data);

   if (total == 0)
     {
	array->total = 0;
	array->data = NULL;
	free(tmp);
     }
   else
     {
	array->data = tmp;
     }

   array->count = total;
}
