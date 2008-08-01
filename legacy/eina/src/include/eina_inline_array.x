#ifndef EVAS_INLINE_ARRAY_H
#define EVAS_INLINE_ARRAY_H

#ifdef __GNUC__
# define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
# define UNLIKELY(x) (x)
#endif

static inline Eina_Bool
_eina_array_grow(Eina_Array *array)
{
   void **tmp;
   unsigned int total;

   total = array->total + array->step;
   tmp = realloc(array->data, sizeof (void*) * total);
   if (UNLIKELY(!tmp)) return 0;

   array->total = total;
   array->data = tmp;

   return 1;
}

static inline void
_eina_array_append(Eina_Array *array, void *data)
{
   if (UNLIKELY((array->count + array->step) > array->total))
     if (!_eina_array_grow(array)) return ;

   array->data[array->count++] = data;
}

static inline void *
_eina_array_get(Eina_Array *array, unsigned int index)
{
   return array->data[index];
}

static inline unsigned int
eina_array_count(Eina_Array *array)
{
   return array->count;
}

#endif
