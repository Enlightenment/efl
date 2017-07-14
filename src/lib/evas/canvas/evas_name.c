#include "evas_common_private.h"
#include "evas_private.h"

EAPI void
evas_object_name_set(Evas_Object *eo_obj, const char *name)
{
   Evas_Object_Protected_Data *obj = efl_isa(eo_obj, EFL_CANVAS_OBJECT_CLASS) ?
            efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS) : NULL;
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
   Evas_Object_Protected_Data *obj = efl_isa(eo_obj, EFL_CANVAS_OBJECT_CLASS) ?
            efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS) : NULL;
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

   if (!efl_isa(eo_obj, EFL_CANVAS_GROUP_CLASS)) return NULL;
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

/* new in EO */
EOLIAN const char *
_efl_canvas_object_efl_object_debug_name_override_get(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   const char *norend = obj->no_render ? ":no_render" : "";
   const char *clip = obj->clip.clipees ? ":clipper" : "";
   const char *base;

   base = efl_debug_name_get(efl_super(eo_obj, EFL_CANVAS_OBJECT_CLASS));
   if (obj->cur->visible)
     {
        return eina_slstr_printf("%s%s%s:(%d,%d %dx%d)", base, norend, clip,
                                 obj->cur->geometry.x, obj->cur->geometry.y,
                                 obj->cur->geometry.w, obj->cur->geometry.h);
     }
   else
     {
        return eina_slstr_printf("%s:hidden%s%s", base, norend, clip);
     }
}
