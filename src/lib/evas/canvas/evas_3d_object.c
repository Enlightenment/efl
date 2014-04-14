#include "evas_common_private.h"
#include "evas_private.h"

#include "Eo.h"

#define MY_CLASS EO_EVAS_3D_OBJECT_CLASS

EOLIAN static void
_eo_evas_3d_object_eo_base_constructor(Eo *obj, Evas_3D_Object_Data *pd)
{
   Eo *e = NULL;
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj, e = eo_parent_get());
   pd->evas = e;
   pd->type = EVAS_3D_OBJECT_TYPE_INVALID;
   memset(&pd->dirty[0], 0x00, sizeof(Eina_Bool) * EVAS_3D_STATE_MAX);
}


EOLIAN static Evas *
 _eo_evas_3d_object_evas_common_interface_evas_get(Eo *obj EINA_UNUSED, Evas_3D_Object_Data *pd)
{
   return pd->evas;
}

EOLIAN static void
_eo_evas_3d_object_type_set(Eo *obj EINA_UNUSED, Evas_3D_Object_Data *pd, Evas_3D_Object_Type type)
{
   pd->type = type;
}

EOLIAN static Evas_3D_Object_Type
_eo_evas_3d_object_type_get(Eo *obj EINA_UNUSED, Evas_3D_Object_Data *pd)
{
   return pd->type;
}

EOLIAN static Eina_Bool
_eo_evas_3d_object_dirty_get(Eo *obj EINA_UNUSED, Evas_3D_Object_Data *pd, Evas_3D_State state)
{
   return pd->dirty[state];
}

EOLIAN static void
_eo_evas_3d_object_change(Eo *obj, Evas_3D_Object_Data *pd, Evas_3D_State state, Evas_3D_Object *ref)
{
   /* Skip already dirty properties. */
   if (pd->dirty[state])
     return;

   pd->dirty[state] = EINA_TRUE;
   pd->dirty[EVAS_3D_STATE_ANY] = EINA_TRUE;

   eo_do(obj, eo_evas_3d_object_change_notify(state, ref));
}

EOLIAN static void
_eo_evas_3d_object_update(Eo *obj, Evas_3D_Object_Data *pd)
{
   if (!pd->dirty[EVAS_3D_STATE_ANY])
     return;

   eo_do(obj, eo_evas_3d_object_update_notify());

   memset(&pd->dirty[0], 0x00, sizeof(Eina_Bool) * EVAS_3D_STATE_MAX);
}

#include "canvas/evas_3d_object.eo.c"
