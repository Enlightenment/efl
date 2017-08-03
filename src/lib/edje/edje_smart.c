#define EFL_CANVAS_GROUP_PROTECTED
#define EFL_CANVAS_GROUP_BETA

#include "edje_private.h"

#include <Eo.h>

#ifdef MY_CLASS
# undef MY_CLASS
#endif

#define MY_CLASS             EDJE_OBJECT_CLASS

#define MY_CLASS_NAME        "Edje"
#define MY_CLASS_NAME_LEGACY "edje"

Eina_Inlist *_edje_edjes = NULL;

/************************** API Routines **************************/

EAPI Evas_Object *
edje_object_add(Evas *evas)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(evas, NULL);
   return efl_add(MY_CLASS, evas, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_edje_object_efl_object_constructor(Eo *obj, Edje *ed)
{
   Eo *parent;
   Evas *e;
   void *tmp;

   ed->base = efl_data_ref(obj, EFL_CANVAS_GROUP_CLIPPED_CLASS);
   ed->duration_scale = 1.0;

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   _edje_lib_ref();

   parent = efl_parent_get(obj);
   e = evas_object_evas_get(parent);
   tmp = ecore_evas_ecore_evas_get(e);

   ed->canvas_animator = !!tmp;

   return obj;
}

EOLIAN static void
_edje_object_efl_object_destructor(Eo *obj, Edje *class_data)
{
   if (class_data->file_obj)
     {
        efl_del(class_data->file_obj);
        class_data->file_obj = NULL;
     }
   efl_destructor(efl_super(obj, MY_CLASS));
   efl_data_unref(obj, class_data->base);
}

EOLIAN static Eina_Strbuf *
_edje_object_efl_object_debug_name_override(Eo *obj, Edje *ed, Eina_Strbuf *sb)
{
   sb = efl_debug_name_override(efl_super(obj, MY_CLASS), sb);
   eina_strbuf_append_printf(sb, ":file='%s':group='%s'",
                             ed->file ? eina_file_filename_get(ed->file->f) : NULL,
                             ed->group);
   return sb;
}

EOLIAN static void
_edje_object_efl_object_dbg_info_get(Eo *eo_obj, Edje *_pd EINA_UNUSED, Efl_Dbg_Info *root) EINA_ARG_NONNULL(3)
{
   efl_dbg_info_get(efl_super(eo_obj, MY_CLASS), root);
   Efl_Dbg_Info *group = EFL_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);
   Edje_Load_Error error;

   const char *file, *edje_group;
   efl_file_get(eo_obj, &file, &edje_group);
   EFL_DBG_INFO_APPEND(group, "File", EINA_VALUE_TYPE_STRING, file);
   EFL_DBG_INFO_APPEND(group, "Group", EINA_VALUE_TYPE_STRING, edje_group);

   error = edje_object_load_error_get(eo_obj);
   if (error != EDJE_LOAD_ERROR_NONE)
     {
        EFL_DBG_INFO_APPEND(group, "Error", EINA_VALUE_TYPE_STRING,
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

static void
_edje_text_class_free(void *data)
{
   Edje_Text_Class *tc = data;

   if (tc->name) eina_stringshare_del(tc->name);
   if (tc->font) eina_stringshare_del(tc->font);
   free(tc);
}

static void
_edje_size_class_free(void *data)
{
   Edje_Size_Class *sc = data;

   if (sc->name) eina_stringshare_del(sc->name);
   free(sc);
}

/* Private Routines */
EOLIAN static void
_edje_object_efl_canvas_group_group_add(Eo *obj, Edje *ed)
{
   Evas *tev = evas_object_evas_get(obj);

   evas_event_freeze(tev);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   ed->is_rtl = EINA_FALSE;
   ed->have_objects = EINA_TRUE;
   ed->references = 1;
   ed->user_defined = NULL;
   ed->color_classes = eina_hash_string_small_new(_edje_color_class_free);
   ed->text_classes = eina_hash_string_small_new(_edje_text_class_free);
   ed->size_classes = eina_hash_string_small_new(_edje_size_class_free);

   evas_object_geometry_get(obj, &(ed->x), &(ed->y), &(ed->w), &(ed->h));
   ed->obj = obj;
   _edje_edjes = eina_inlist_append(_edje_edjes, EINA_INLIST_GET(ed));
   evas_event_thaw(tev);
   evas_event_thaw_eval(tev);
}

EOLIAN static void
_edje_object_efl_canvas_group_group_del(Eo *obj, Edje *ed)
{
   _edje_block_violate(ed);
   ed->delete_me = 1;
   _edje_edjes = eina_inlist_remove(_edje_edjes, EINA_INLIST_GET(ed));
   evas_object_smart_data_set(obj, NULL);
   if (_edje_lua_script_only(ed)) _edje_lua_script_only_shutdown(ed);
#ifdef HAVE_EPHYSICS
   /* clear physics world  / shutdown ephysics */
   if ((ed->collection) && (ed->collection->physics_enabled) && (ed->world))
     {
        if (EPH_LOAD())
          {
             EPH_CALL(ephysics_world_del)(ed->world);
             EPH_CALL(ephysics_shutdown)();
          }
     }
#endif
   if (ed->persp) edje_object_perspective_set(obj, NULL);
   _edje_file_del(ed);
   _edje_clean_objects(ed);
   _edje_unref(ed);
   _edje_lib_unref();
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_edje_object_efl_gfx_position_set(Eo *obj, Edje *ed, Evas_Coord x, Evas_Coord y)
{
   unsigned short i;

   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, x, y))
     return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);

   if ((ed->x == x) && (ed->y == y)) return;
   ed->x = x;
   ed->y = y;
//   evas_object_move(ed->clipper, ed->x, ed->y);

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
_edje_object_efl_gfx_size_set(Eo *obj, Edje *ed, Evas_Coord w, Evas_Coord h)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, w, h))
     return;

   if ((w == ed->w) && (h == ed->h)) goto super;
   if (ed->collection)
     {
        _edje_limit_get(ed, ed->collection->limits.horizontal, ed->collection->limits.horizontal_count, ed->w, w);
        _edje_limit_get(ed, ed->collection->limits.vertical, ed->collection->limits.vertical_count, ed->h, h);
     }
   ed->w = w;
   ed->h = h;
