#include "config.h"
#include "Efl.h"

EOLIAN static Eina_Bool
_efl_io_closer_closed_set(Eo *obj, void *pd EINA_UNUSED, Eina_Bool is_closed)
{
   if (is_closed)
     return efl_io_closer_close(obj) == 0;

   return EINA_FALSE;
}

#include "interfaces/efl_io_closer.eo.c"
