#include "edje_private.h"

#include <Eo.h>

#ifdef MY_CLASS
# undef MY_CLASS
#endif

#define MY_CLASS EDJE_OBJECT_CLASS

#define MY_CLASS_NAME "Edje"
#define MY_CLASS_NAME_LEGACY "edje"

Eina_List *_edje_edjes = NULL;

/************************** API Routines **************************/

EAPI Evas_Object *
edje_object_add(Evas *evas)
{
   Evas_Object *e;
   e = eo_add(MY_CLASS, evas);
   eo_unref(e);
   return e;
}

EOLIAN static void
_edje_object_eo_base_constructor(Eo *obj, Edje *ed)
{
   ed->base = eo_data_ref(obj, EVAS_SMART_CLIPPED_CLASS);

   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj, evas_obj_type_set(MY_CLASS_NAME_LEGACY));
   _edje_lib_ref();
}

EOLIAN static void
_edje_object_eo_base_destructor(Eo *obj, Edje *class_data EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
   eo_data_unref(obj, class_data);
}

EOLIAN static void
_edje_object_eo_base_dbg_info_get(Eo *eo_obj, Edje *_pd EINA_UNUSED, Eo_Dbg_Info *root) EINA_ARG_NONNULL(3)
{
   eo_do_super(eo_obj, MY_CLASS, eo_dbg_info_get(root));
   Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);

   const char *file, *edje_group;
   eo_do(eo_obj, efl_interface_file_get(&file, &edje_group));
   EO_DBG_INFO_APPEND(group, "File", EINA_VALUE_TYPE_STRING, file);
   EO_DBG_INFO_APPEND(group, "Group", EINA_VALUE_TYPE_STRING, edje_group);

   Edje_Load_Error error = EDJE_LOAD_ERROR_NONE;
   eo_do(eo_obj, error = edje_obj_load_error_get());
   if (error != EDJE_LOAD_ERROR_NONE)
     {
        EO_DBG_INFO_APPEND(group, "Error", EINA_VALUE_TYPE_STRING,
              edje_load_error_str(error));
     }
}

static void
_edje_color_class_free(void *data)
{
   Edje_Color_Class *cc = data;

   if (cc->name) eina_stringshare_del(cc->name);
   free(cc);
}


/* Private Routines */
EOLIAN static void
_edje_object_evas_object_smart_add(Eo *obj, Edje *ed)
{
   Evas *tev = evas_object_evas_get(obj);

   evas_event_freeze(tev);

   ed->base->evas = tev;
   ed->base->clipper = evas_object_rectangle_add(ed->base->evas);
   evas_object_static_clip_set(ed->base->clipper, 1);
   evas_object_smart_member_add(ed->base->clipper, obj);
   evas_object_color_set(ed->base->clipper, 255, 255, 255, 255);
   evas_object_move(ed->base->clipper, -100000, -100000);
   evas_object_resize(ed->base->clipper, 200000, 200000);
   evas_object_pass_events_set(ed->base->clipper, 1);
   ed->is_rtl = EINA_FALSE;
   ed->have_objects = EINA_TRUE;
   ed->references = 1;
   ed->user_defined = NULL;
   ed->color_classes = eina_hash_string_small_new(_edje_color_class_free);

   evas_object_geometry_get(obj, &(ed->x), &(ed->y), &(ed->w), &(ed->h));
   ed->obj = obj;
   _edje_edjes = eina_list_append(_edje_edjes, obj);
   /*
     {
        Eina_List *l;
        const void *data;

        printf("--- EDJE DUMP [%i]\n", eina_list_count(_edje_edjes));
        EINA_LIST_FOREACH(_edge_edges, l, data)
          {
             ed = _edje_fetch(data);
             printf("EDJE: %80s | %80s\n", ed->path, ed->part);
          }
        printf("--- EDJE DUMP [%i]\n", eina_list_count(_edje_edjes));
     }
   */
   evas_event_thaw(tev);
   evas_event_thaw_eval(tev);
}

