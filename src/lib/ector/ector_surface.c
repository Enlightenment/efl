#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

typedef struct _Ector_Generic_Surface_Data Ector_Generic_Surface_Data;
struct _Ector_Generic_Surface_Data
{
   int w, h;
};

void
_ector_generic_surface_size_set(Eo *obj EINA_UNUSED,
                                Ector_Generic_Surface_Data *pd,
                                int w, int h)
{
   pd->w = w;
   pd->h = h;
}

void
_ector_generic_surface_size_get(Eo *obj EINA_UNUSED,
                                Ector_Generic_Surface_Data *pd,
                                int *w, int *h)
{
   if (w) *w = pd->w;
   if (h) *h = pd->h;
}

#include "ector_generic_surface.eo.c"
