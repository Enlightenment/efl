#include "evas_common_private.h"
#include "evas_private.h"


EAPI void
evas_object_data_set(Evas_Object *obj, const char *key, const void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   efl_key_data_set(obj, key, data);
}

EAPI void *
evas_object_data_get(const Evas_Object *obj, const char *key)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   void *data = NULL;
   data = efl_key_data_get((Evas_Object *)obj, key);
   return data;
}

EAPI void *
evas_object_data_del(Evas_Object *obj, const char *key)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   void *data = NULL;
   data = efl_key_data_get(obj, key);
   efl_key_data_set(obj, key, NULL);
   return data;
}
