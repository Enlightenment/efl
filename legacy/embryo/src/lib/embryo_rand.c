/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>

#include "Embryo.h"
#include "embryo_private.h"

/* exported random number api */

static Embryo_Cell
_embryo_rand_rand(Embryo_Program *ep __UNUSED__, Embryo_Cell *params __UNUSED__)
{
   return (Embryo_Cell)(rand() & 0xffff);
}

static Embryo_Cell
_embryo_rand_randf(Embryo_Program *ep __UNUSED__, Embryo_Cell *params __UNUSED__)
{
   double r;
   float f;

   r = (double)(rand() & 0xffff) / 65535.0;
   f = (float)r;
   return EMBRYO_FLOAT_TO_CELL(f);
}

/* functions used by the rest of embryo */

void
_embryo_rand_init(Embryo_Program *ep)
{
   embryo_program_native_call_add(ep, "rand",  _embryo_rand_rand);
   embryo_program_native_call_add(ep, "randf", _embryo_rand_randf);
}
