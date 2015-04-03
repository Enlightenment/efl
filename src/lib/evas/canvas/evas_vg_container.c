#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EVAS_VG_CONTAINER_CLASS

void
_evas_vg_container_eo_base_constructor(Eo *obj, Evas_VG_Container_Data *pd)
{
   Eo *parent;

   eo_do_super(obj, MY_CLASS, eo_constructor());
}

Eina_Bool
_evas_vg_container_evas_vg_node_bound_get(Eo *obj,
                                          Evas_VG_Container_Data *pd,
                                          Eina_Rectangle *r)
{
   // FIXME: iterate children and get their boundary to
}


#include "evas_vg_container.eo.c"
