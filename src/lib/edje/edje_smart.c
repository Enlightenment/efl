#define EFL_CANVAS_GROUP_PROTECTED
#define EFL_CANVAS_GROUP_BETA
#define EFL_PART_PROTECTED

#include "edje_private.h"

#include "canvas/evas_canvas.eo.h"

#ifdef MY_CLASS
# undef MY_CLASS
#endif

#define MY_CLASS             EFL_CANVAS_LAYOUT_CLASS

#define MY_CLASS_NAME        "Edje"
#define MY_CLASS_NAME_LEGACY "edje"

Eina_Inlist *_edje_edjes = NULL;

/************************** API Routines **************************/

EAPI Evas_Object *
edje_object_add(Evas *evas)
{
   evas = evas_find(evas);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(evas, EVAS_CANVAS_CLASS), NULL);
   return efl_add(MY_CLASS, evas, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_efl_canvas_layout_efl_object_constructor(Eo *obj, Edje *ed)
{
   Eo *parent;
   Evas *e;
   void *tmp;

   efl_canvas_group_clipped_set(obj, EINA_TRUE);
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   ed->base.evas = evas_object_evas_get(obj);
   ed->base.clipper = (Evas_Object *) efl_canvas_group_clipper_get(obj);
   ed->duration_scale = 1.0;
   _edje_lib_ref();

   parent = efl_parent_get(obj);
   e = evas_object_evas_get(parent);
   tmp = ecore_evas_ecore_evas_get(e);

   ed->canvas_animator = !!tmp;

   return obj;
}


EOLIAN static void
_efl_canvas_layout_efl_object_invalidate(Eo *obj, Edje *ed)
{
   _edje_file_callbacks_del(ed, NULL);

   efl_invalidate(efl_super(obj, MY_CLASS));

   //invalidate is done, this means the legacy evas deletion event is called.
   for (int i = 0; i < ed->table_parts_size; ++i)
     {
        Edje_Real_Part *rp = ed->table_parts[i];
        switch(rp->type)
          {
            case EDJE_RP_TYPE_SWALLOW:
              _edje_real_part_swallow_clear(ed, rp);
            break;

            case EDJE_RP_TYPE_CONTAINER:
              if (rp->part->type == EDJE_PART_TYPE_BOX)
                _edje_real_part_box_remove_all(ed, rp, 0);
              else if (rp->part->type == EDJE_PART_TYPE_TABLE)
                _edje_real_part_table_clear(ed, rp, 0);
            break;
          }
     }
}

EOLIAN static void
_efl_canvas_layout_efl_object_debug_name_override(Eo *obj, Edje *ed, Eina_Strbuf *sb)
{
   efl_debug_name_override(efl_super(obj, MY_CLASS), sb);
   eina_strbuf_append_printf(sb, ":file='%s':group='%s'",
                             ed->file ? eina_file_filename_get(ed->file->f) : NULL,
                             ed->group);
}

EOLIAN static void
_efl_canvas_layout_efl_object_dbg_info_get(Eo *eo_obj, Edje *_pd EINA_UNUSED, Efl_Dbg_Info *root) EINA_ARG_NONNULL(3)
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
_efl_canvas_layout_efl_canvas_group_group_add(Eo *obj, Edje *ed)
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
_efl_canvas_layout_efl_canvas_group_group_del(Eo *obj, Edje *ed)
{
   _edje_block_violate(ed);
   ed->delete_me = 1;
   _edje_edjes = eina_inlist_remove(_edje_edjes, EINA_INLIST_GET(ed));
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
   _edje_unref(ed);
   _edje_lib_unref();
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_canvas_layout_efl_gfx_entity_position_set(Eo *obj, Edje *ed, Eina_Position2D pos)
{
   unsigned short i;

   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);

   if ((ed->x == pos.x) && (ed->y == pos.y)) return;
   ed->x = pos.x;
   ed->y = pos.y;
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
             if (ep->object)
               evas_object_move(ep->object,
                                ed->x + ep->x + ep->typedata.text->offset.x,
                                ed->y + ep->y + ep->typedata.text->offset.y);
             else if (ep->type != EFL_CANVAS_LAYOUT_PART_TYPE_NONE)
               WRN("No object for part '%s' in group '%s'",
                   ep->part ? ep->part->name : "<invalid>", ed->group);
          }
        else
          {
             if (ep->object)
               evas_object_move(ep->object, ed->x + ep->x, ed->y + ep->y);
             else if (ep->type != EFL_CANVAS_LAYOUT_PART_TYPE_NONE)
               WRN("No object for part '%s' in group '%s'",
                   ep->part ? ep->part->name : "<invalid>", ed->group);
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
_efl_canvas_layout_efl_gfx_entity_size_set(Eo *obj, Edje *ed, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   if ((sz.w == ed->w) && (sz.h == ed->h)) goto super;
   if (ed->collection)
     {
        _edje_limit_get(ed, ed->collection->limits.horizontal, ed->collection->limits.horizontal_count, ed->w, sz.w);
        _edje_limit_get(ed, ed->collection->limits.vertical, ed->collection->limits.vertical_count, ed->h, sz.h);
     }
   ed->w = sz.w;
   ed->h = sz.h;
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
   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
}

static void
_edje_object_show(Eo *obj, Edje *ed)
{
   Eina_List *l;
   Edje *edg;

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), EINA_TRUE);
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

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), EINA_FALSE);
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
_efl_canvas_layout_efl_gfx_entity_visible_set(Eo *obj, Edje *ed, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   if (vis) _edje_object_show(obj, ed);
   else _edje_object_hide(obj, ed);
}

