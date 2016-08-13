#include "config.h"
#include "Efl.h"

EOLIAN static Eina_Bool
_efl_io_sizer_size_set(Eo *o, void *pd EINA_UNUSED, uint64_t size)
{
   return efl_io_sizer_resize(o, size) == 0;
}

#include "interfaces/efl_io_sizer.eo.c"
