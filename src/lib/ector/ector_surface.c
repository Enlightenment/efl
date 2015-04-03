#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

typedef struct _Ector_Generic_Surface_Data Ector_Generic_Surface_Data;
struct _Ector_Generic_Surface_Data
{
};

void
_ector_generic_surface_size_set(Eo *obj,
                                Ector_Generic_Surface_Data *pd,
                                int w, int h)
{
}

void
_ector_generic_surface_size_get(Eo *obj,
                                Ector_Generic_Surface_Data *pd,
                                int *w, int *h)
{
}

#include "ector_generic_surface.eo.c"
