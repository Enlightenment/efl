#include "config.h"
#include "Efl.h"

EOLIAN static Eina_Bool
_efl_io_positioner_position_set(Eo *o, void *pd EINA_UNUSED, uint64_t position)
{
   return efl_io_positioner_seek(o, position, EFL_IO_POSITIONER_WHENCE_START) == 0;
}

#include "interfaces/efl_io_positioner.eo.c"
