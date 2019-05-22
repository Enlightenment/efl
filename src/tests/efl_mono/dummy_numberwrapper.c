#include "libefl_mono_native_test.h"

typedef struct Dummy_Numberwrapper_Data
{
   int number;
} Dummy_Numberwrapper_Data;

void _dummy_numberwrapper_number_set(EINA_UNUSED Eo *obj, Dummy_Numberwrapper_Data *pd, int n)
{
   pd->number = n;
}

int _dummy_numberwrapper_number_get(EINA_UNUSED const Eo *obj, Dummy_Numberwrapper_Data *pd)
{
   return pd->number;
}

#include "dummy_numberwrapper.eo.c"
