#include "evas_common.h"
#include "evas_private.h"

EAPI void
evas_object_name_set(Evas_Object *eo_obj, const char *name)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_name_set(name));
}

void
_name_set(Eo *eo_obj, void *_pd, va_list *list)
{
   const char *name = va_arg(*list, const char *);

   Evas_Object_Protected_Data *obj = _pd;
   if (obj->name)
     {
        eina_hash_del(obj->layer->evas->name_hash, obj->name, eo_obj);
        free(obj->name);
     }
   if (!name) obj->name = NULL;
   else
     {
        obj->name = strdup(name);
        eina_hash_add(obj->layer->evas->name_hash, obj->name, eo_obj);
     }
}

EAPI const char *
evas_object_name_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   const char *name = NULL;
   eo_do((Eo *)eo_obj, evas_obj_name_get(&name));
   return name;
}

void
_name_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **name = va_arg(*list, const char **);
   const Evas_Object_Protected_Data *obj = _pd;
   *name = obj->name;
}

EAPI Evas_Object *
evas_object_name_find(const Evas *e, const char *name)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *ret = NULL;
   eo_do((Eo *)e, evas_canvas_object_name_find(name, &ret));
   return ret;
}

void
_canvas_object_name_find(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   const char *name = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   const Evas_Public_Data *e = _pd;
   if (!name) *ret = NULL;
   else *ret = (Evas_Object *)eina_hash_find(e->name_hash, name);
}

static Evas_Object *
_evas_object_name_child_find(const Evas_Object *eo_obj, const char *name, int recurse)
{
   const Eina_Inlist *lst;
   Evas_Object_Protected_Data *child;

   if (!eo_isa(eo_obj, EVAS_OBJ_SMART_CLASS)) return NULL;
   lst = evas_object_smart_members_get_direct(eo_obj);
   EINA_INLIST_FOREACH(lst, child)
     {
        if (child->delete_me) continue;
        if (!child->name) continue;
        if (!strcmp(name, child->name)) return child->object;
        if (recurse != 0)
          {
             if ((eo_obj = _evas_object_name_child_find(child->object, name, recurse - 1)))
               return (Evas_Object *)eo_obj;
          }
     }
   return NULL;
}

EAPI Evas_Object *
evas_object_name_child_find(const Evas_Object *eo_obj, const char *name, int recurse)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *child = NULL;
   eo_do((Eo *)eo_obj, evas_obj_name_child_find(name, recurse, &child));
   return child;
}

void
_name_child_find(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *name = va_arg(*list, const char *);
   int recurse = va_arg(*list, int);
   Evas_Object **child = va_arg(*list, Evas_Object **);
   *child = (!name ?  NULL : _evas_object_name_child_find(eo_obj, name, recurse));
}