EOLIAN static void
_edje_object_evas_object_smart_del(Eo *obj, Edje *ed)
{
   _edje_block_violate(ed);
   ed->delete_me = 1;
   _edje_edjes = eina_list_remove(_edje_edjes, obj);
   evas_object_smart_data_set(obj, NULL);
   if (_edje_script_only(ed)) _edje_script_only_shutdown(ed);
   if (_edje_lua_script_only(ed)) _edje_lua_script_only_shutdown(ed);
#ifdef HAVE_EPHYSICS
   /* clear physics world  / shutdown ephysics */
   if ((ed->collection) && (ed->collection->physics_enabled))
     {
        ephysics_world_del(ed->world);
        ephysics_shutdown();
     }
#endif
   if (ed->persp) edje_object_perspective_set(obj, NULL);
   _edje_file_del(ed);
   _edje_clean_objects(ed);
   _edje_unref(ed);
   _edje_lib_unref();
}

EOLIAN static void
_edje_object_evas_object_smart_move(Eo *obj EINA_UNUSED, Edje *ed, Evas_Coord x, Evas_Coord y)
{
   unsigned int i;

   if ((ed->x == x) && (ed->y == y)) return;
   ed->x = x;
   ed->y = y;
//   evas_object_move(ed->clipper, ed->x, ed->y);

   if (_edje_script_only(ed))
     {
        _edje_script_only_move(ed);
        return;
     }
   if (_edje_lua_script_only(ed))
     {
        _edje_lua_script_only_move(ed);
        return;
     }

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *ep;

        ep = ed->table_parts[i];
        if ((ep->type == EDJE_RP_TYPE_TEXT) && (ep->typedata.text))
          {
             evas_object_move(ep->object,
                              ed->x + ep->x + ep->typedata.text->offset.x,
                              ed->y + ep->y + ep->typedata.text->offset.y);
          }
        else
          {
             evas_object_move(ep->object, ed->x + ep->x, ed->y + ep->y);
             if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
                 (ep->typedata.swallow))
               {
                  if (ep->typedata.swallow->swallowed_object)
                    evas_object_move
                       (ep->typedata.swallow->swallowed_object,
                        ed->x + ep->x,
                        ed->y + ep->y);
               }
          }
        if (ep->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
          _edje_entry_real_part_configure(ed, ep);
     }

   if (ed->have_mapped_part)
     {
        ed->dirty = EINA_TRUE;
        ed->have_mapped_part = EINA_FALSE;
        ed->need_map_update = EINA_TRUE;
        _edje_recalc_do(ed);
        ed->need_map_update = EINA_FALSE;
     }

//   _edje_emit(ed, "move", NULL);
}

static void
_edje_limit_emit(Edje *ed, const char *limit_name, Eina_Bool over)
{
   char *buffer;
   unsigned int length;

   if (!limit_name) return;

   length = strlen(limit_name) + 13;
   buffer = alloca(length);
   snprintf(buffer, length, "limit,%s,%s", limit_name, over ? "over" : "below");
   _edje_emit(ed, buffer, NULL);
}

static void
_edje_limit_get(Edje *ed, Edje_Limit **limits, unsigned int length, Evas_Coord size_current, Evas_Coord size_next)
{
   unsigned int i;

   if (size_next == size_current) return;

   for (i = 0; i < length; ++i)
     {
        if ((size_current <= limits[i]->value) && (limits[i]->value < size_next))
          {
             _edje_limit_emit(ed, limits[i]->name, EINA_TRUE);
          }
        else if ((size_next <= limits[i]->value) && (limits[i]->value < size_current))
          {
             _edje_limit_emit(ed, limits[i]->name, EINA_FALSE);
          }
     }
}

EOLIAN static void
_edje_object_evas_object_smart_resize(Eo *obj EINA_UNUSED, Edje *ed, Evas_Coord w, Evas_Coord h)
{
   if ((w == ed->w) && (h == ed->h)) return;
   if (ed->collection)
     {
        _edje_limit_get(ed, ed->collection->limits.horizontal, ed->collection->limits.horizontal_count, ed->w, w);
        _edje_limit_get(ed, ed->collection->limits.vertical, ed->collection->limits.vertical_count, ed->h, h);
     }
   ed->w = w;
   ed->h = h;
#ifdef HAVE_EPHYSICS
   if ((ed->collection) && (ed->world))
        ephysics_world_render_geometry_set(
           ed->world, ed->x, ed->y, ed->collection->physics.world.z,
           ed->w, ed->h, ed->collection->physics.world.depth);
#endif
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = EINA_TRUE;
#endif
   if (_edje_script_only(ed))
     {
        _edje_script_only_resize(ed);
        return;
     }
   if (_edje_lua_script_only(ed))
     {
        _edje_lua_script_only_resize(ed);
        return;
     }
//   evas_object_resize(ed->clipper, ed->w, ed->h);
   ed->dirty = EINA_TRUE;
   _edje_recalc_do(ed);
   _edje_emit(ed, "resize", NULL);
}

