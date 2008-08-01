#ifndef EINA_ARRAY_H_
#define EINA_ARRAY_H_

#include <stdlib.h>

#include "eina_types.h"

typedef struct _Eina_Array       Eina_Array;             /**< A generic vector */
struct _Eina_Array /** An array of data */
{
   void		**data;   /**< Pointer to a vector of pointer to payload */
   unsigned int   total;  /**< Total number of slot in the vector */
   unsigned int   count;  /**< Number of activ slot in the vector */
   unsigned int	  step;   /**< How much must we grow the vector When it is full */
};

EAPI Eina_Array *eina_array_new                  (unsigned int step);
EAPI void        eina_array_setup                (Eina_Array *array, unsigned int step);
EAPI void        eina_array_free                 (Eina_Array *array);
EAPI void        eina_array_append               (Eina_Array *array, void *data);
EAPI void       *eina_array_get                  (Eina_Array *array, unsigned int index);
EAPI void        eina_array_clean                (Eina_Array *array);
EAPI void        eina_array_flush                (Eina_Array *array);
EAPI void        eina_array_remove               (Eina_Array *array, Eina_Bool (*keep)(void *data, void *gdata), void *gdata);

#define EINA_ARRAY_ITER_NEXT(array, index, item) for ((index) = 0, (item) = _eina_array_get((array), (index)); (index) < (array)->count; ++(index), (item) = _eina_array_get((array), (index)))

#include "eina_inline_array.x"

#endif
