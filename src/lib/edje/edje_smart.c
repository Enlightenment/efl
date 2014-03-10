#include "edje_private.h"

#include <Eo.h>

EAPI Eo_Op EDJE_OBJ_BASE_ID = EO_NOOP;

#ifdef MY_CLASS
# undef MY_CLASS
#endif

#define MY_CLASS EDJE_OBJ_CLASS

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

static void
_edje_smart_constructor(Eo *obj, void *class_data, va_list *list EINA_UNUSED)
{
   Edje *ed = class_data;
   ed->base = eo_data_ref(obj, EVAS_OBJ_SMART_CLIPPED_CLASS);

   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj, evas_obj_type_set(MY_CLASS_NAME_LEGACY));
   _edje_lib_ref();
}

static void
_edje_smart_destructor(Eo *obj, void *class_data, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
   eo_data_unref(obj, class_data);
}

static void
_dbg_info_get(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eo_Dbg_Info *root = (Eo_Dbg_Info *) va_arg(*list, Eo_Dbg_Info *);
   eo_do_super(eo_obj, MY_CLASS, eo_dbg_info_get(root));
   Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);

   const char *file, *edje_group;
   eo_do(eo_obj, edje_obj_file_get(&file, &edje_group));
   EO_DBG_INFO_APPEND(group, "File", EINA_VALUE_TYPE_STRING, file);
   EO_DBG_INFO_APPEND(group, "Group", EINA_VALUE_TYPE_STRING, edje_group);

   Edje_Load_Error error;
   eo_do(eo_obj, edje_obj_load_error_get(&error));
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
static void
_edje_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Edje *ed = _pd;
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

static void
_edje_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Edje *ed = _pd;

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

static void
_edje_smart_move(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Edje *ed = _pd;
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

static void
_edje_smart_resize(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   Edje *ed = _pd;

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

static void
_edje_smart_show(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Edje *ed = _pd;

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
   _edje_emit(ed, "show", NULL);
}

static void
_edje_smart_hide(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Edje *ed = _pd;

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
   _edje_emit(ed, "hide", NULL);
}

static void
_edje_smart_calculate(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Edje *ed = _pd;
   _edje_recalc_do(ed);
}

static void
_edje_smart_file_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *file = va_arg(*list, const char *);
   const char *group = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_File *f = NULL;
   Eina_Array *nested;

   if (ret) *ret = EINA_FALSE;

   if (file)
     {
        f = eina_file_open(file, EINA_FALSE);
        if (!f) 
          {
             Edje *ed;

             ed = _edje_fetch(obj);
             ed->load_error = EDJE_LOAD_ERROR_DOES_NOT_EXIST;
             return ;
          }
     }
   nested = eina_array_new(8);

   if (_edje_object_file_set_internal(obj, f, group, NULL, NULL, nested))
     if (ret) *ret = EINA_TRUE;

   eina_array_free(nested);
   eina_file_close(f);
   _edje_object_orientation_inform(obj);
}

static void
_edje_smart_mmap_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const Eina_File *f = va_arg(*list, Eina_File *);
   const char *group = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Array *nested;

   if (ret) *ret = EINA_FALSE;

   nested = eina_array_new(8);

   if (_edje_object_file_set_internal(obj, f, group, NULL, NULL, nested))
     if (ret) *ret = EINA_TRUE;

   eina_array_free(nested);
   _edje_object_orientation_inform(obj);
}

