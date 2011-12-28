#include "evas_common.h"
#include "evas_private.h"

EAPI void
evas_object_name_set(Evas_Object *obj, const char *name)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->name)
     {
        eina_hash_del(obj->layer->evas->name_hash, obj->name, obj);
        free(obj->name);
     }
   if (!name) obj->name = NULL;
   else
     {
        obj->name = strdup(name);
        eina_hash_add(obj->layer->evas->name_hash, obj->name, obj);
     }
}

EAPI const char *
evas_object_name_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   return obj->name;
}

EAPI Evas_Object *
evas_object_name_find(const Evas *e, const char *name)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   if (!name) return NULL;
   return (Evas_Object *)eina_hash_find(e->name_hash, name);
}

static Evas_Object *
_evas_object_name_child_find(const Evas_Object *obj, const char *name, int recurse)
{
   const Eina_Inlist *lst;
   Evas_Object *child;
   
   if (!obj->smart.smart) return NULL;
   lst = evas_object_smart_members_get_direct(obj);
   EINA_INLIST_FOREACH(lst, child)
     {
        if (child->delete_me) continue;
        if (!child->name) continue;
        if (!strcmp(name, child->name)) return child;
        if (recurse != 0)
          {
             if ((obj = _evas_object_name_child_find(child, name, recurse - 1)))
               return obj;
          }
     }
   return NULL;
}

EAPI Evas_Object *
evas_object_name_child_find(const Evas_Object *obj, const char *name, int recurse)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!name) return NULL;
   return _evas_object_name_child_find(obj, name, recurse);
}