#ifdef HAVE_EPHYSICS
   if ((ed->collection) && (ed->world))
     {
        if (EPH_LOAD())
          EPH_CALL(ephysics_world_render_geometry_set)
            (ed->world, ed->x, ed->y, ed->collection->physics.world.z,
             ed->w, ed->h, ed->collection->physics.world.depth);
     }
#endif
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = EINA_TRUE;
#endif
   if (_edje_lua_script_only(ed))
     {
        _edje_lua_script_only_resize(ed);
        goto super;
     }
//   evas_object_resize(ed->clipper, ed->w, ed->h);
   ed->dirty = EINA_TRUE;
   _edje_recalc_do(ed);
   _edje_emit(ed, "resize", NULL);

super:
   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);
}

static void
_edje_object_show(Eo *obj, Edje *ed)
{
   Eina_List *l;
   Edje *edg;

   efl_gfx_visible_set(efl_super(obj, MY_CLASS), EINA_TRUE);
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

static void
_edje_object_hide(Eo *obj, Edje *ed)
{
   Eina_List *l;
   Edje *edg;

   efl_gfx_visible_set(efl_super(obj, MY_CLASS), EINA_FALSE);
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
_edje_object_efl_gfx_visible_set(Eo *obj, Edje *ed, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   if (vis) _edje_object_show(obj, ed);
   else _edje_object_hide(obj, ed);
}

EOLIAN static void
_edje_object_efl_canvas_object_no_render_set(Eo *obj, Edje *ed, Eina_Bool enable)
{
   Eina_List *l;
   Edje *edg;

   enable = !!enable;
   if (efl_canvas_object_no_render_get(obj) == enable) return;
   efl_canvas_object_no_render_set(efl_super(obj, MY_CLASS), enable);

   EINA_LIST_FOREACH(ed->groups, l, edg)
     if (edg != ed) efl_canvas_object_no_render_set(edg->obj, enable);
}

EOLIAN static void
_edje_object_efl_canvas_group_group_calculate(Eo *obj EINA_UNUSED, Edje *ed)
{
   _edje_recalc_do(ed);
}

EOLIAN static Eina_Bool
_edje_object_efl_file_file_set(Eo *obj, Edje *ed, const char *file, const char *group)
{
   Eina_Bool ret;
   Eina_File *f = NULL;
   Eina_Array *nested;

   ret = EINA_FALSE;

   if (ed->file_obj)
     {
        efl_del(ed->file_obj);
        ed->file_obj = NULL;
     }
   if (file)
     {
        const char *file2;

        ed->file_obj = efl_vpath_manager_fetch(EFL_VPATH_MANAGER_CLASS, file);
        efl_vpath_file_do(ed->file_obj);
        // XXX:FIXME: allow this to be async
        efl_vpath_file_wait(ed->file_obj);
        file2 = efl_vpath_file_result_get(ed->file_obj);

        f = eina_file_open(file2, EINA_FALSE);
        if ((ed->file_obj) && (!efl_vpath_file_keep_get(ed->file_obj)))
          {
             efl_del(ed->file_obj);
             ed->file_obj = NULL;
          }
        if (!f)
          {
             efl_del(ed->file_obj);
             ed->file_obj = NULL;
             if (ed->path) eina_stringshare_del(ed->path);
             ed->path = NULL;
             ed->load_error = EDJE_LOAD_ERROR_DOES_NOT_EXIST;
             return ret;
          }
     }
   nested = eina_array_new(8);

   if (_edje_object_file_set_internal(obj, f, group, NULL, NULL, nested))
     {
        if (file)
          {
             ed->path = eina_stringshare_add(file);
          }
        else
          {
             if (ed->path) eina_stringshare_del(ed->path);
             ed->path = NULL;
          }
        ret = EINA_TRUE;
     }
   else
     {
        if (ed->path) eina_stringshare_del(ed->path);
        ed->path = NULL;
     }

   eina_array_free(nested);
   eina_file_close(f);
   _edje_object_orientation_inform(obj);

   return ret;
}

EOLIAN static Eina_Bool
_edje_object_efl_file_mmap_set(Eo *obj, Edje *pd EINA_UNUSED,
                               const Eina_File *f, const char *key)
{
   Eina_Bool ret;
   Eina_Array *nested;

   ret = EINA_FALSE;

   nested = eina_array_new(8);

   if (_edje_object_file_set_internal(obj, f, key, NULL, NULL, nested))
     ret = EINA_TRUE;

   eina_array_free(nested);
   _edje_object_orientation_inform(obj);

   return ret;
}

EOLIAN static void
_edje_object_efl_file_mmap_get(Eo *obj EINA_UNUSED, Edje *pd,
                               const Eina_File **f, const char **key)
{
   if (f) *f = pd->file->f;
   if (key) *key = pd->group;
}

EAPI Eina_Bool
edje_object_mmap_set(Edje_Object *obj, const Eina_File *file, const char *group)
{
   return efl_file_mmap_set(obj, file, group);
}

EAPI Eina_Bool
edje_object_file_set(Edje_Object *obj, const char *file, const char *group)
{
   return efl_file_set(obj, file, group);
}

EAPI void
edje_object_file_get(const Edje_Object *obj, const char **file, const char **group)
{
   efl_file_get((Edje_Object *)obj, file, group);
}

EOLIAN static void
_edje_object_efl_canvas_object_paragraph_direction_set(Eo *obj, Edje *ed, Evas_BiDi_Direction dir)
{
   efl_canvas_object_paragraph_direction_set(efl_super(obj, MY_CLASS), dir);

   /* Make it dirty to recalculate edje.
      It needs to move text objects according to new paragraph direction */
   ed->dirty = EINA_TRUE;
   efl_canvas_group_need_recalculate_set(obj, 1);
}

EOLIAN static void
_edje_object_efl_observer_update(Eo *obj EINA_UNUSED, Edje *ed, Efl_Object *obs, const char *key, void *data)
{
   if (!obs) return;

   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;

   if ((obs == _edje_color_class_member) || (obs == _edje_size_class_member))
     {
#ifdef EDJE_CALC_CACHE
        ed->all_part_change = EINA_TRUE;
#endif
     }
   else if (obs == _edje_text_class_member)
     {
        _edje_textblock_styles_cache_free(ed, key);
        _edje_textblock_style_all_update(ed);
#ifdef EDJE_CALC_CACHE
        ed->text_part_change = EINA_TRUE;
#endif
     }

   _edje_recalc(ed);

   if (obs == _edje_color_class_member)
     {
        if (data)
          _edje_emit(ed, (const char *)data, key);

        if ((ed->file) && (ed->file->color_tree))
          {
             Edje_Color_Tree_Node *ctn = NULL;
             Eina_List *l = NULL;
             char *name;

             EINA_LIST_FOREACH(ed->file->color_tree, l, ctn)
               {
                  if (!strcmp(ctn->name, key) && (ctn->color_classes))
                    {
                       EINA_LIST_FOREACH(ctn->color_classes, l, name)
                         efl_observable_observers_update(_edje_color_class_member, name, data);

                       break;
                    }
               }
          }
     }
}

/* Internal EO APIs and hidden overrides */

#define EDJE_OBJECT_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(edje_object), \
   EFL_OBJECT_OP_FUNC(efl_dbg_info_get, _edje_object_efl_object_dbg_info_get)

#include "edje_object.eo.c"
#include "efl_canvas_layout_calc.eo.c"
#include "efl_canvas_layout_signal.eo.c"
#include "efl_canvas_layout_group.eo.c"