EOLIAN static void
_efl_canvas_layout_efl_canvas_object_no_render_set(Eo *obj, Edje *ed, Eina_Bool enable)
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
_efl_canvas_layout_efl_canvas_group_group_calculate(Eo *obj EINA_UNUSED, Edje *ed)
{
   _edje_recalc_do(ed);
}

EOLIAN static Eina_Bool
_efl_canvas_layout_efl_file_mmap_set(Eo *obj, Edje *pd EINA_UNUSED,
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
_efl_canvas_layout_efl_file_mmap_get(const Eo *obj EINA_UNUSED, Edje *pd,
                               const Eina_File **f, const char **key)
{
   if (f) *f = pd->file ? pd->file->f : NULL;
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
_efl_canvas_layout_efl_canvas_object_paragraph_direction_set(Eo *obj, Edje *ed, Evas_BiDi_Direction dir)
{
   efl_canvas_object_paragraph_direction_set(efl_super(obj, MY_CLASS), dir);

   /* Make it dirty to recalculate edje.
      It needs to move text objects according to new paragraph direction */
   ed->dirty = EINA_TRUE;
   efl_canvas_group_need_recalculate_set(obj, 1);
}

EOLIAN static void
_efl_canvas_layout_efl_observer_update(Eo *obj EINA_UNUSED, Edje *ed, Efl_Object *obs, const char *key, void *data)
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

EOLIAN Eina_Bool
_efl_canvas_layout_efl_player_playable_get(const Eo *obj EINA_UNUSED, Edje *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN void
_efl_canvas_layout_efl_player_play_set(Eo *obj EINA_UNUSED, Edje *ed, Eina_Bool play)
{
   double t;
   Eina_List *l;
   Edje_Running_Program *runp;
   unsigned short i;

   if (!ed) return;
   if (ed->delete_me) return;
   if (play)
     {
        if (!ed->paused) return;
        ed->paused = EINA_FALSE;
        t = ecore_time_get() - ed->paused_at;
        EINA_LIST_FOREACH(ed->actions, l, runp)
          runp->start_time += t;
     }
   else
     {
        if (ed->paused) return;
        ed->paused = EINA_TRUE;
        ed->paused_at = ecore_time_get();
     }

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;
        rp = ed->table_parts[i];
        if ((rp->part->type == EDJE_PART_TYPE_GROUP) &&
            ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
             (rp->typedata.swallow)) &&
            (rp->typedata.swallow->swallowed_object))
          edje_object_play_set(rp->typedata.swallow->swallowed_object, play);
     }
}

EOLIAN Eina_Bool
_efl_canvas_layout_efl_player_play_get(const Eo *obj EINA_UNUSED, Edje *ed)
{
   if (!ed) return EINA_FALSE;
   if (ed->delete_me) return EINA_FALSE;
   if (ed->paused) return EINA_FALSE;

   return EINA_TRUE;
}

EOLIAN void
_efl_canvas_layout_efl_player_play_speed_set(Eo *obj EINA_UNUSED, Edje *pd , double speed)
{
   if (speed <= 0.0) speed = 1.0;
   pd->duration_scale = 1.0/speed;
}

EOLIAN double
_efl_canvas_layout_efl_player_play_speed_get(const Eo *obj EINA_UNUSED, Edje *pd)
{
   return 1.0/pd->duration_scale;
}

/* Internal EO APIs and hidden overrides */

#define EFL_CANVAS_LAYOUT_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_canvas_layout), \
   EFL_OBJECT_OP_FUNC(efl_dbg_info_get, _efl_canvas_layout_efl_object_dbg_info_get)

#include "efl_canvas_layout.eo.c"
#include "edje_global.eo.c"
#include "efl_layout_calc.eo.c"
#include "efl_layout_signal.eo.c"
#include "efl_layout_group.eo.c"