static void
_edje_smart_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _edje_smart_constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _edje_smart_destructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DBG_INFO_GET), _dbg_info_get),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _edje_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _edje_smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _edje_smart_move),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW), _edje_smart_show),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_HIDE), _edje_smart_hide),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _edje_smart_resize),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALCULATE), _edje_smart_calculate),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SCALE_SET), _scale_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SCALE_GET), _scale_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MIRRORED_GET), _mirrored_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MIRRORED_SET), _mirrored_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_DATA_GET), _data_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_FREEZE), _freeze),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_THAW), _thaw),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_COLOR_CLASS_SET), _color_class_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_COLOR_CLASS_GET), _color_class_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_CLASS_SET), _text_class_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXISTS), _part_exists),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_OBJECT_GET), _part_object_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_GEOMETRY_GET), _part_geometry_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_ITEM_PROVIDER_SET), _item_provider_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_CHANGE_CB_SET), _text_change_cb_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_PUSH), _part_text_style_user_push),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_POP), _part_text_style_user_pop),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_PEEK), _part_text_style_user_peek),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SET), _part_text_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_GET), _part_text_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ESCAPED_SET), _part_text_escaped_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_UNESCAPED_SET), _part_text_unescaped_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_UNESCAPED_GET), _part_text_unescaped_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECTION_GET), _part_text_selection_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_NONE), _part_text_select_none),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ALL), _part_text_select_all),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INSERT), _part_text_insert),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_APPEND), _part_text_append),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ANCHOR_LIST_GET), _part_text_anchor_list_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ANCHOR_GEOMETRY_GET), _part_text_anchor_geometry_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ITEM_LIST_GET), _part_text_item_list_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ITEM_GEOMETRY_GET), _part_text_item_geometry_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_GEOMETRY_GET), _part_text_cursor_geometry_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_USER_INSERT), _part_text_user_insert),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ALLOW_SET), _part_text_select_allow_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ABORT), _part_text_select_abort),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_BEGIN), _part_text_select_begin),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_EXTEND), _part_text_select_extend),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_IMF_CONTEXT_GET), _part_text_imf_context_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_NEXT), _part_text_cursor_next),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_PREV), _part_text_cursor_prev),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_UP), _part_text_cursor_up),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_DOWN), _part_text_cursor_down),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_BEGIN_SET), _part_text_cursor_begin_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_END_SET), _part_text_cursor_end_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_COPY), _part_text_cursor_copy),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_LINE_BEGIN_SET), _part_text_cursor_line_begin_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_LINE_END_SET), _part_text_cursor_line_end_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_COORD_SET), _part_text_cursor_coord_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_IS_FORMAT_GET), _part_text_cursor_is_format_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_IS_VISIBLE_FORMAT_GET), _part_text_cursor_is_visible_format_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_CONTENT_GET), _part_text_cursor_content_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_POS_SET), _part_text_cursor_pos_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_POS_GET), _part_text_cursor_pos_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_IMF_CONTEXT_RESET), _part_text_imf_context_reset),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_SET), _part_text_input_panel_layout_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_GET), _part_text_input_panel_layout_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_VARIATION_SET), _part_text_input_panel_layout_variation_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_VARIATION_GET), _part_text_input_panel_layout_variation_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_AUTOCAPITAL_TYPE_SET), _part_text_autocapital_type_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_AUTOCAPITAL_TYPE_GET), _part_text_autocapital_type_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_PREDICTION_ALLOW_SET), _part_text_prediction_allow_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_PREDICTION_ALLOW_GET), _part_text_prediction_allow_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_ENABLED_SET), _part_text_input_panel_enabled_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_ENABLED_GET), _part_text_input_panel_enabled_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW), _part_text_input_panel_show),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_HIDE), _part_text_input_panel_hide),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LANGUAGE_SET), _part_text_input_panel_language_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LANGUAGE_GET), _part_text_input_panel_language_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_IMDATA_SET), _part_text_input_panel_imdata_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_IMDATA_GET), _part_text_input_panel_imdata_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_TYPE_SET), _part_text_input_panel_return_key_type_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_TYPE_GET), _part_text_input_panel_return_key_type_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_DISABLED_SET), _part_text_input_panel_return_key_disabled_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_DISABLED_GET), _part_text_input_panel_return_key_disabled_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW_ON_DEMAND_SET), _part_text_input_panel_show_on_demand_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW_ON_DEMAND_GET), _part_text_input_panel_show_on_demand_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_ADD), _text_insert_filter_callback_add),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_DEL), _text_insert_filter_callback_del),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_DEL_FULL), _text_insert_filter_callback_del_full),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_ADD), _text_markup_filter_callback_add),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_DEL), _text_markup_filter_callback_del),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_DEL_FULL), _text_markup_filter_callback_del_full),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_SWALLOW), _part_swallow),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_UNSWALLOW), _part_unswallow),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_SWALLOW_GET), _part_swallow_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIZE_MIN_GET), _size_min_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIZE_MAX_GET), _size_max_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_CALC_FORCE), _calc_force),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIZE_MIN_CALC), _size_min_calc),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PARTS_EXTENDS_CALC), _parts_extends_calc),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIZE_MIN_RESTRICTED_CALC), _size_min_restricted_calc),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_STATE_GET), _part_state_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_DIR_GET), _part_drag_dir_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_VALUE_SET), _part_drag_value_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_VALUE_GET), _part_drag_value_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_SIZE_SET), _part_drag_size_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_SIZE_GET), _part_drag_size_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_STEP_SET), _part_drag_step_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_STEP_GET), _part_drag_step_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_PAGE_SET), _part_drag_page_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_PAGE_GET), _part_drag_page_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_STEP), _part_drag_step),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_PAGE), _part_drag_page),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_APPEND), _part_box_append),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_PREPEND), _part_box_prepend),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_INSERT_BEFORE), _part_box_insert_before),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_INSERT_AT), _part_box_insert_at),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_REMOVE), _part_box_remove),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_REMOVE_AT), _part_box_remove_at),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_REMOVE_ALL), _part_box_remove_all),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_ACCESS_PART_LIST_GET), _access_part_list_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_CHILD_GET), _part_table_child_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_PACK), _part_table_pack),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_UNPACK), _part_table_unpack),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_COL_ROW_SIZE_GET), _part_table_col_row_size_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_CLEAR), _part_table_clear),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PERSPECTIVE_SET), _perspective_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PERSPECTIVE_GET), _perspective_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PRELOAD), _preload),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_UPDATE_HINTS_SET), _update_hints_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_UPDATE_HINTS_GET), _update_hints_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_OBJECT_GET), _part_external_object_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_SET), _part_external_param_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_GET), _part_external_param_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_CONTENT_GET), _part_external_content_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_TYPE_GET), _part_external_param_type_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_FILE_SET), _edje_smart_file_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MMAP_SET), _edje_smart_mmap_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_FILE_GET), _file_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_LOAD_ERROR_GET), _load_error_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MESSAGE_SEND), _message_send),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MESSAGE_HANDLER_SET), _message_handler_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MESSAGE_SIGNAL_PROCESS), _message_signal_process),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_ADD), _signal_callback_add),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_DEL), _signal_callback_del),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIGNAL_EMIT), _signal_emit),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PLAY_SET), _play_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PLAY_GET), _play_get),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_ANIMATION_SET), _animation_set),
        EO_OP_FUNC(EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_ANIMATION_GET), _animation_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_SCALE_SET, "Set the scaling factor for a given Edje object."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_SCALE_GET, "Get a given Edje object's scaling factor."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_MIRRORED_GET, "Get the RTL orientation for this object."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_MIRRORED_SET, "Set the RTL orientation for this object."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_DATA_GET, "Retrive an <b>EDC data field's value</b> from a given Edje"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_FREEZE, "Freezes the Edje object."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_THAW, "Thaws the Edje object."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_COLOR_CLASS_SET, "Sets the object color class."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_COLOR_CLASS_GET, "Gets the object color class."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_TEXT_CLASS_SET, "Sets Edje text class."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_EXISTS, "Check if an Edje part exists in a given Edje object's group"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_OBJECT_GET, "Get a handle to the Evas object implementing a given Edje"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_GEOMETRY_GET, "Retrieve the geometry of a given Edje part, in a given Edje"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_ITEM_PROVIDER_SET, "Set the function that provides item objects for named items in an edje entry text"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_TEXT_CHANGE_CB_SET, "Set the object text callback."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_PUSH, "Set the style of the"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_POP, "Delete the top style form the user style stack."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_PEEK, "Return the text of the object part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_SET, "Sets the text for an object part"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_GET, "Return the text of the object part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_ESCAPED_SET, "Sets the text for an object part, but converts HTML escapes to UTF8"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_UNESCAPED_SET, "Sets the raw (non escaped) text for an object part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_UNESCAPED_GET, "Returns the text of the object part, without escaping."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_SELECTION_GET, "Return the selection text of the object part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_NONE, "Set the selection to be none."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ALL, "Set the selection to be everything."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INSERT, "Insert text for an object part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_APPEND, "Insert text for an object part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_ANCHOR_LIST_GET, "Return a list of char anchor names."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_ANCHOR_GEOMETRY_GET, "Return a list of Evas_Textblock_Rectangle anchor rectangles."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_ITEM_LIST_GET, "Return a list of char item names."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_ITEM_GEOMETRY_GET, "Return item geometry."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_GEOMETRY_GET, "Returns the cursor geometry of the part relative to the edje"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_USER_INSERT, "This function inserts text as if the user has inserted it."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ALLOW_SET, "Enables selection if the entry is an EXPLICIT selection mode"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ABORT, "Aborts any selection action on a part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_BEGIN, "Starts selecting at current cursor position"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_EXTEND, "Extends the current selection to the current cursor position"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_IMF_CONTEXT_GET, "Get the input method context in entry."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_NEXT, "Advances the cursor to the next cursor position."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_PREV, "Moves the cursor to the previous char"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_UP, "Move the cursor to the char above the current cursor position."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_DOWN, "Moves the cursor to the char below the current cursor position."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_BEGIN_SET, "Moves the cursor to the beginning of the text part"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_END_SET, "Moves the cursor to the end of the text part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_COPY, "Copy the cursor to another cursor."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_LINE_BEGIN_SET, "Move the cursor to the beginning of the line."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_LINE_END_SET, "Move the cursor to the end of the line."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_COORD_SET, "Position the given cursor to a X,Y position."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_IS_FORMAT_GET, "Returns whether the cursor points to a format."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_IS_VISIBLE_FORMAT_GET, "Return true if the cursor points to a visible format"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_CONTENT_GET, "Returns the content (char) at the cursor position."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_POS_SET, "Sets the cursor position to the given value"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_POS_GET, "Retrieves the current position of the cursor"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_IMF_CONTEXT_RESET, "Reset the input method context if needed."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_SET, "Set the layout of the input panel."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_GET, "Get the layout of the input panel."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_VARIATION_SET, "Set the variation of the input panel."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_VARIATION_GET, "Get the variation of the input panel."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_AUTOCAPITAL_TYPE_SET, "Set the autocapitalization type on the immodule."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_AUTOCAPITAL_TYPE_GET, "Retrieves the autocapitalization type"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_PREDICTION_ALLOW_SET, "Set whether the prediction is allowed or not."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_PREDICTION_ALLOW_GET, "Get whether the prediction is allowed or not."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_ENABLED_SET, "Sets the attribute to show the input panel automatically."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_ENABLED_GET, "Retrieve the attribute to show the input panel automatically."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW, "Show the input panel (virtual keyboard) based on the input panel property such as layout, autocapital types, and so on."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_HIDE, "Hide the input panel (virtual keyboard)."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LANGUAGE_SET, "Set the language mode of the input panel."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LANGUAGE_GET, "Get the language mode of the input panel."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_IMDATA_SET, "Set the input panel-specific data to deliver to the input panel."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_IMDATA_GET, "Get the specific data of the current active input panel."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_TYPE_SET, "Set the return key type. This type is used to set string or icon on the return key of the input panel."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_TYPE_GET, "Get the return key type."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_DISABLED_SET, "Set the return key on the input panel to be disabled."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_DISABLED_GET, "Get whether the return key on the input panel should be disabled or not."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW_ON_DEMAND_SET, "Set the attribute to show the input panel in case of only an user's explicit Mouse Up event."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW_ON_DEMAND_GET, "Get the attribute to show the input panel in case of only an user's explicit Mouse Up event."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_ADD, "Add a filter function for newly inserted text."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_DEL, "Delete a function from the filter list."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_DEL_FULL, "Delete a function and matching user data from the filter list."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_ADD, "Add a markup filter function for newly inserted text."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_DEL, "Delete a function from the markup filter list."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_DEL_FULL, "Delete a function and matching user data from the markup filter list."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_SWALLOW, "Swallows an object into one of the Edje object @c SWALLOW"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_UNSWALLOW, "Unswallow an object."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_SWALLOW_GET, "Get the object currently swallowed by a part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_SIZE_MIN_GET, "Get the minimum size specified -- as an EDC property -- for a"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_SIZE_MAX_GET, "Get the maximum size specified -- as an EDC property -- for a"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_CALC_FORCE, "Force a Size/Geometry calculation."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_SIZE_MIN_CALC, "Calculate the minimum required size for a given Edje object."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PARTS_EXTENDS_CALC, "Calculate the geometry of the region, relative to a given Edje"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_SIZE_MIN_RESTRICTED_CALC, "Calculate the minimum required size for a given Edje object."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_STATE_GET, "Returns the state of the Edje part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_DRAG_DIR_GET, "Determine dragable directions."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_DRAG_VALUE_SET, "Set the dragable object location."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_DRAG_VALUE_GET, "Get the dragable object location."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_DRAG_SIZE_SET, "Set the dragable object size."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_DRAG_SIZE_GET, "Get the dragable object size."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_DRAG_STEP_SET, "Sets the drag step increment."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_DRAG_STEP_GET, "Gets the drag step increment values."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_DRAG_PAGE_SET, "Sets the page step increments."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_DRAG_PAGE_GET, "Gets the page step increments."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_DRAG_STEP, "Steps the dragable x,y steps."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_DRAG_PAGE, "Pages x,y steps."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_BOX_APPEND, "Appends an object to the box."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_BOX_PREPEND, "Prepends an object to the box."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_BOX_INSERT_BEFORE, "Adds an object to the box."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_BOX_INSERT_AT, "Inserts an object to the box."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_BOX_REMOVE, "Removes an object from the box."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_BOX_REMOVE_AT, "Removes an object from the box."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_BOX_REMOVE_ALL, "Removes all elements from the box."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_ACCESS_PART_LIST_GET, "Retrieve a list all accessibility part names"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TABLE_CHILD_GET, "Retrieve a child from a table"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TABLE_PACK, "Packs an object into the table."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TABLE_UNPACK, "Removes an object from the table."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TABLE_COL_ROW_SIZE_GET, "Gets the number of columns and rows the table has."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_TABLE_CLEAR, "Removes all object from the table."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PERSPECTIVE_SET, "Set the given perspective object on this Edje object."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PERSPECTIVE_GET, "Get the current perspective used on this Edje object."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PRELOAD, "Preload the images on the Edje Object in the background."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_UPDATE_HINTS_SET, "Edje will automatically update the size hints on itself."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_UPDATE_HINTS_GET, "Wether or not Edje will update size hints on itself."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_EXTERNAL_OBJECT_GET, "Get the object created by this external part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_SET, "Set the parameter for the external part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_GET, "Get the parameter for the external part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_EXTERNAL_CONTENT_GET, "Get an object contained in an part of type EXTERNAL"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_TYPE_GET, "Facility to query the type of the given parameter of the given part."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_FILE_SET, "Sets the @b EDJ file (and group within it) to load an Edje"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_MMAP_SET, "Sets the @b EDJ file (and group within it) to load an Edje"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_FILE_GET, "Get the file and group name that a given Edje object is bound to"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_LOAD_ERROR_GET, "Gets the (last) file loading error for a given Edje object"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_MESSAGE_SEND, "Send an (Edje) message to a given Edje object"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_MESSAGE_HANDLER_SET, "Set an Edje message handler function for a given Edje object."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_MESSAGE_SIGNAL_PROCESS, "Process an object's message queue."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_ADD, "Add a callback for an arriving Edje signal, emitted by"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_DEL, "Remove a signal-triggered callback from an object."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_SIGNAL_EMIT, "Send/emit an Edje signal to a given Edje object"),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PLAY_SET, "Set the Edje object to playing or paused states."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_PLAY_GET, "Get the Edje object's state."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_ANIMATION_SET, "Set the object's animation state."),
     EO_OP_DESCRIPTION(EDJE_OBJ_SUB_ID_ANIMATION_GET, "Get the Edje object's animation state."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description edje_smart_class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&EDJE_OBJ_BASE_ID, op_desc, EDJE_OBJ_SUB_ID_LAST),
     NULL,
     sizeof(Edje),
     _edje_smart_class_constructor,
     NULL
};

EO_DEFINE_CLASS(edje_object_class_get, &edje_smart_class_desc, EVAS_OBJ_SMART_CLIPPED_CLASS, NULL);

