#include "evas_common_private.h"
#include "evas_private.h"


EAPI void
evas_object_data_set(Evas_Object *obj, const char *key, const void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(obj, eo_base_data_set(key, data, NULL));
}

EAPI void *
evas_object_data_get(const Evas_Object *obj, const char *key)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   void *data = NULL;
   eo_do((Evas_Object *)obj, data = eo_base_data_get(key));
   return data;
}

EAPI void *
evas_object_data_del(Evas_Object *obj, const char *key)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   void *data = NULL;
   eo_do(obj, data = eo_base_data_get(key), eo_base_data_del(key));
   return data;
}
