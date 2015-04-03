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

Eina_Bool
_ector_generic_surface_update_push(Eo *obj,
                                   Ector_Generic_Surface_Data *pd,
                                   const Eina_Rectangle *r,
                                   Ector_Update_Type type)
{
}

Eina_Bool
_ector_generic_surface_update_reset(Eo *obj, Ector_Generic_Surface_Data *pd)
{
}

#include "ector_generic_surface.eo.c"
