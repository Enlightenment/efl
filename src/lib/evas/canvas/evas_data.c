#include "evas_common_private.h"
#include "evas_private.h"

EAPI void
evas_object_data_set(Evas_Object *eo_obj, const char *key, const void *data)
{
   if (!efl_isa(eo_obj, EFL_CANVAS_OBJECT_CLASS)) return;
   efl_key_data_set(eo_obj, key, data);
}

EAPI void *
evas_object_data_get(const Evas_Object *eo_obj, const char *key)
{
   if (!efl_isa(eo_obj, EFL_CANVAS_OBJECT_CLASS)) return NULL;
   return efl_key_data_get(eo_obj, key);
}

EAPI void *
evas_object_data_del(Evas_Object *eo_obj, const char *key)
{
   void *data;

   if (!efl_isa(eo_obj, EFL_CANVAS_OBJECT_CLASS)) return NULL;
   data = efl_key_data_get(eo_obj, key);
   efl_key_data_set(eo_obj, key, NULL);
   return data;
}
