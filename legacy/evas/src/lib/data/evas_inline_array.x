#ifndef EVAS_INLINE_ARRAY_H
#define EVAS_INLINE_ARRAY_H

#ifdef __GNUC__
# define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
# define UNLIKELY(x) (x)
#endif

static inline Evas_Bool
_evas_array_grow(Evas_Array *array)
{
   void **tmp;
   size_t total;

   total = array->total + array->step;
   tmp = (void **)realloc(array->data, sizeof (void*) * total);
   if (!tmp) return 0;

   array->total = total;
   array->data = tmp;

   return 1;
}

static inline void
_evas_array_append(Evas_Array *array, void *data)
{
   if (UNLIKELY((array->count + array->step) > array->total))
     if (!_evas_array_grow(array)) return ;

   array->data[array->count++] = data;
}

static inline void *
_evas_array_get(Evas_Array *array, unsigned int index)
{
   return array->data[index];
}

#endif
