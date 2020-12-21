#include "evas_common_private.h"
#include "evas_private.h"

EVAS_API void
evas_object_name_set(Evas_Object *eo_obj, const char *name)
{
   Evas_Object_Protected_Data *obj = efl_isa(eo_obj, EFL_CANVAS_OBJECT_CLASS) ?
            efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS) : NULL;
   if (!obj) return;
   if (obj->name)
     {
        if (obj->layer && obj->layer->evas && obj->layer->evas->name_hash)
          eina_hash_list_remove(obj->layer->evas->name_hash, obj->name, eo_obj);
        free(obj->name);
     }
   if (!name) obj->name = NULL;
   else
     {
        obj->name = strdup(name);
        if (obj->layer && obj->layer->evas && obj->layer->evas->name_hash)
          eina_hash_list_prepend(obj->layer->evas->name_hash, obj->name, eo_obj);
     }
}

EVAS_API const char *
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
   else return eina_list_data_get(eina_hash_find(e->name_hash, name));
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

EVAS_API Evas_Object *
evas_object_name_child_find(const Evas_Object *eo_obj, const char *name, int recurse)
{
   return (!name ?  NULL : _priv_evas_object_name_child_find(eo_obj, name, recurse));
}

/* new in EO */
EOLIAN void
_efl_canvas_object_efl_object_debug_name_override(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Strbuf *sb)
{
   const char *norend = obj->no_render ? ":no_render" : "";
   const char *clip = obj->clip.clipees ? ":clipper" : "";

   efl_debug_name_override(efl_super(eo_obj, EFL_CANVAS_OBJECT_CLASS), sb);
   if (!obj->cur)
     {
        eina_strbuf_append_printf(sb, ":nostate");
     }
   else if (obj->cur->visible)
     {
        eina_strbuf_append_printf(sb, "%s%s:(%d,%d %dx%d)", norend, clip,
                                  obj->cur->geometry.x, obj->cur->geometry.y,
                                  obj->cur->geometry.w, obj->cur->geometry.h);
        if ((obj->cur->color.r != 255) || (obj->cur->color.g != 255) ||
            (obj->cur->color.b != 255) || (obj->cur->color.a != 255))
          {
             eina_strbuf_append_printf(sb, ":rgba(%d,%d,%d,%d)",
                                       obj->cur->color.r, obj->cur->color.g,
                                       obj->cur->color.b, obj->cur->color.a);
          }
     }
   else
     {
        eina_strbuf_append_printf(sb, ":hidden%s%s", norend, clip);
     }
}