EOLIAN static void
_edje_object_evas_object_smart_show(Eo *obj, Edje *ed)
{
   Eina_List *l;
   Edje *edg;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_show());
   if (evas_object_visible_get(obj)) return;
   if (_edje_script_only(ed))
     {
        _edje_script_only_show(ed);
        return;
     }
   if (_edje_lua_script_only(ed))
     {
        _edje_lua_script_only_show(ed);
        return;
     }
   if (eina_list_count(ed->groups) > 1)
     {
        EINA_LIST_FOREACH(ed->groups, l, edg)
          {
             Edje_Real_Part *rp;

             if (edg == ed) continue;
             rp = evas_object_data_get(edg->obj, "\377 edje.part_obj");
             if ((rp) && (rp->chosen_description->visible))
               evas_object_show(edg->obj);
          }
     }
   _edje_emit(ed, "show", NULL);
}

EOLIAN static void
_edje_object_evas_object_smart_hide(Eo *obj, Edje *ed)
{
   Eina_List *l;
   Edje *edg;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_hide());
   if (!evas_object_visible_get(obj)) return;
   if (_edje_script_only(ed))
     {
        _edje_script_only_hide(ed);
        return;
     }
   if (_edje_lua_script_only(ed))
     {
        _edje_lua_script_only_hide(ed);
        return;
     }
   EINA_LIST_FOREACH(ed->groups, l, edg)
     if (edg != ed) evas_object_hide(edg->obj);
   _edje_emit(ed, "hide", NULL);
}

EOLIAN static void
_edje_object_evas_object_smart_calculate(Eo *obj EINA_UNUSED, Edje *ed)
{
   _edje_recalc_do(ed);
}

EOLIAN static Eina_Bool
_edje_object_efl_interface_file_file_set(Eo *obj, Edje *_pd EINA_UNUSED, const char *file, const char *group)
{
   Eina_Bool ret;
   Eina_File *f = NULL;
   Eina_Array *nested;

   ret = EINA_FALSE;

   if (file)
     {
        f = eina_file_open(file, EINA_FALSE);
        if (!f) 
          {
             Edje *ed;

             ed = _edje_fetch(obj);
             ed->load_error = EDJE_LOAD_ERROR_DOES_NOT_EXIST;
             return ret;
          }
     }
   nested = eina_array_new(8);

   if (_edje_object_file_set_internal(obj, f, group, NULL, NULL, nested))
     ret = EINA_TRUE;

   eina_array_free(nested);
   eina_file_close(f);
   _edje_object_orientation_inform(obj);

   return ret;
}

EOLIAN static Eina_Bool
_edje_object_mmap_set(Eo *obj, Edje *_pd EINA_UNUSED, const Eina_File *f, const char *group)
{
   Eina_Bool ret;
   Eina_Array *nested;

   ret = EINA_FALSE;

   nested = eina_array_new(8);

   if (_edje_object_file_set_internal(obj, f, group, NULL, NULL, nested))
     ret = EINA_TRUE;

   eina_array_free(nested);
   _edje_object_orientation_inform(obj);

   return ret;
}

EAPI Eina_Bool
edje_object_file_set(Eo *obj, const char *file, const char *group)
{
   Eina_Bool ret = 0;
   eo_do((Eo *) obj, ret = efl_interface_file_set(file, group));
   return ret;
}

EAPI void
edje_object_file_get(const Eo *obj, const char **file, const char **group)
{
   eo_do((Eo *) obj, efl_interface_file_get(file, group));
}

#include "edje_object.eo.c"

