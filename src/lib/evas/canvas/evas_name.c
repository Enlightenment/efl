#include "evas_common_private.h"
#include "evas_private.h"

EAPI void
evas_object_name_set(Evas_Object *eo_obj, const char *name)
{
   Evas_Object_Protected_Data *obj = eo_isa(eo_obj, EVAS_OBJECT_CLASS) ?
            eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS) : NULL;
   if (!obj) return;
   if (obj->name)
     {
        if (obj->layer && obj->layer->evas && obj->layer->evas->name_hash)
          eina_hash_del(obj->layer->evas->name_hash, obj->name, eo_obj);
        free(obj->name);
     }
   if (!name) obj->name = NULL;
   else
     {
        obj->name = strdup(name);
        if (obj->layer && obj->layer->evas && obj->layer->evas->name_hash)
          eina_hash_add(obj->layer->evas->name_hash, obj->name, eo_obj);
     }
}

EAPI const char *
evas_object_name_get(const Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_isa(eo_obj, EVAS_OBJECT_CLASS) ?
            eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS) : NULL;
   if (!obj) return NULL;
   return obj->name;
}

EOLIAN Evas_Object*
_evas_canvas_object_name_find(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, const char *name)
{
   if (!name) return NULL;
   else return (Evas_Object *)eina_hash_find(e->name_hash, name);
}

static Evas_Object *
_priv_evas_object_name_child_find(const Evas_Object *eo_obj, const char *name, int recurse)
{
   const Eina_Inlist *lst;
   Evas_Object_Protected_Data *child;

   if (!eo_isa(eo_obj, EFL_CANVAS_GROUP_CLASS)) return NULL;
   lst = evas_object_smart_members_get_direct(eo_obj);
   EINA_INLIST_FOREACH(lst, child)
     {
        if (child->delete_me) continue;
        if (!child->name) continue;
        if (!strcmp(name, child->name)) return child->object;
        if (recurse != 0)
          {
             if ((eo_obj = _priv_evas_object_name_child_find(child->object, name, recurse - 1)))
               return (Evas_Object *)eo_obj;
          }
     }
   return NULL;
}

EAPI Evas_Object *
evas_object_name_child_find(const Evas_Object *eo_obj, const char *name, int recurse)
{
   return (!name ?  NULL : _priv_evas_object_name_child_find(eo_obj, name, recurse));
}

