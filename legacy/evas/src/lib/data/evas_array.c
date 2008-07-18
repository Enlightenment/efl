/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>

#include "Evas_Data.h"


#ifdef __GNUC__
# define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
# define UNLIKELY(x) (x)
#endif


static Evas_Bool _evas_array_grow(Evas_Array *array);

static inline void
_evas_array_append(Evas_Array *array, void *data)
{
   if (UNLIKELY((array->count + array->step) > array->total))
     if (!_evas_array_grow(array)) return ;

   array->data[array->count++] = data;
}

static inline void*
_evas_array_get(Evas_Array *array, unsigned int index)
{
   return array->data[index];
}


static Evas_Bool
_evas_array_grow(Evas_Array *array)
{
   void **tmp;
   size_t total;

   total = array->total + array->step;
   tmp = realloc(array->data, sizeof (void*) * total);
   if (!tmp) return 0;

   array->total = total;
   array->data = tmp;

   return 1;
}

EAPI void
evas_array_append(Evas_Array *array, void *data)
{
   _evas_array_append(array, data);
}

EAPI void*
evas_array_get(Evas_Array *array, unsigned int index)
{
   return _evas_array_get(array, index);
}

EAPI void
evas_array_clean(Evas_Array *array)
{
   array->count = 0;
}

EAPI void
evas_array_setup(Evas_Array *array, unsigned int step)
{
   array->step = step;
}

EAPI void
evas_array_flush(Evas_Array *array)
{
   array->count = 0;
   array->total = 0;

   if (array->data) free(array->data);
   array->data = NULL;
}

EAPI Evas_Array*
evas_array_new(unsigned int step)
{
   Evas_Array *array;

   array = malloc(sizeof (Evas_Array));
   if (!array) return NULL;

   array->data = NULL;
   array->total = 0;
   array->count = 0;
   array->step = step;

   return array;
}

EAPI void
evas_array_free(Evas_Array *array)
{
   evas_array_flush(array);
   free(array);
}

EAPI void
evas_array_remove(Evas_Array *array, Evas_Bool (*keep)(void *data, void *gdata), void *gdata)
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

	data = _evas_array_get(array, i);

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
