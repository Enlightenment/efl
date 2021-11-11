#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_TEXT_PROTECTED
#define EFL_ACCESS_EDITABLE_TEXT_PROTECTED
#define ELM_LAYOUT_PROTECTED
#define EFL_PART_PROTECTED
#define EFL_INPUT_CLICKABLE_PROTECTED

#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

#include "elm_entry_common.h"
#include "elm_widget_entry.h"
#include "efl_ui_textbox_part.eo.h"
#include "elm_part_helper.h"
#include "efl_canvas_textblock_internal.h"

typedef struct _Efl_Ui_Textbox_Data        Efl_Ui_Textbox_Data;
typedef struct _Efl_Ui_Text_Rectangle   Efl_Ui_Text_Rectangle;
typedef struct _Anchor                  Anchor;

/**
 * Base widget smart data extended with entry instance data.
 */
struct _Efl_Ui_Textbox_Data
{
   Evas_Object                          *hit_rect, *entry_edje;

   Eo                                   *popup;
   Eo                                   *popup_list;
   Eo                                   *text_obj;
   Eo                                   *text_guide_obj;
   Eo                                   *text_table;
   Eo                                   *pan;
   Eo                                   *scroller;
   Eo                                   *manager;
   Eo                                   *cursor;
   Eo                                   *cursor_bidi;
   Evas_Object                          *start_handler;
   Evas_Object                          *end_handler;
   Eina_Future                          *deferred_decoration_job;
   /* for deferred appending */
   int                                   append_text_position;
   int                                   append_text_len;
   /* Only for clipboard */
   const char                           *text;
   Evas_Coord                            ent_w, ent_h;
   Evas_Coord                            downx, downy;
   Evas_Coord                            ox, oy;
   Eina_List                            *anchors;
   int                                  gen;
   Eina_List                            *sel;
   Efl_Canvas_Textblock_Factory              *item_factory;
   Efl_Canvas_Textblock_Factory              *item_fallback_factory;
   Mod_Api                              *api; // module api if supplied
   int                                   cursor_pos;
   Elm_Scroller_Policy                   policy_h, policy_v;
   Efl_Text_Cursor_Object                      *sel_handler_cursor;
   struct
     {
        Evas_Object *hover_parent; /**< hover parent object. entry is a hover parent object by default */
        Evas_Object *pop; /**< hidden icon for hover target */
        Evas_Object *hover; /**< hover object */
        const char  *hover_style; /**< style of a hover object */
     } anchor_hover;

   const char                           *cnp_mime_type;
   Elm_Sel_Format                        drop_format;

   struct {
        char                             *text;
        Eina_Bool                        enabled;
   } async;

   struct {
        Eina_Size2D                      scroll;
        Eina_Size2D                      layout;
   } last;
   Efl_Ui_Textbox_Cnp_Content            content;
   Eina_Bool                             sel_handles_enabled : 1;
   Eina_Bool                             start_handler_down : 1;
   Eina_Bool                             start_handler_shown : 1;
   Eina_Bool                             end_handler_down : 1;
   Eina_Bool                             end_handler_shown : 1;
   Eina_Bool                             deferred_decoration_selection : 1;
   Eina_Bool                             deferred_decoration_cursor : 1;
   Eina_Bool                             deferred_decoration_anchor : 1;
   Eina_Bool                             context_menu_enabled : 1;
   Eina_Bool                             long_pressed : 1;
   Eina_Bool                             has_text : 1;
   Eina_Bool                             use_down : 1;
   Eina_Bool                             sel_mode : 1;
   Eina_Bool                             changed : 1;
   Eina_Bool                             scroll : 1;
   Eina_Bool                             text_changed : 1;
   Eina_Bool                             calc_force : 1;
   Eina_Bool                             cursor_update : 1;
   Eina_Bool                             color_is_set : 1;
};

struct _Anchor
{
   Eo                    *obj;
   char                  *name;
   Efl_Text_Attribute_Handle *annotation;
   Eina_List             *rects;
   int                   gen;
   Eina_Bool              item : 1;
   Eina_Bool              updated : 1;
};

#define EFL_UI_TEXT_DATA_GET(o, sd) \
  Efl_Ui_Textbox_Data * sd = efl_data_scope_get(o, EFL_UI_TEXTBOX_CLASS)

#define EFL_UI_TEXT_DATA_GET_OR_RETURN(o, ptr)         \
  EFL_UI_TEXT_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, efl_class_name_get(o));              \
       return;                                       \
    }

#define EFL_UI_TEXT_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_TEXT_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       ERR("No widget data for object %p (%s)",       \
           o, efl_class_name_get(o));               \
       return val;                                    \
    }

#define EFL_UI_TEXT_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), EFL_UI_TEXTBOX_CLASS))) \
    return

struct _Efl_Ui_Text_Rectangle
{
   Evas_Object             *obj_bg, *obj_fg, *obj;
};

#define MY_CLASS EFL_UI_TEXTBOX_CLASS
#define MY_CLASS_PFX efl_ui_textbox
#define MY_CLASS_NAME "Efl.Ui.Textbox"
#define MY_CLASS_NAME_LEGACY "elm_entry"

#include "efl_ui_internal_text_interactive.h"
#include "efl_ui_internal_text_scroller.h"

/* Maximum chunk size to be inserted to the entry at once
 * FIXME: This size is arbitrary, should probably choose a better size.
 * Possibly also find a way to set it to a low value for weak computers,
 * and to a big value for better computers. */
#define EFL_UI_TEXT_CHUNK_SIZE 10000
#define EFL_UI_TEXT_DELAY_WRITE_TIME 2.0

#define ENTRY_PASSWORD_MASK_CHARACTER      0x002A
#define ENTRY_PASSWORD_MASK_CHARACTER_UTF8 "\x2A"

static Eina_List *entries = NULL;

struct _Mod_Api
{
   void (*obj_hook)(Evas_Object *obj);
   void (*obj_unhook)(Evas_Object *obj);
   void (*obj_longpress)(Evas_Object *obj);
};

static const char PART_NAME_HANDLER_START[] = "handler/start";
static const char PART_NAME_HANDLER_END[] = "handler/end";
static const char PART_NAME_CURSOR[] = "cursor";
static const char PART_NAME_SELECTION[] = "selection";
static const char PART_NAME_ANCHOR[] = "anchor";

static void _create_selection_handlers(Evas_Object *obj, Efl_Ui_Textbox_Data *sd);
static void _update_decorations(Eo *obj);
static void _create_text_cursors(Eo *obj, Efl_Ui_Textbox_Data *sd);
static void _efl_ui_textbox_changed_cb(void *data, const Efl_Event *event);
static void _efl_ui_textbox_changed_user_cb(void *data, const Efl_Event *event);
static void _efl_ui_textbox_selection_start_clear_cb(void *data, const Efl_Event *event);
static void _efl_ui_textbox_selection_changed_cb(void *data, const Efl_Event *event);
static void _efl_ui_textbox_cursor_changed_cb(void *data, const Efl_Event *event);
static void _text_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED);
static void _scroller_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED);
static void _text_position_changed_cb(void *data, const Efl_Event *event EINA_UNUSED);
static void _efl_ui_textbox_move_cb(void *data, const Efl_Event *event EINA_UNUSED);
static const char* _efl_ui_textbox_selection_get(const Eo *obj, Efl_Ui_Textbox_Data *sd);
static void _edje_signal_emit(Efl_Ui_Textbox_Data *obj, const char *sig, const char *src);
static void _decoration_defer_all(Eo *obj);
static inline Eo * _decoration_create(Eo *obj, Efl_Ui_Textbox_Data *sd, const char *source, Eina_Bool above);
static void _decoration_defer(Eo *obj);
static void _clear_text_selection(Efl_Ui_Textbox_Data *sd);
static void _anchors_free(Efl_Ui_Textbox_Data *sd);
static void _selection_defer(Eo *obj, Efl_Ui_Textbox_Data *sd);
static Eina_Position2D _decoration_calc_offset(Efl_Ui_Textbox_Data *sd);
static void _update_text_theme(Eo *obj, Efl_Ui_Textbox_Data *sd);
static void _efl_ui_textbox_selection_paste_type(Eo *obj, Efl_Ui_Textbox_Data *sd, Efl_Ui_Cnp_Buffer type);

static Eina_Bool _key_action_copy(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_paste(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_cut(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_menu(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"copy", _key_action_copy},
   {"paste", _key_action_paste},
   {"cut", _key_action_cut},
   {"menu", _key_action_menu},
   {NULL, NULL}
};

static void
_efl_ui_textbox_guide_update(Evas_Object *obj,
                        Eina_Bool has_text)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   if ((has_text) && (!sd->has_text))
     efl_layout_signal_emit(sd->entry_edje, "efl,guide,disabled", "efl");
   else if ((!has_text) && (sd->has_text))
     efl_layout_signal_emit(sd->entry_edje, "efl,guide,enabled", "efl");

   sd->has_text = has_text;
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   efl_ui_mirrored_set(sd->entry_edje, rtl);

   if (sd->anchor_hover.hover)
     efl_ui_mirrored_set(sd->anchor_hover.hover, rtl);
}

static void
_hide_selection_handler(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (!sd->start_handler) return;

   if (sd->start_handler_shown)
     {
        efl_layout_signal_emit(sd->start_handler, "efl,handler,hide", "efl");
        sd->start_handler_shown = EINA_FALSE;
     }
   if (sd->end_handler_shown)
     {
        efl_layout_signal_emit(sd->end_handler, "efl,handler,hide", "efl");
        sd->end_handler_shown = EINA_FALSE;
     }
}

static Eina_Rect
_viewport_region_get(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);
   Eina_Rect rect;
   Evas_Object *parent;

   if (sd->scroll)
     {
        rect = efl_ui_scrollable_viewport_geometry_get(sd->scroller);
     }
   else
     {
        rect = efl_gfx_entity_geometry_get(sd->text_obj);
     }

   parent = efl_ui_widget_parent_get(obj);
   while (parent)
     {
        if (efl_isa(parent, EFL_UI_SCROLLABLE_INTERFACE))
          {
             Eina_Rect r = efl_gfx_entity_geometry_get(parent);
             if (!eina_rectangle_intersection(&rect.rect, &r.rect))
               {
                  rect = EINA_RECT_EMPTY();
                  break;
               }
          }
        parent = efl_ui_widget_parent_get(parent);
     }

   return rect;
}

static void
_update_selection_handler(Eo *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   Evas_Coord sx, sy, sh;
   Evas_Coord ex, ey, eh;

   if (!efl_text_interactive_have_selection_get(obj))
     {
        _hide_selection_handler(obj);
        return;
     }

   if (sd->sel_handles_enabled)
     {
        Eina_Rect rect;
        Eina_Position2D off;
        Evas_Coord hx, hy;
        Eina_Bool hidden = EINA_FALSE;
        Efl_Text_Cursor_Object *sel_start, *sel_end;
        Eina_Rect rc_tmp;

        efl_text_interactive_selection_cursors_get(obj, &sel_start, &sel_end);

        if (!sd->start_handler)
          _create_selection_handlers(obj, sd);

        //evas_object_geometry_get(sd->entry_edje, &ent_x, &ent_y, NULL, NULL);

        rc_tmp = efl_text_cursor_object_cursor_geometry_get(sel_start, EFL_TEXT_CURSOR_TYPE_BEFORE);
        sx = rc_tmp.x;
        sy = rc_tmp.y;
        sh = rc_tmp.h;

        off = _decoration_calc_offset(sd);
        hx = off.x + sx;
        hy = off.y + sy + sh;
        efl_gfx_entity_position_set(sd->start_handler, EINA_POSITION2D(hx, hy));

        rect = _viewport_region_get(obj);

        if (!eina_rectangle_xcoord_inside(&rect.rect, hx) ||
            !eina_rectangle_ycoord_inside(&rect.rect, hy))
          {
             hidden = EINA_TRUE;
          }
        if (!sd->start_handler_shown && !hidden)
          {
             efl_layout_signal_emit(sd->start_handler,
                                     "efl,handler,show", "efl");
             sd->start_handler_shown = EINA_TRUE;
          }
        else if (sd->start_handler_shown && hidden)
          {
             efl_layout_signal_emit(sd->start_handler,
                                     "efl,handler,hide", "efl");
             sd->start_handler_shown = EINA_FALSE;
          }

        hidden = EINA_FALSE;
        rc_tmp = efl_text_cursor_object_cursor_geometry_get(sel_end, EFL_TEXT_CURSOR_TYPE_BEFORE);
        ex = rc_tmp.x;
        ey = rc_tmp.y;
        eh = rc_tmp.h;

        hx = off.x + ex;
        hy = off.y + ey + eh;
        efl_gfx_entity_position_set(sd->end_handler, EINA_POSITION2D(hx, hy));

        if (!eina_rectangle_xcoord_inside(&rect.rect, hx) ||
            !eina_rectangle_ycoord_inside(&rect.rect, hy))
          {
             hidden = EINA_TRUE;
          }
        if (!sd->end_handler_shown && !hidden)
          {
             efl_layout_signal_emit(sd->end_handler,
                                     "efl,handler,show", "efl");
             sd->end_handler_shown = EINA_TRUE;
          }
        else if (sd->end_handler_shown && hidden)
          {
             efl_layout_signal_emit(sd->end_handler,
                                     "efl,handler,hide", "efl");
             sd->end_handler_shown = EINA_FALSE;
          }
     }
   else
     {
        if (sd->start_handler_shown)
          {
             efl_layout_signal_emit(sd->start_handler,
                                     "efl,handler,hide", "efl");
             sd->start_handler_shown = EINA_FALSE;
          }
        if (sd->end_handler_shown)
          {
             efl_layout_signal_emit(sd->end_handler,
                                     "efl,handler,hide", "efl");
             sd->end_handler_shown = EINA_FALSE;
          }
     }
}

static Eina_Value
_selection_data_cb(Efl_Ui_Textbox *obj, void *data EINA_UNUSED, const Eina_Value value)
{
   Eina_Content *content;
   Eina_Slice slice;
   Efl_Text_Cursor_Object *cur, *start, *end;
   Efl_Text_Change_Info info = { NULL, 0, 0, 0, 0 };

   if (eina_value_type_get(&value) != EINA_VALUE_TYPE_CONTENT)
     return EINA_VALUE_EMPTY;

   content = eina_value_to_content(&value);
   slice = eina_content_data_get(content);
   efl_text_interactive_selection_cursors_get(obj, &start, &end);
   if (!efl_text_cursor_object_equal(start, end))
     {
        efl_text_cursor_object_range_delete(start, end);
        efl_text_interactive_all_unselect(obj);
     }
   cur = efl_text_interactive_main_cursor_get(obj);
   info.type = EFL_TEXT_CHANGE_TYPE_INSERT;
   info.position = efl_text_cursor_object_position_get(cur);
   info.length = slice.len;
   info.content = slice.mem;
   if (eina_streq(eina_content_type_get(content), "application/x-elementary-markup"))
     {
        efl_text_cursor_object_markup_insert(cur, slice.mem);
     }
   else if (!strncmp(eina_content_type_get(content), "image/", strlen("image/")))
     {
        Eina_Strbuf *result = eina_strbuf_new();
        eina_strbuf_append_printf(result, "<item absize=240x180 href=");
        eina_strbuf_append_slice(result, slice);
        eina_strbuf_append_printf(result, "></item>");
        efl_text_cursor_object_markup_insert(cur, eina_strbuf_string_get(result));
        eina_strbuf_free(result);
     }
   else // TEXT
     {
        efl_text_cursor_object_text_insert(cur, slice.mem);
     }
   efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_CHANGED_USER, &info);

   return EINA_VALUE_EMPTY;
}

static Eina_Array*
_figure_out_types(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   Eina_Array *types = eina_array_new(10);

   if (sd->content & EFL_UI_TEXTBOX_CNP_CONTENT_MARKUP)
     eina_array_push(types, "application/x-elementary-markup");
   if (sd->content & EFL_UI_TEXTBOX_CNP_CONTENT_IMAGE)
     {
        eina_array_push(types, "image/png");
        eina_array_push(types, "image/jpeg");
        eina_array_push(types, "image/x-ms-bmp");
        eina_array_push(types, "image/gif");
        eina_array_push(types, "image/tiff");
        eina_array_push(types, "image/svg+xml");
        eina_array_push(types, "image/x-xpixmap");
        eina_array_push(types, "image/x-tga");
        eina_array_push(types, "image/x-portable-pixmap");
     }
   if (sd->content & EFL_UI_TEXTBOX_CNP_CONTENT_TEXT)
     eina_array_push(types, "text/plain;charset=utf-8");
   return types;
}

static Eina_Bool
_accepting_drops(Eo *obj, Efl_Ui_Textbox_Data *sd, Eina_Accessor *mime_types)
{
   int i = 0;
   const char *mime_type;

   if (efl_ui_widget_disabled_get(obj)) return EINA_FALSE;

   EINA_ACCESSOR_FOREACH(mime_types, i, mime_type)
     {
        if (sd->content & EFL_UI_TEXTBOX_CNP_CONTENT_TEXT &&
            eina_streq(mime_type, "text/plain;charset=utf-8"))
          return EINA_TRUE;

        if (sd->content & EFL_UI_TEXTBOX_CNP_CONTENT_IMAGE &&
            strncmp(mime_type, "image/", strlen("image/")))
          return EINA_TRUE;

        if (sd->content & EFL_UI_TEXTBOX_CNP_CONTENT_MARKUP &&
            eina_streq(mime_type, "application/x-elementary-markup"))
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_dnd_enter_cb(void *data EINA_UNUSED,
              const Efl_Event *ev)
{
   Efl_Ui_Drop_Event *dnd_enter = ev->info;
   EFL_UI_TEXT_DATA_GET(ev->object, sd);
   if (_accepting_drops(ev->object, sd, dnd_enter->available_types))
     efl_ui_focus_util_focus(ev->object);
}

static void
_dnd_pos_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Drop_Event *dnd_pos = ev->info;
   Eina_Position2D po, pe, pos;
   EFL_UI_TEXT_DATA_GET(ev->object, sd);
   int cursor_pos;

   if (!_accepting_drops(ev->object, sd, dnd_pos->available_types))
     return;

   po = efl_gfx_entity_position_get(ev->object);
   pe = efl_gfx_entity_position_get(sd->entry_edje);
   pos.x = dnd_pos->position.x + po.x - pe.x;
   pos.y = dnd_pos->position.y + po.x - pe.y;

   edje_object_part_text_cursor_coord_set
      (sd->entry_edje, "efl.text", EDJE_CURSOR_USER, pos.x, pos.y);
   cursor_pos = edje_object_part_text_cursor_pos_get
      (sd->entry_edje, "efl.text", EDJE_CURSOR_USER);
   edje_object_part_text_cursor_pos_set(sd->entry_edje, "efl.text",
                                        EDJE_CURSOR_MAIN, cursor_pos);
}

static void
_dnd_drop_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Drop_Event *drop = ev->info;

   Eina_Array *types;

   EFL_UI_TEXT_DATA_GET(ev->object, sd);
   types = _figure_out_types(ev->object, sd);

   if (_accepting_drops(ev->object, sd, drop->available_types))
     {
        Eina_Future *future;

        future = efl_ui_dnd_drop_data_get(ev->object, evas_device_seat_id_get(evas_default_device_get(evas_object_evas_get(ev->object), EVAS_DEVICE_CLASS_SEAT)), eina_array_iterator_new(types));
        efl_future_then(ev->object, future, _selection_data_cb);
     }

   eina_array_free(types);
}

/* we can't reuse layout's here, because it's on entry_edje only */
EOLIAN static void
_efl_ui_textbox_efl_ui_widget_disabled_set(Eo *obj, Efl_Ui_Textbox_Data *sd, Eina_Bool disabled)
{
   const char *emission;

   if (efl_ui_widget_disabled_get(obj) == disabled)
     return;

   efl_ui_widget_disabled_set(efl_super(obj, MY_CLASS), disabled);

   emission = efl_ui_widget_disabled_get(obj) ? "efl,state,disabled" : "efl,state,enabled";
   efl_layout_signal_emit(sd->entry_edje, emission, "efl");
   if (sd->scroll)
     {
        efl_ui_scrollable_scroll_freeze_set(obj, efl_ui_widget_disabled_get(obj));
     }

   _update_text_theme(obj, sd);
}

/* we can't issue the layout's theming code here, cause it assumes an
 * unique edje object, always */
EOLIAN static Eina_Error
_efl_ui_textbox_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   Eina_Error theme_apply;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   theme_apply = efl_ui_widget_theme_apply(efl_super(obj,MY_CLASS));
   if (theme_apply == EFL_UI_THEME_APPLY_ERROR_GENERIC) return EFL_UI_THEME_APPLY_ERROR_GENERIC;

   _update_text_theme(obj, sd);

   efl_event_freeze(obj);

   edje_object_mirrored_set
     (wd->resize_obj, efl_ui_mirrored_get(obj));

   efl_gfx_entity_scale_set
     (wd->resize_obj,
     efl_gfx_entity_scale_get(obj) * elm_config_scale_get());

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   elm_widget_element_update(obj, sd->entry_edje,
                                   elm_widget_theme_element_get(obj));

   // elm_entry_cursor_pos_set -> cursor,changed -> widget_show_region_set
   // -> smart_objects_calculate will call all smart calculate functions,
   // and one of them can delete elm_entry.
   efl_ref(obj);

   if (efl_ui_focus_object_focus_get(obj))
     {
        efl_layout_signal_emit(sd->entry_edje, "efl,action,focus", "efl");
     }

   efl_layout_signal_process(sd->entry_edje, EINA_FALSE);

   Evas_Object* clip = efl_canvas_object_clipper_get(sd->entry_edje);
   efl_canvas_object_clipper_set(sd->hit_rect, clip);

   if (sd->start_handler)
     {
        elm_widget_element_update(obj, sd->start_handler, PART_NAME_HANDLER_START);
        elm_widget_element_update(obj, sd->end_handler, PART_NAME_HANDLER_END);
     }

   sd->has_text = !sd->has_text;
   _efl_ui_textbox_guide_update(obj, !sd->has_text);
   efl_event_thaw(obj);

   efl_event_callback_call(obj, EFL_UI_LAYOUT_EVENT_THEME_CHANGED, NULL);

   efl_unref(obj);

   if (efl_content_get(efl_part(sd->entry_edje, "efl.text")) == NULL && !sd->scroller)
     {
        efl_pack_table(sd->text_table, sd->text_obj, 0, 0, 1, 1);
        efl_pack_table(sd->text_table, sd->text_guide_obj, 0, 0, 1, 1);
        efl_content_set(efl_part(sd->entry_edje, "efl.text"), sd->text_table);
     }

   if (!sd->cursor && !sd->cursor_bidi)
     _create_text_cursors(obj, sd);

   return theme_apply;
}

static void
_cursor_geometry_recalc(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   Evas_Coord cx, cy, cw, ch;
   Eina_Rect rc;

   if (!efl_text_interactive_editable_get(obj)) return;

   cx = cy = cw = ch = 0;

   Efl_Text_Cursor_Object *main_cur =
      efl_text_interactive_main_cursor_get(obj);

   rc = efl_text_cursor_object_cursor_geometry_get(main_cur, EFL_TEXT_CURSOR_TYPE_BEFORE);
   cx = rc.x;
   cy = rc.y;
   cw = rc.w;
   ch = rc.h;

   edje_object_size_min_restricted_calc(sd->cursor, &cw, NULL, cw, 0);
   if (cw < 1) cw = 1;
   if (ch < 1) ch = 1;
   edje_object_size_min_restricted_calc(sd->cursor, &cw, NULL, cw, 0);

   efl_ui_scrollable_scroll(sd->scroller, EINA_RECT(cx, cy, cw, ch), EINA_FALSE);

}

#define SIZE2D_EQ(X, Y) (((X).w == (Y).w) && ((X).h == (Y).h))

EOLIAN static void
_efl_ui_textbox_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   Eina_Size2D min = EINA_SIZE2D(0, 0);
   Eina_Size2D edmin = EINA_SIZE2D(0, 0);
   Eina_Size2D sz = EINA_SIZE2D(0, 0);

   efl_canvas_group_need_recalculate_set(obj, EINA_FALSE);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   sz = efl_gfx_entity_size_get(obj);
   if (!sd->calc_force && SIZE2D_EQ(sd->last.layout, sz) &&
         !sd->text_changed)
     {
        return;
     }

   sd->calc_force = EINA_FALSE;
   sd->last.layout.w = sz.w;
   sd->last.layout.h = sz.h;
   sd->text_changed = EINA_FALSE;

   if (sd->scroll)
     {
        efl_canvas_group_calculate(sd->scroller);
        min = efl_gfx_hint_size_min_get(sd->scroller);
        if (!efl_text_multiline_get(obj))
          {
             edje_object_size_min_calc(wd->resize_obj, &edmin.w, &edmin.h);
             min.w = edmin.w;
             min.h = edmin.h;
             efl_gfx_hint_size_restricted_min_set(obj, min);
          }

        sd->cursor_update = EINA_TRUE;
        _decoration_defer_all(obj);
     }
   else
     {
        Eina_Size2D text_sz = efl_gfx_entity_size_get(sd->text_obj);
        edje_object_size_min_calc(wd->resize_obj, &edmin.w, &edmin.h);
        efl_event_freeze(sd->text_obj);
        efl_gfx_entity_size_set(sd->text_obj, EINA_SIZE2D(sz.w, 0));
        /* ignore current object size for single-line since we always need to know the actual size */
        if (!efl_text_multiline_get(obj))
          min = efl_canvas_textblock_size_native_get(sd->text_obj);
        else
          min = efl_canvas_textblock_size_formatted_get(sd->text_obj);
        efl_gfx_entity_size_set(sd->text_obj, text_sz);
        efl_event_thaw(sd->text_obj);
        min.w += edmin.w;
        min.h += edmin.h;
        efl_gfx_hint_size_restricted_min_set(obj, min);
     }
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_ui_focus_object_on_focus_update(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   Efl_Object *top;

   if (!efl_text_interactive_editable_get(obj)) return EINA_FALSE;

   top = efl_provider_find(obj, EFL_UI_WIN_CLASS);

   if (efl_ui_focus_object_focus_get(obj))
     {
        efl_canvas_object_key_focus_set(sd->text_obj, EINA_TRUE);

        _edje_signal_emit(sd, "efl,action,focus", "efl");

        if (efl_input_text_input_panel_autoshow_get(obj) && !efl_input_text_input_panel_show_on_demand_get(obj))
          elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_ON);
        if (_elm_config->atspi_mode)
          efl_access_state_changed_signal_emit(obj, EFL_ACCESS_STATE_TYPE_FOCUSED, EINA_TRUE);
     }
   else
     {
        Eo *sw = sd->text_obj;

        _edje_signal_emit(sd, "efl,action,unfocus", "efl");
        efl_canvas_object_key_focus_set(sw, EINA_FALSE);

        if (efl_input_text_input_panel_autoshow_get(obj))
          elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_OFF);
        if (_elm_config->atspi_mode)
          efl_access_state_changed_signal_emit(obj, EFL_ACCESS_STATE_TYPE_FOCUSED, EINA_FALSE);

        if (_elm_config->selection_clear_enable)
          {
             if ((efl_text_interactive_have_selection_get(obj)) && (!sd->popup))
               {
                  sd->sel_mode = EINA_FALSE;
                  efl_ui_widget_scroll_hold_pop(obj);
                  edje_object_part_text_select_allow_set(sd->entry_edje, "efl.text", EINA_FALSE);
                  efl_layout_signal_emit(sd->entry_edje, "efl,state,select,off", "efl");
                  edje_object_part_text_select_none(sd->entry_edje, "efl.text");
               }
          }
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Rect
_efl_ui_textbox_efl_ui_widget_interest_region_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   Eina_Rect edje, elm;
   Eina_Rect r = {};

   r = efl_text_cursor_object_cursor_geometry_get(
         efl_text_interactive_main_cursor_get(obj), EFL_TEXT_CURSOR_TYPE_BEFORE);

   if (!efl_text_multiline_get(obj))
     {
        Eina_Rect rr = efl_gfx_entity_geometry_get(sd->entry_edje);
        r.h = rr.h;
        r.y = 0;
     }

   edje = efl_gfx_entity_geometry_get(sd->entry_edje);
   elm = efl_gfx_entity_geometry_get(obj);

   r.x += edje.x - elm.x;
   r.y += edje.y - elm.y;
   if (r.w < 1) r.w = 1;
   if (r.h < 1) r.h = 1;

   return r;
}

static void
_popup_position(Evas_Object *obj)
{
   Eina_Rect r;
   Evas_Coord cx, cy, cw, ch;
   Eina_Size2D m;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   cx = cy = 0;
   cw = ch = 1;
   r = efl_gfx_entity_geometry_get(sd->entry_edje);
   if (sd->use_down)
     {
        cx = sd->downx - r.x;
        cy = sd->downy - r.y;
        cw = 1;
        ch = 1;
     }
   else
     edje_object_part_text_cursor_geometry_get
       (sd->entry_edje, "efl.text", &cx, &cy, &cw, &ch);

   m = efl_gfx_hint_size_restricted_min_get(sd->popup);
   if (cx + m.w > r.w)
     cx = r.w - m.w;
   if (cy + m.h > r.h)
     cy = r.h - m.h;
   efl_gfx_entity_geometry_set(sd->popup, EINA_RECT(r.x + cx, r.y + cy, m.w, m.h));
}

static void
_selection_lost_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Ui_Wm_Selection_Changed *changed = ev->info;
   EFL_UI_TEXT_DATA_GET(ev->object, sd);

   if (changed->buffer == EFL_UI_CNP_BUFFER_SELECTION && changed->caused_by != ev->object)
     {
        efl_text_interactive_all_unselect(ev->object);
        _selection_defer(ev->object, sd);
     }
}

static void
_selection_store(Efl_Ui_Cnp_Buffer buffer,
                 Evas_Object *obj)
{
   char *sel;
   Efl_Text_Cursor_Object *start, *end;
   Eina_Content *content;

   efl_text_interactive_selection_cursors_get(obj, &start, &end);
   sel = efl_text_cursor_object_range_markup_get(start, end);

   if ((!sel) || (!sel[0])) return;  /* avoid deleting our own selection */

   content = eina_content_new((Eina_Slice)EINA_SLICE_STR_FULL(sel), "application/x-elementary-markup");

   efl_ui_selection_set(obj, buffer, content, evas_device_seat_id_get(evas_default_device_get(evas_object_evas_get(obj), EVAS_DEVICE_CLASS_SEAT)));

   free(sel);
}

static void
_popup_dismiss( Efl_Ui_Textbox_Data *sd)
{
   efl_del(sd->popup_list);
   efl_del(sd->popup);
   sd->popup = NULL;
   sd->popup_list = NULL;
}

static void
_backwall_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);
   _popup_dismiss(sd);
}

static void
_popup_item_cut_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);
   efl_ui_textbox_selection_cut(data);
   _popup_dismiss(sd);
}

static void
_popup_item_copy_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);
   efl_ui_textbox_selection_copy(data);
   _popup_dismiss(sd);
}

static void
_popup_item_cancel_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   if (!efl_text_interactive_selection_allowed_get(data)) return;

   if (!_elm_config->desktop_entry)
     efl_ui_widget_scroll_hold_pop(data);

   sd->sel_mode = EINA_FALSE;
   efl_text_interactive_all_unselect(data);
   _popup_dismiss(sd);
}

static void
_popup_item_paste_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);
   efl_ui_textbox_selection_paste(data);
   _popup_dismiss(sd);
}

static void
_menu_call(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (sd->anchor_hover.hover) return;

   efl_event_callback_call(obj, EFL_UI_TEXTBOX_EVENT_CONTEXT_OPEN, NULL);

   if ((sd->api) && (sd->api->obj_longpress))
     {
        sd->api->obj_longpress(obj);
     }
   else if (sd->context_menu_enabled)
     {
        Eina_Bool ownersel;

        ownersel = elm_cnp_clipboard_selection_has_owner(obj);
        /* prevent stupid blank hoversel */
        if (efl_text_interactive_have_selection_get(obj) && efl_text_password_get(obj)) return;
        if (_elm_config->desktop_entry && (!efl_text_interactive_have_selection_get(obj)) && ((!efl_text_interactive_editable_get(obj)) || (!ownersel)))
        return;
        if (sd->popup) _popup_dismiss(sd);
        else efl_ui_widget_scroll_freeze_push(obj);

        sd->popup = efl_add(EFL_UI_POPUP_CLASS, obj);

        sd->popup_list = efl_add(EFL_UI_LIST_CLASS, sd->popup);
        efl_content_set(sd->popup, sd->popup_list);
        efl_gfx_hint_align_set(sd->popup_list, 1.0, 1.0);
        efl_gfx_hint_weight_set(sd->popup_list, 1.0, 1.0);

        efl_ui_widget_sub_object_add(obj, sd->popup);
        efl_ui_popup_anchor_set(sd->popup, obj);
        efl_event_callback_add(sd->popup, EFL_UI_POPUP_EVENT_BACKWALL_CLICKED, _backwall_clicked, obj);

        if (efl_text_interactive_have_selection_get(obj))
          {
             if (!efl_text_password_get(obj))
               {
                  if (efl_text_interactive_editable_get(obj))
                    {
                       Eo *il = NULL;
                       il = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, sd->popup_list);
                       efl_text_set(il, E_("Cut"));
                       efl_gfx_hint_align_set(il, 1.0, 1.0);
                       efl_gfx_hint_weight_set(sd->popup_list, 1.0, 1.0);
                       efl_pack_end(sd->popup_list, il);
                       efl_event_callback_add(il, EFL_UI_EVENT_SELECTED_CHANGED, _popup_item_cut_cb, obj);

                       il = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, sd->popup_list);
                       efl_text_set(il, E_("Copy"));
                       efl_gfx_hint_align_set(il, 1.0, 1.0);
                       efl_gfx_hint_weight_set(sd->popup_list, 1.0, 1.0);
                       efl_pack_end(sd->popup_list, il);
                       efl_event_callback_add(il, EFL_UI_EVENT_SELECTED_CHANGED, _popup_item_copy_cb, obj);

                       il = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, sd->popup_list);
                       efl_text_set(il, E_("Paste"));
                       efl_gfx_hint_align_set(il, 1.0, 1.0);
                       efl_gfx_hint_weight_set(sd->popup_list, 1.0, 1.0);
                       efl_pack_end(sd->popup_list, il);
                       efl_event_callback_add(il, EFL_UI_EVENT_SELECTED_CHANGED, _popup_item_paste_cb, obj);

                       il = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, sd->popup_list);
                       efl_text_set(il, E_("Cancel"));
                       efl_gfx_hint_align_set(il, 1.0, 1.0);
                       efl_gfx_hint_weight_set(sd->popup_list, 1.0, 1.0);
                       efl_pack_end(sd->popup_list, il);
                       efl_event_callback_add(il, EFL_UI_EVENT_SELECTED_CHANGED, _popup_item_cancel_cb, obj);
                    }
               }
          }
        else
          {
             if (!sd->sel_mode)
               {
                  if (ownersel)
                    {
                       if (efl_text_interactive_editable_get(obj))
                         {
                            Eo *il = NULL;
                            il = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, sd->popup_list);
                            efl_text_set(il, E_("Paste"));
                            efl_gfx_hint_align_set(il, 1.0, 1.0);
                            efl_gfx_hint_weight_set(sd->popup_list, 1.0, 1.0);
                            efl_pack_end(sd->popup_list, il);
                            efl_event_callback_add(il, EFL_UI_EVENT_SELECTED_CHANGED, _popup_item_paste_cb, obj);
                         }
                    }
               }
             else
               {
                  Eo *il = NULL;
                  il = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, sd->popup_list);
                  efl_text_set(il, E_("Cancel"));
                  efl_gfx_hint_align_set(il, 1.0, 1.0);
                  efl_gfx_hint_weight_set(sd->popup_list, 1.0, 1.0);
                  efl_pack_end(sd->popup_list, il);
                  efl_event_callback_add(il, EFL_UI_EVENT_SELECTED_CHANGED, _popup_item_cancel_cb, obj);
               }
          }

        if (sd->popup)
          {
             _popup_position(obj);
          }
     }
}

static Eina_Bool
_is_pointer_inside_viewport(Eo *textbox,Efl_Ui_Textbox_Data *sd)
{
   if (sd->scroller)
     {
        Eo *top = efl_provider_find(textbox, EFL_UI_WIN_CLASS);
        Eina_Position2D pos = {0};
        if (efl_canvas_scene_pointer_position_get(top, NULL, &pos))
          {
             Eina_Rect rc = efl_ui_scrollable_viewport_geometry_get(sd->scroller);
             if (!eina_rectangle_coords_inside(&rc.rect, pos.x, pos.y))
               return EINA_FALSE;
          }
     }
   return EINA_TRUE;
}

static void
_long_press_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   if (!_is_pointer_inside_viewport(data, sd))
     return;

   /* Context menu will not appear if context menu disabled is set
    * as false on a long press callback */
   if (!_elm_config->context_menu_disabled &&
            (!_elm_config->desktop_entry))
     _menu_call(data);

   sd->long_pressed = EINA_TRUE;
}


static Eina_Bool
_key_action_copy(Evas_Object *obj, const char *params EINA_UNUSED)
{
   efl_ui_textbox_selection_copy(obj);
   return EINA_TRUE;
}

static Eina_Bool
_key_action_cut(Evas_Object *obj, const char *params EINA_UNUSED)
{
   efl_ui_textbox_selection_cut(obj);
   return EINA_TRUE;
}

static Eina_Bool
_key_action_paste(Evas_Object *obj, const char *params EINA_UNUSED)
{
   efl_ui_textbox_selection_paste(obj);
   return EINA_TRUE;
}

static Eina_Bool
_key_action_menu(Evas_Object *obj, const char *params EINA_UNUSED)
{
   Eina_Bool b_ret = EINA_FALSE;
   if (!_elm_config->context_menu_disabled)
     {
        _menu_call(obj);
        b_ret = EINA_TRUE;
     }
   return b_ret;
}

static void
_mouse_down_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Pointer_Data *ev;
   ev = efl_data_scope_get(event->info, EFL_INPUT_POINTER_CLASS);
   EFL_UI_TEXT_DATA_GET(data, sd);

   if (efl_ui_widget_disabled_get(data)) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   sd->downx = ev->cur.x;
   sd->downy = ev->cur.y;
   sd->long_pressed = EINA_FALSE;


   if (ev->button == 2)
     {
        _efl_ui_textbox_selection_paste_type(data, sd, EFL_UI_CNP_BUFFER_SELECTION);
     }

    /* If right button is pressed and context menu disabled is true,
     * then only context menu will appear */
   if (ev->button == 3 && (!_elm_config->context_menu_disabled))
     {
        if (_elm_config->desktop_entry)
          {
             if (!_is_pointer_inside_viewport(data, sd))
               return;
             sd->use_down = 1;
             _menu_call(data);
          }
     }
}

static void
_mouse_up_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Pointer_Data *ev;
   ev = efl_data_scope_get(event->info, EFL_INPUT_POINTER_CLASS);
   Efl_Object *top;

   EFL_UI_TEXT_DATA_GET(data, sd);

   if (efl_ui_widget_disabled_get(data)) return;
   if (ev->button == 1)
     {
        efl_input_clickable_longpress_abort(data, 1);

        /* Since context menu disabled flag was checked at long press start while mouse
         * down, hence the same should be checked at mouse up from a long press
         * as well */
        if (sd->long_pressed)
          {
             if (!_elm_config->context_menu_disabled)
               {
                  _menu_call(data);
               }
          }
        else
          {
             top = efl_provider_find(data, EFL_UI_WIN_CLASS);
             if (efl_input_text_input_panel_autoshow_get(data) && efl_input_text_input_panel_show_on_demand_get(data))
               elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_ON);
          }
     }
  /* Since context menu disabled flag was checked at mouse right key down,
   * hence the same should be stopped at mouse up of right key as well */
   else if ((ev->button == 3) && (!_elm_config->context_menu_disabled) &&
            (!_elm_config->desktop_entry))
     {
         sd->use_down = 1;
         _menu_call(data);
     }
}

static void
_mouse_move_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Pointer_Data *ev;
   ev = efl_data_scope_get(event->info, EFL_INPUT_POINTER_CLASS);
   Evas_Coord dx, dy;

   EFL_UI_TEXT_DATA_GET(data, sd);

   if (efl_ui_widget_disabled_get(data)) return;
   if (ev->pressed_buttons == 1)
     {
        if (sd->long_pressed)
          {
             Eina_Rect r;
             Eina_Bool rv;

             r = efl_gfx_entity_geometry_get(sd->entry_edje);
             rv = edje_object_part_text_cursor_coord_set
               (sd->entry_edje, "efl.text", EDJE_CURSOR_USER,
               ev->cur.x - r.x, ev->cur.y - r.y);
             if (rv)
               {
                  edje_object_part_text_cursor_copy
                    (sd->entry_edje, "efl.text", EDJE_CURSOR_USER, EDJE_CURSOR_MAIN);
               }
             else
               WRN("Warning: Cannot move cursor");

          }
     }
   if (!sd->sel_mode)
     {
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
          {
             efl_input_clickable_longpress_abort(data, 1);
          }
     }

   dx = sd->downx - ev->cur.x;
   dx *= dx;
   dy = sd->downy - ev->cur.y;
   dy *= dy;
   if ((dx + dy) > ((_elm_config->finger_size / 2) *
                    (_elm_config->finger_size / 2)))
     {
        efl_input_clickable_longpress_abort(data, 1);
     }
}

static Evas_Object *
_item_get(void *data, const char *item)
{
   Evas_Object *o = NULL;

   EFL_UI_TEXT_DATA_GET(data, sd);

   if (item)
     {
        if (sd->item_factory)
          {
             o = efl_canvas_textblock_factory_create(sd->item_factory, data, item);
          }
        else if (sd->item_fallback_factory)
          {
             o = efl_canvas_textblock_factory_create(sd->item_fallback_factory,
                   data, item);
          }
     }
   return o;
}

static void
_selection_handlers_offset_calc(Evas_Object *obj, Evas_Object *handler)
{
   Evas_Coord cx, cy, cw, ch;
   Evas_Coord hh;
   Eina_Position2D pos;
   Eina_Rect rc;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   pos = efl_gfx_entity_position_get(sd->text_obj);
   rc = efl_text_cursor_object_cursor_geometry_get(
         efl_text_interactive_main_cursor_get(obj), EFL_TEXT_CURSOR_TYPE_BEFORE);
   cx = rc.x;
   cy = rc.y;
   cw = rc.w;
   ch = rc.h;

   edje_object_size_min_calc(handler, NULL, &hh);

   sd->ox = pos.x + cx + (cw / 2);
   if (ch > hh)
     {
        sd->oy = pos.y + cy + ch;
     }
   else
     {
        sd->oy = pos.y + cy + (ch / 2);
     }

   efl_input_clickable_longpress_abort(obj, 1);
   sd->long_pressed = EINA_FALSE;
}

static void
_start_handler_mouse_down_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   int start_pos, end_pos, pos;
   Efl_Text_Cursor_Object *sel_start, *sel_end;
   Efl_Text_Cursor_Object *main_cur;

   Eo *text_obj = sd->text_obj;

   sd->start_handler_down = EINA_TRUE;

   /* Get the cursors */
   efl_text_interactive_selection_cursors_get(text_obj, &sel_start, &sel_end);
   main_cur = efl_text_interactive_main_cursor_get(text_obj);

   start_pos = efl_text_cursor_object_position_get(sel_start);
   end_pos = efl_text_cursor_object_position_get(sel_end);

   if (start_pos <= end_pos)
     {
        pos = start_pos;
        sd->sel_handler_cursor = sel_start;
     }
   else
     {
        pos = end_pos;
        sd->sel_handler_cursor = sel_end;
     }
   efl_text_cursor_object_position_set(main_cur, pos);
   _selection_handlers_offset_calc(data, sd->start_handler);
}

static void
_start_handler_mouse_up_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   sd->start_handler_down = EINA_FALSE;
   /* Context menu should not appear, even in case of selector mode, if the
    * flag is false (disabled) */
   if ((!_elm_config->context_menu_disabled) &&
       (!_elm_config->desktop_entry) && (sd->long_pressed))
     _menu_call(data);
}

static void
_start_handler_mouse_move_cb(void *data, const Efl_Event *event)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   if (!sd->start_handler_down) return;
   Efl_Input_Pointer_Data *ev;
   ev = efl_data_scope_get(event->info, EFL_INPUT_POINTER_CLASS);
   Eina_Rect re;
   Evas_Coord cx, cy;
   int pos;

   re = efl_gfx_entity_geometry_get(sd->entry_edje);
   cx = ev->cur.x - sd->ox - re.x;
   cy = ev->cur.y - sd->oy - re.y;
   if (cx <= 0) cx = 1;

   efl_text_cursor_object_char_coord_set(sd->sel_handler_cursor, EINA_POSITION2D(cx, cy));
   pos = efl_text_cursor_object_position_get(sd->sel_handler_cursor);

   /* Set the main cursor. */
   efl_text_cursor_object_position_set(
         efl_text_interactive_main_cursor_get(sd->text_obj), pos);

   efl_input_clickable_longpress_abort(data, 1);
   sd->long_pressed = EINA_FALSE;
}

static void
_end_handler_mouse_down_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   Efl_Text_Cursor_Object *sel_start, *sel_end;
   Efl_Text_Cursor_Object *main_cur;
   int pos, start_pos, end_pos;

   sd->end_handler_down = EINA_TRUE;

   Eo *text_obj = sd->text_obj;

   efl_text_interactive_selection_cursors_get(text_obj, &sel_start, &sel_end);
   main_cur = efl_text_interactive_main_cursor_get(text_obj);

   start_pos = efl_text_cursor_object_position_get(sel_start);
   end_pos = efl_text_cursor_object_position_get(sel_end);

   if (start_pos < end_pos)
     {
        pos = end_pos;
        sd->sel_handler_cursor = sel_end;
     }
   else
     {
        pos = start_pos;
        sd->sel_handler_cursor = sel_start;
     }

   efl_text_cursor_object_position_set(main_cur, pos);
   _selection_handlers_offset_calc(data, sd->end_handler);
}

static void
_end_handler_mouse_up_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   sd->end_handler_down = EINA_FALSE;
   /* Context menu appear was checked in case of selector start, and hence
    * the same should be checked at selector end as well */
   if ((!_elm_config->context_menu_disabled) &&
       (!_elm_config->desktop_entry) && (sd->long_pressed))
     _menu_call(data);
}

static void
_end_handler_mouse_move_cb(void *data, const Efl_Event *event)
{
   EFL_UI_TEXT_DATA_GET(data, sd);
   Efl_Input_Pointer_Data *ev;
   ev = efl_data_scope_get(event->info, EFL_INPUT_POINTER_CLASS);

   if (!sd->end_handler_down) return;
   Eina_Rect re;
   Evas_Coord cx, cy;
   int pos;

   re = efl_gfx_entity_geometry_get(sd->entry_edje);
   cx = ev->cur.x - sd->ox - re.x;
   cy = ev->cur.y - sd->oy - re.y;
   if (cx <= 0) cx = 1;

   efl_text_cursor_object_char_coord_set(sd->sel_handler_cursor, EINA_POSITION2D(cx, cy));
   pos = efl_text_cursor_object_position_get(sd->sel_handler_cursor);
   /* Set the main cursor. */
   efl_text_cursor_object_position_set(efl_text_interactive_main_cursor_get(data), pos);
   efl_input_clickable_longpress_abort(data, 1);
   sd->long_pressed = EINA_FALSE;
}

static void
_create_selection_handlers(Evas_Object *obj, Efl_Ui_Textbox_Data *sd)
{
   Evas_Object *handle;

   handle = _decoration_create(obj, sd, PART_NAME_HANDLER_START, EINA_TRUE);
   efl_canvas_object_pass_events_set(handle, EINA_FALSE);
   sd->start_handler = handle;
   efl_event_callback_add(handle, EFL_EVENT_POINTER_DOWN,
                                  _start_handler_mouse_down_cb, obj);
   efl_event_callback_add(handle, EFL_EVENT_POINTER_MOVE,
                                  _start_handler_mouse_move_cb, obj);
   efl_event_callback_add(handle, EFL_EVENT_POINTER_UP,
                                  _start_handler_mouse_up_cb, obj);
   efl_gfx_entity_visible_set(handle, EINA_TRUE);

   handle = _decoration_create(obj, sd, PART_NAME_HANDLER_END, EINA_TRUE);
   efl_canvas_object_pass_events_set(handle, EINA_FALSE);
   sd->end_handler = handle;
   efl_event_callback_add(handle, EFL_EVENT_POINTER_DOWN,
                                  _end_handler_mouse_down_cb, obj);
   efl_event_callback_add(handle, EFL_EVENT_POINTER_MOVE,
                                  _end_handler_mouse_move_cb, obj);
   efl_event_callback_add(handle, EFL_EVENT_POINTER_UP,
                                  _end_handler_mouse_up_cb, obj);
   efl_gfx_entity_visible_set(handle, EINA_TRUE);
}

EOLIAN static void
_efl_ui_textbox_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Textbox_Data *sd, Eina_Position2D pos)
{
   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
   efl_gfx_entity_position_set(sd->hit_rect, pos);

   if (sd->popup) _popup_position(obj);
}

EOLIAN static void
_efl_ui_textbox_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Textbox_Data *sd, Eina_Size2D sz)
{
   efl_gfx_entity_size_set(sd->hit_rect, sz);
   _update_selection_handler(obj);

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
}

EOLIAN static void
_efl_ui_textbox_efl_gfx_entity_visible_set(Eo *obj, Efl_Ui_Textbox_Data *sd EINA_UNUSED, Eina_Bool vis)
{
   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), vis);
   if (vis) _update_selection_handler(obj);
}

EOLIAN static void
_efl_ui_textbox_efl_canvas_group_group_member_add(Eo *obj, Efl_Ui_Textbox_Data *sd, Evas_Object *member)
{
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), member);

   if (sd->hit_rect)
     efl_gfx_stack_raise_to_top(sd->hit_rect);
}

static void
_update_guide_text(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   const char *txt;
   Eina_Bool show_guide;

   txt = efl_text_get(sd->text_obj);

   show_guide = (!txt || (txt[0] == '\0'));

   efl_gfx_entity_visible_set(sd->text_guide_obj, show_guide);

}

/**
 * @internal
 * Returns the numeric value of HEX chars for example for ch = 'A'
 * the function will return 10.
 *
 * @param ch The HEX char.
 * @return numeric value of HEX.
 */
static int
_hex_string_get(char ch, Eina_Bool *ok)
{
   if ((ch >= '0') && (ch <= '9')) return (ch - '0');
   else if ((ch >= 'A') && (ch <= 'F')) return (ch - 'A' + 10);
   else if ((ch >= 'a') && (ch <= 'f')) return (ch - 'a' + 10);
   *ok = EINA_FALSE;
   return 0;
}


static inline Eina_Bool
_format_color_parse(const char *str, int slen,
      unsigned char *r, unsigned char *g,
      unsigned char *b, unsigned char *a)
{
   Eina_Bool v = EINA_TRUE;

   *r = *g = *b = *a = 0;

   if (slen == 7) /* #RRGGBB */
     {
        *r = (_hex_string_get(str[1], &v) << 4) | (_hex_string_get(str[2], &v));
        *g = (_hex_string_get(str[3], &v) << 4) | (_hex_string_get(str[4], &v));
        *b = (_hex_string_get(str[5], &v) << 4) | (_hex_string_get(str[6], &v));
        *a = 0xff;
     }
   else if (slen == 9) /* #RRGGBBAA */
     {
        *r = (_hex_string_get(str[1], &v) << 4) | (_hex_string_get(str[2], &v));
        *g = (_hex_string_get(str[3], &v) << 4) | (_hex_string_get(str[4], &v));
        *b = (_hex_string_get(str[5], &v) << 4) | (_hex_string_get(str[6], &v));
        *a = (_hex_string_get(str[7], &v) << 4) | (_hex_string_get(str[8], &v));
     }
   else if (slen == 4) /* #RGB */
     {
        *r = _hex_string_get(str[1], &v);
        *r = (*r << 4) | *r;
        *g = _hex_string_get(str[2], &v);
        *g = (*g << 4) | *g;
        *b = _hex_string_get(str[3], &v);
        *b = (*b << 4) | *b;
        *a = 0xff;
     }
   else if (slen == 5) /* #RGBA */
     {
        *r = _hex_string_get(str[1], &v);
        *r = (*r << 4) | *r;
        *g = _hex_string_get(str[2], &v);
        *g = (*g << 4) | *g;
        *b = _hex_string_get(str[3], &v);
        *b = (*b << 4) | *b;
        *a = _hex_string_get(str[4], &v);
        *a = (*a << 4) | *a;
     }
   else v = EINA_FALSE;

   *r = (*r * *a) / 255;
   *g = (*g * *a) / 255;
   *b = (*b * *a) / 255;
   return v;
}

/**
  * @internal
  * Updates the text properties of the object from the theme.
  *
  * This update functions skips any property that was already set,
  * to allow users to override the theme during the construction of the widget.
  */
static void
_update_text_theme(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   const char *font_name;
   const char *font_size;
   const char *colorcode = NULL;
   Eina_Bool disabled;

   int font_size_n;
   unsigned char r, g, b, a;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   disabled = efl_ui_widget_disabled_get(obj);

   // Main Text
   // font_set
   if (!efl_text_font_family_get(sd->text_obj))
     {
        font_name = efl_layout_group_data_get(wd->resize_obj, "font.name");
        efl_text_font_family_set(sd->text_obj, font_name);
     }
   if (!efl_text_font_size_get(sd->text_obj))
     {
        font_size = efl_layout_group_data_get(wd->resize_obj, "font.size");
        font_size_n = font_size ? atoi(font_size) : 0;
        efl_text_font_size_set(sd->text_obj, font_size_n);
     }

   // color
   if (!sd->color_is_set)
     {
        // If user set color by him self, we will not change it back even if
        // control become disabled.
        if (disabled)
          colorcode = efl_layout_group_data_get(wd->resize_obj, "style.color_disabled");
        if (!colorcode)
          colorcode = efl_layout_group_data_get(wd->resize_obj, "style.color");
        if (colorcode && !strncmp(colorcode, "cc:", 3))
          {
             int r, g, b, a;

             if (edje_object_color_class_get(wd->resize_obj, colorcode + 3,
                                             &r, &g, &b, &a,
                                             NULL, NULL, NULL, NULL,
                                             NULL, NULL, NULL, NULL))
               {
                  r = (r * a) / 255;
                  g = (g * a) / 255;
                  b = (b * a) / 255;
                  efl_text_color_set(sd->text_obj, r, g, b, a);
               }
          }
        else if (colorcode && _format_color_parse(colorcode, strlen(colorcode), &r, &g, &b, &a))
          {
             efl_text_color_set(sd->text_obj, r, g, b, a);
          }
     }

   // Guide Text
   if (!efl_text_font_family_get(sd->text_guide_obj))
     {
        font_name = efl_layout_group_data_get(wd->resize_obj, "guide.font.name");
        efl_text_font_family_set(sd->text_guide_obj, font_name);
     }
   if (!efl_text_font_size_get(sd->text_guide_obj))
     {
        font_size = efl_layout_group_data_get(wd->resize_obj, "guide.font.size");
        font_size_n = font_size ? atoi(font_size) : 0;
        efl_text_font_size_set(sd->text_guide_obj, font_size_n);
     }

   colorcode = NULL;
   // color
   if (disabled)
     colorcode = efl_layout_group_data_get(wd->resize_obj, "guide.style.color_disabled");
   if (!colorcode)
     colorcode = efl_layout_group_data_get(wd->resize_obj, "guide.style.color");
   if (colorcode && !strncmp(colorcode, "cc:", 3))
     {
        int r, g, b, a;

        if (edje_object_color_class_get(wd->resize_obj, colorcode + 3,
                                        &r, &g, &b, &a,
                                        NULL, NULL, NULL, NULL,
                                        NULL, NULL, NULL, NULL))
          {
             r = (r * a) / 255;
             g = (g * a) / 255;
             b = (b * a) / 255;
             efl_text_color_set(sd->text_guide_obj, r, g, b, a);
          }
     }
   else if(colorcode && _format_color_parse(colorcode, strlen(colorcode), &r, &g, &b, &a))
     {
        efl_text_color_set(sd->text_guide_obj, r, g, b, a);
     }
}

EOLIAN static Eo *
_efl_ui_textbox_efl_object_constructor(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   Eo *text_obj;

   sd->content = EFL_UI_TEXTBOX_CNP_CONTENT_MARKUP;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "text");

   efl_event_callback_add(obj, EFL_UI_SELECTION_EVENT_WM_SELECTION_CHANGED, _selection_lost_cb, NULL);
   efl_event_callback_add(obj, EFL_UI_DND_EVENT_DROP_ENTERED, _dnd_enter_cb, NULL);
   efl_event_callback_add(obj, EFL_UI_DND_EVENT_DROP_POSITION_CHANGED, _dnd_pos_cb, NULL);
   efl_event_callback_add(obj, EFL_UI_DND_EVENT_DROP_DROPPED, _dnd_drop_cb, NULL);

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_event_callback_add(obj, EFL_INPUT_EVENT_LONGPRESSED, _long_press_cb, obj);

   text_obj = efl_add(EFL_UI_INTERNAL_TEXT_INTERACTIVE_CLASS, obj);
   efl_event_callback_forwarder_add(text_obj, EFL_TEXT_INTERACTIVE_EVENT_CHANGED_USER, obj);
   efl_event_callback_forwarder_add(text_obj, EFL_TEXT_INTERACTIVE_EVENT_HAVE_SELECTION_CHANGED, obj);
   efl_event_callback_forwarder_add(text_obj, EFL_TEXT_INTERACTIVE_EVENT_SELECTION_CHANGED, obj);
   efl_event_callback_forwarder_add(text_obj, EFL_TEXT_INTERACTIVE_EVENT_REDO_REQUEST, obj);
   efl_event_callback_forwarder_add(text_obj, EFL_TEXT_INTERACTIVE_EVENT_UNDO_REQUEST, obj);
   efl_event_callback_forwarder_add(text_obj, EFL_TEXT_INTERACTIVE_EVENT_PREEDIT_CHANGED, obj);
   sd->text_obj = text_obj;
   sd->text_guide_obj = efl_add(EFL_CANVAS_TEXTBLOCK_CLASS, obj);
   sd->text_table = efl_add(EFL_UI_TABLE_CLASS, obj);
   efl_composite_attach(obj, text_obj);

   sd->entry_edje = wd->resize_obj;
   sd->context_menu_enabled = EINA_TRUE;
   efl_text_interactive_editable_set(obj, EINA_TRUE);
   efl_text_interactive_selection_allowed_set(obj, EINA_TRUE);
   sd->last.scroll = EINA_SIZE2D(0, 0);
   sd->sel_handles_enabled = EINA_FALSE;

   return obj;
}

EOLIAN static Eo *
_efl_ui_textbox_efl_object_finalize(Eo *obj,
                                 Efl_Ui_Textbox_Data *sd)
{
   obj = efl_finalize(efl_super(obj, MY_CLASS));

   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_ENTRY);

   _update_guide_text(obj, sd);

   sd->item_fallback_factory = efl_add(EFL_UI_TEXT_FACTORY_FALLBACK_CLASS, obj);

   efl_gfx_hint_weight_set
      (sd->entry_edje, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   efl_gfx_hint_align_set
      (sd->entry_edje, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_event_callback_add(sd->text_obj, EFL_TEXT_INTERACTIVE_EVENT_CHANGED_USER,
         _efl_ui_textbox_changed_user_cb, obj);
   efl_event_callback_add(sd->text_obj, EFL_CANVAS_TEXTBLOCK_EVENT_CHANGED,
         _efl_ui_textbox_changed_cb, obj);
   efl_event_callback_add(sd->text_obj, EFL_TEXT_INTERACTIVE_EVENT_HAVE_SELECTION_CHANGED,
         _efl_ui_textbox_selection_start_clear_cb, obj);
   efl_event_callback_add(sd->text_obj, EFL_TEXT_INTERACTIVE_EVENT_SELECTION_CHANGED,
         _efl_ui_textbox_selection_changed_cb, obj);
   efl_event_callback_add(efl_text_interactive_main_cursor_get(sd->text_obj), EFL_TEXT_CURSOR_OBJECT_EVENT_CHANGED,
         _efl_ui_textbox_cursor_changed_cb, obj);
   efl_event_callback_add(sd->text_obj, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED,
         _text_position_changed_cb, obj);
   efl_event_callback_add(sd->entry_edje, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED,
         _efl_ui_textbox_move_cb, obj);

   efl_event_callback_add
     (sd->entry_edje, EFL_EVENT_POINTER_DOWN, _mouse_down_cb, obj);
   efl_event_callback_add
     (sd->entry_edje, EFL_EVENT_POINTER_UP, _mouse_up_cb, obj);
   efl_event_callback_add
     (sd->entry_edje, EFL_EVENT_POINTER_MOVE, _mouse_move_cb, obj);
   efl_ui_action_connector_bind_clickable_to_object(sd->entry_edje, obj);

   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED,
         _text_size_changed_cb, obj);

   efl_ui_widget_focus_allow_set(obj, efl_text_interactive_editable_get(obj));

   efl_input_text_input_panel_layout_set(obj, EFL_INPUT_TEXT_PANEL_LAYOUT_TYPE_NORMAL);
   efl_input_text_input_panel_autoshow_set(obj, EINA_TRUE);
   efl_input_text_predictable_set(obj, EINA_TRUE);
   efl_input_text_input_content_type_set(obj, EFL_INPUT_TEXT_CONTENT_TYPE_AUTO_COMPLETE);

   sd->calc_force = EINA_TRUE;

   return obj;

}

EOLIAN static void
_efl_ui_textbox_efl_object_destructor(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   efl_event_freeze(obj);

   _popup_dismiss(sd);
   if ((sd->api) && (sd->api->obj_unhook))
     sd->api->obj_unhook(obj);  // module - unhook

   entries = eina_list_remove(entries, obj);
   eina_stringshare_del(sd->text);
   eina_stringshare_del(sd->anchor_hover.hover_style);

   efl_event_thaw(obj);

   if (sd->start_handler)
     {
        efl_del(sd->start_handler);
        efl_del(sd->end_handler);
     }

   _anchors_free(sd);
   _clear_text_selection(sd);

   if (sd->item_factory) efl_unref(sd->item_factory);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_textbox_efl_text_format_multiline_set(Eo *obj, Efl_Ui_Textbox_Data *sd, Eina_Bool enabled)
{
   enabled = !!enabled;
   if (efl_text_multiline_get(obj) == enabled) return;
   efl_text_multiline_set(sd->text_obj, enabled);

   if (sd->scroller)
     {
        if (enabled)
          {
             efl_ui_internal_text_scroller_mode_set(sd->scroller, EFL_UI_TEXT_SCROLLER_MODE_MULTILINE);
          }
        else
          {
             efl_ui_internal_text_scroller_mode_set(sd->scroller, EFL_UI_TEXT_SCROLLER_MODE_SINGLELINE);
          }
     }
}

EOLIAN static void
_efl_ui_textbox_efl_text_format_password_set(Eo *obj, Efl_Ui_Textbox_Data *sd, Eina_Bool password)
{
   password = !!password;

   if (efl_text_password_get(obj) == password) return;
   if (!efl_text_replacement_char_get(obj))
     efl_text_replacement_char_set(obj, ENTRY_PASSWORD_MASK_CHARACTER_UTF8);
   efl_text_password_set(sd->text_obj, password);

   if (password)
     {
        efl_text_multiline_set(obj, EINA_FALSE);
        efl_input_text_input_content_type_set(obj, ((efl_input_text_input_content_type_get(obj) & ~EFL_INPUT_TEXT_CONTENT_TYPE_AUTO_COMPLETE) | EFL_INPUT_TEXT_CONTENT_TYPE_SENSITIVE_DATA));
        efl_access_object_role_set(obj, EFL_ACCESS_ROLE_PASSWORD_TEXT);
     }
   else
     {
        efl_text_multiline_set(obj, EINA_TRUE);
        efl_input_text_input_content_type_set(obj, ((efl_input_text_input_content_type_get(obj) | EFL_INPUT_TEXT_CONTENT_TYPE_AUTO_COMPLETE) & ~EFL_INPUT_TEXT_CONTENT_TYPE_SENSITIVE_DATA));
        efl_access_object_role_set(obj, EFL_ACCESS_ROLE_ENTRY);
     }
}

EOLIAN static void
_efl_ui_textbox_efl_text_style_text_color_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *pd, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   pd->color_is_set = EINA_TRUE;
   efl_text_color_set(pd->text_obj, r, g, b, a);
}
static void
_efl_ui_textbox_calc_force(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   sd->calc_force = EINA_TRUE;
   edje_object_calc_force(sd->entry_edje);
   efl_canvas_group_calculate(obj);
}

static const char*
_efl_ui_textbox_selection_get(const Eo *obj, Efl_Ui_Textbox_Data *sd EINA_UNUSED)
{
   Efl_Text_Cursor_Object *start_obj, *end_obj;

   if ((efl_text_password_get(obj))) return NULL;

   efl_text_interactive_selection_cursors_get(obj, &start_obj, &end_obj);
   return efl_text_cursor_object_range_text_get(start_obj, end_obj);
}

EOLIAN static void
_efl_ui_textbox_selection_handles_enabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Eina_Bool enabled)
{
   if (sd->sel_handles_enabled == enabled) return;
   sd->sel_handles_enabled = enabled;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_selection_handles_enabled_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   return sd->sel_handles_enabled;
}

static void
_efl_ui_textbox_entry_insert(Eo *obj, Efl_Ui_Textbox_Data *sd, const char *entry)
{
   Efl_Text_Cursor_Object *cur_obj = efl_text_interactive_main_cursor_get(obj);
   efl_text_cursor_object_text_insert(cur_obj, entry);
   sd->text_changed = EINA_TRUE;
   efl_canvas_group_change(obj);
}

EOLIAN static Efl_Text_Cursor_Object *
_efl_ui_textbox_cursor_create(Eo *obj, Efl_Ui_Textbox_Data *pd)
{
   Eo* cursor = efl_text_cursor_object_create(pd->text_obj);;
   efl_text_cursor_object_text_object_set(cursor, pd->text_obj, obj);
   return cursor;
}

EOLIAN static void
_efl_ui_textbox_efl_text_interactive_editable_set(Eo *obj, Efl_Ui_Textbox_Data *sd, Eina_Bool editable)
{
   if (efl_text_interactive_editable_get(obj) == editable) return;

   efl_text_interactive_editable_set(efl_super(obj, MY_CLASS), editable);

   efl_ui_widget_focus_allow_set(obj, editable);

   if (editable)
     {
        if (sd->cursor)
          {
             efl_gfx_entity_visible_set(sd->cursor, EINA_TRUE);
             efl_gfx_entity_visible_set(sd->cursor_bidi, EINA_TRUE);
          }
     }
   if (!editable && sd->cursor)
     {
        efl_gfx_entity_visible_set(sd->cursor, EINA_FALSE);
        efl_gfx_entity_visible_set(sd->cursor_bidi, EINA_FALSE);
     }
}

static void
_efl_ui_textbox_select_region_set(Eo *obj, Efl_Ui_Textbox_Data *sd EINA_UNUSED, int start, int end)
{
   Efl_Text_Cursor_Object *sel_start, *sel_end;

   if (efl_text_password_get(obj)) return;

   efl_text_interactive_selection_cursors_get(obj, &sel_start, &sel_end);

   efl_text_cursor_object_position_set(sel_start, start);
   efl_text_cursor_object_position_set(sel_end, end);
}

static void
_efl_ui_textbox_select_region_get(Eo *obj, int *start, int *end)
{
   Efl_Text_Cursor_Object *sel_start, *sel_end;

   efl_text_interactive_selection_cursors_get(obj, &sel_start, &sel_end);

   if(start) *start = efl_text_cursor_object_position_get(sel_start);
   if(end) *end = efl_text_cursor_object_position_get(sel_end);
}

EOLIAN static void
_efl_ui_textbox_selection_cut(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   Efl_Text_Cursor_Object *start, *end;
   Efl_Text_Change_Info info = { NULL, 0, 0, 0, 0 };
   char *tmp;
   int end_pos, start_pos;

   /* Store it */
   sd->sel_mode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set
       (sd->entry_edje, "efl.text", EINA_FALSE);
   efl_layout_signal_emit(sd->entry_edje, "efl,state,select,off", "efl");

   if (!_elm_config->desktop_entry)
     efl_ui_widget_scroll_hold_pop(obj);

   /*In password mode, cut will remove text only*/
   if (!efl_text_password_get(obj))
     _selection_store(EFL_UI_CNP_BUFFER_COPY_AND_PASTE, obj);
   efl_text_interactive_selection_cursors_get(obj, &start, &end);

   start_pos = efl_text_cursor_object_position_get(start);
   end_pos = efl_text_cursor_object_position_get(end);
   tmp = efl_text_cursor_object_range_text_get(start, end);
   info.type = EFL_TEXT_CHANGE_TYPE_REMOVE;
   info.position = start_pos;
   info.length = end_pos - start_pos;
   info.content = tmp;
   efl_text_cursor_object_range_delete(start, end);
   efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_CHANGED_USER, &info);
   free(tmp);
   tmp = NULL;
   efl_text_interactive_all_unselect(obj);

   efl_event_callback_call(obj, EFL_UI_TEXTBOX_EVENT_SELECTION_CUT, NULL);
}

EOLIAN static void
_efl_ui_textbox_selection_copy(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   if (efl_text_password_get(obj)) return;

   sd->sel_mode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     {
        edje_object_part_text_select_allow_set
          (sd->entry_edje, "efl.text", EINA_FALSE);
        efl_layout_signal_emit(sd->entry_edje, "efl,state,select,off", "efl");
        efl_ui_widget_scroll_hold_pop(obj);
     }
   _selection_store(EFL_UI_CNP_BUFFER_COPY_AND_PASTE, obj);
   efl_event_callback_call(obj, EFL_UI_TEXTBOX_EVENT_SELECTION_COPY, NULL);
}
static void
_efl_ui_textbox_selection_paste_type(Eo *obj, Efl_Ui_Textbox_Data *sd, Efl_Ui_Cnp_Buffer type)
{
   Eina_Future *future;
   Eina_Array *types = _figure_out_types(obj, sd);

   future = efl_ui_selection_get(obj, type, evas_device_seat_id_get(evas_default_device_get(evas_object_evas_get(obj), EVAS_DEVICE_CLASS_SEAT)), eina_array_iterator_new(types));

   efl_future_then(obj, future, _selection_data_cb);

   efl_event_callback_call(obj, EFL_UI_TEXTBOX_EVENT_SELECTION_PASTE, NULL);
   eina_array_free(types);
}

EOLIAN static void
_efl_ui_textbox_selection_paste(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   _efl_ui_textbox_selection_paste_type(obj, sd, EFL_UI_CNP_BUFFER_COPY_AND_PASTE);
}

EOLIAN static void
_efl_ui_textbox_context_menu_enabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Eina_Bool enabled)
{
   if (sd->context_menu_enabled == enabled) return;
   sd->context_menu_enabled = enabled;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_context_menu_enabled_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   return sd->context_menu_enabled;
}

EOLIAN static void
_efl_ui_textbox_cnp_dnd_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Efl_Ui_Textbox_Cnp_Content content)
{
   sd->content = content;
}

EOLIAN static Efl_Ui_Textbox_Cnp_Content
_efl_ui_textbox_cnp_dnd_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   return sd->content;
}

EOLIAN static void
_efl_ui_textbox_scrollable_set(Eo *obj, Efl_Ui_Textbox_Data *sd, Eina_Bool scroll)
{
   if (sd->scroll == scroll) return;
   sd->scroll = scroll;

   if (scroll)
     {
        efl_content_set(efl_part(sd->entry_edje, "efl.text"), NULL);
        sd->scroller = efl_add(EFL_UI_INTERNAL_TEXT_SCROLLER_CLASS, obj,
              efl_ui_internal_text_scroller_initialize(efl_added,
                 sd->text_obj, sd->text_table));

        if (efl_text_multiline_get(obj))
          efl_ui_internal_text_scroller_mode_set(sd->scroller, EFL_UI_TEXT_SCROLLER_MODE_MULTILINE);
        else
          efl_ui_internal_text_scroller_mode_set(sd->scroller, EFL_UI_TEXT_SCROLLER_MODE_SINGLELINE);

        efl_content_set(efl_part(sd->entry_edje, "efl.text"), sd->scroller);
        efl_canvas_object_clipper_set(sd->cursor,
              efl_ui_internal_text_scroller_viewport_clip_get(sd->scroller));
        efl_event_callback_add(sd->scroller, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED,
              _scroller_size_changed_cb, obj);
     }
   else
     {
        /* sd->text_table should not be deleted, so we need to use content_unset
         * instead of efl_content_set(sd->scroller, NULL)
        */
        efl_content_unset(sd->scroller);
        efl_content_set(efl_part(sd->entry_edje, "efl.text"), sd->text_table);
        efl_del(sd->scroller);
        sd->scroller = NULL;
     }
   efl_canvas_group_change(obj);
}

EOLIAN static Eina_Bool
_efl_ui_textbox_scrollable_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   return sd->scroll;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_ui_widget_on_access_activate(Eo *obj, Efl_Ui_Textbox_Data *_pd EINA_UNUSED, Efl_Ui_Activate act)
{
   if (act != EFL_UI_ACTIVATE_DEFAULT) return EINA_FALSE;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (!efl_ui_widget_disabled_get(obj) &&
       !(efl_event_freeze_count_get(obj) > 0))
     {
        efl_event_callback_call(obj, EFL_INPUT_EVENT_CLICKED, NULL);
        if (efl_text_interactive_editable_get(obj) && efl_input_text_input_panel_autoshow_get(obj))
          edje_object_part_text_input_panel_show(sd->entry_edje, "efl.text");
     }
   return EINA_TRUE;
}

// ATSPI Accessibility

EOLIAN static Eina_Unicode
_efl_ui_textbox_efl_access_text_character_get(const Eo *obj, Efl_Ui_Textbox_Data *_pd EINA_UNUSED, int offset)
{
   const char *txt;
   int idx = 0;
   Eina_Unicode ret = 0;
   if (offset < 0) return ret;

   if (efl_text_password_get(obj)) return ENTRY_PASSWORD_MASK_CHARACTER;

   txt = efl_text_get(obj);
   if (!txt) return ret;

   ret = eina_unicode_utf8_next_get(txt, &idx);
   while (offset--) ret = eina_unicode_utf8_next_get(txt, &idx);

   return ret;
}

EOLIAN static int
_efl_ui_textbox_efl_access_text_character_count_get(const Eo *obj, Efl_Ui_Textbox_Data *_pd EINA_UNUSED)
{
   const char *txt;

   txt = efl_text_get(obj);
   if (!txt) return -1;
   return eina_unicode_utf8_get_len(txt);
}

EOLIAN static void
_efl_ui_textbox_efl_access_text_string_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *pd, Efl_Access_Text_Granularity granularity, int *start_offset, int *end_offset, char **ret EFL_TRANSFER_OWNERSHIP)
{
   Evas_Textblock_Cursor *cur = NULL, *cur2 = NULL;

   EINA_SAFETY_ON_NULL_RETURN(ret);
   *ret = NULL;

   cur = evas_object_textblock_cursor_new(pd->text_obj);
   cur2 = evas_object_textblock_cursor_new(pd->text_obj);
   if (!cur || !cur2) goto fail;

   evas_textblock_cursor_pos_set(cur, *start_offset);
   if (evas_textblock_cursor_pos_get(cur) != *start_offset) goto fail;

   switch (granularity)
     {
      case EFL_ACCESS_TEXT_GRANULARITY_CHAR:
         break;
      case EFL_ACCESS_TEXT_GRANULARITY_WORD:
         evas_textblock_cursor_word_start(cur);
         break;
      case EFL_ACCESS_TEXT_GRANULARITY_SENTENCE:
         // TODO - add sentence support in textblock first
         break;
      case EFL_ACCESS_TEXT_GRANULARITY_LINE:
         evas_textblock_cursor_line_char_first(cur);
         break;
      case EFL_ACCESS_TEXT_GRANULARITY_PARAGRAPH:
         evas_textblock_cursor_paragraph_char_first(cur);
         break;
     }

   *start_offset = evas_textblock_cursor_pos_get(cur);
   evas_textblock_cursor_copy(cur, cur2);

   switch (granularity)
     {
      case EFL_ACCESS_TEXT_GRANULARITY_CHAR:
         evas_textblock_cursor_char_next(cur2);
         break;
      case EFL_ACCESS_TEXT_GRANULARITY_WORD:
         evas_textblock_cursor_word_end(cur2);
         // since word_end sets cursor position ON (before) last
         // char of word, we need to manually advance cursor to get
         // proper string from function range_text_get
         evas_textblock_cursor_char_next(cur2);
         break;
      case EFL_ACCESS_TEXT_GRANULARITY_SENTENCE:
         // TODO - add sentence support in textblock first
         break;
      case EFL_ACCESS_TEXT_GRANULARITY_LINE:
         evas_textblock_cursor_line_char_last(cur2);
         break;
      case EFL_ACCESS_TEXT_GRANULARITY_PARAGRAPH:
         evas_textblock_cursor_paragraph_char_last(cur2);
         break;
     }

   if (end_offset) *end_offset = evas_textblock_cursor_pos_get(cur2);

   *ret = evas_textblock_cursor_range_text_get(cur, cur2, EVAS_TEXTBLOCK_TEXT_PLAIN);

   evas_textblock_cursor_free(cur);
   evas_textblock_cursor_free(cur2);

   if (*ret && efl_text_password_get(obj))
     {
        int i = 0;
        while (*ret[i] != '\0')
         *ret[i++] = ENTRY_PASSWORD_MASK_CHARACTER;
     }

   return;

fail:
   if (start_offset) *start_offset = -1;
   if (end_offset) *end_offset = -1;
   if (cur) evas_textblock_cursor_free(cur);
   if (cur2) evas_textblock_cursor_free(cur2);
   *ret = NULL;
}

EOLIAN static char*
_efl_ui_textbox_efl_access_text_access_text_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *pd EINA_UNUSED, int start_offset, int end_offset)
{
   Evas_Textblock_Cursor *cur = NULL, *cur2 = NULL;
   char *ret = NULL;
   Eo *text_obj = pd->text_obj;

   cur = evas_object_textblock_cursor_new(text_obj);
   cur2 = evas_object_textblock_cursor_new(text_obj);
   if (!cur || !cur2) goto fail;

   evas_textblock_cursor_pos_set(cur, start_offset);
   if (evas_textblock_cursor_pos_get(cur) != start_offset) goto fail;

   evas_textblock_cursor_pos_set(cur2, end_offset);
   if (evas_textblock_cursor_pos_get(cur2) != end_offset) goto fail;

   ret = evas_textblock_cursor_range_text_get(cur, cur2, EVAS_TEXTBLOCK_TEXT_PLAIN);

   evas_textblock_cursor_free(cur);
   evas_textblock_cursor_free(cur2);

   if (ret && efl_text_password_get(obj))
     {
        int i = 0;
        while (ret[i] != '\0')
         ret[i++] = ENTRY_PASSWORD_MASK_CHARACTER;
     }

   return ret;

fail:
   if (cur) evas_textblock_cursor_free(cur);
   if (cur2) evas_textblock_cursor_free(cur2);
   return NULL;
}

EOLIAN static int
_efl_ui_textbox_efl_access_text_caret_offset_get(const Eo *obj, Efl_Ui_Textbox_Data *_pd EINA_UNUSED)
{
   return efl_text_cursor_object_position_get(efl_text_interactive_main_cursor_get(obj));
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_text_caret_offset_set(Eo *obj, Efl_Ui_Textbox_Data *_pd EINA_UNUSED, int offset)
{
   efl_text_cursor_object_position_set(efl_text_interactive_main_cursor_get(obj), offset);
   return EINA_TRUE;
}

EOLIAN static int
_efl_ui_textbox_efl_access_text_selections_count_get(const Eo *obj, Efl_Ui_Textbox_Data *_pd EINA_UNUSED)
{
   return _efl_ui_textbox_selection_get(obj, _pd) ? 1 : 0;
}

EOLIAN static void
_efl_ui_textbox_efl_access_text_access_selection_get(const Eo *obj, Efl_Ui_Textbox_Data *_pd EINA_UNUSED, int selection_number, int *start_offset, int *end_offset)
{
   if (selection_number != 0) return;

   _efl_ui_textbox_select_region_get((Eo *)obj, start_offset, end_offset);
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_text_access_selection_set(Eo *obj, Efl_Ui_Textbox_Data *_pd EINA_UNUSED, int selection_number, int start_offset, int end_offset)
{
   if (selection_number != 0) return EINA_FALSE;

   _efl_ui_textbox_select_region_set(obj, _pd, start_offset, end_offset);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_text_selection_remove(Eo *obj, Efl_Ui_Textbox_Data *pd EINA_UNUSED, int selection_number)
{
   if (selection_number != 0) return EINA_FALSE;
   efl_text_interactive_all_unselect(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_text_selection_add(Eo *obj, Efl_Ui_Textbox_Data *pd EINA_UNUSED, int start_offset, int end_offset)
{
   _efl_ui_textbox_select_region_set(obj, pd, start_offset, end_offset);

   return EINA_TRUE;
}

EOLIAN static Eina_List*
_efl_ui_textbox_efl_access_text_bounded_ranges_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *_pd EINA_UNUSED, Eina_Bool screen_coods EINA_UNUSED, Eina_Rect rect EINA_UNUSED, Efl_Access_Text_Clip_Type xclip EINA_UNUSED, Efl_Access_Text_Clip_Type yclip EINA_UNUSED)
{
   return NULL;
}

EOLIAN static int
_efl_ui_textbox_efl_access_text_offset_at_point_get(const Eo *obj, Efl_Ui_Textbox_Data *pd EINA_UNUSED, Eina_Bool screen_coods, int x, int y)
{
   Evas_Textblock_Cursor *cur;
   int ret;
   Eo *text_obj = pd->text_obj;

   if (!text_obj) return -1;

   cur = evas_object_textblock_cursor_new(text_obj);
   if (!cur) return -1;

   if (screen_coods)
     {
        int ee_x, ee_y;
        Ecore_Evas *ee= ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
        x -= ee_x;
        y -= ee_y;
     }

   if (!evas_textblock_cursor_char_coord_set(cur, x, y))
     {
        evas_textblock_cursor_free(cur);
        return -1;
     }

   ret = evas_textblock_cursor_pos_get(cur);
   evas_textblock_cursor_free(cur);

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_text_character_extents_get(const Eo *obj, Efl_Ui_Textbox_Data *pd EINA_UNUSED, int offset, Eina_Bool screen_coods, Eina_Rect *rect)
{
   Evas_Textblock_Cursor *cur;
   int ret;
   Eo *text_obj = pd->text_obj;

   if (!text_obj) return EINA_FALSE;

   cur = evas_object_textblock_cursor_new(text_obj);
   if (!cur) return EINA_FALSE;

   evas_textblock_cursor_pos_set(cur, offset);

   ret = evas_textblock_cursor_char_geometry_get(cur, &rect->x, &rect->y, &rect->w, &rect->h);
   evas_textblock_cursor_free(cur);

   if (ret == -1) return EINA_FALSE;

   if (screen_coods)
     {
        int ee_x, ee_y;
        Ecore_Evas *ee= ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
        rect->x += ee_x;
        rect->y += ee_y;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_text_range_extents_get(const Eo *obj, Efl_Ui_Textbox_Data *pd EINA_UNUSED, Eina_Bool screen_coods, int start_offset, int end_offset, Eina_Rect *rect)
{
   Evas_Textblock_Cursor *cur1, *cur2;
   int ret;
   int x, xx, y, yy;
   Eo *text_obj = pd->text_obj;

   if (!text_obj) return EINA_FALSE;

   cur1 = evas_object_textblock_cursor_new(text_obj);
   if (!cur1) return EINA_FALSE;

   cur2 = evas_object_textblock_cursor_new(text_obj);
   if (!cur2)
     {
        evas_textblock_cursor_free(cur1);
        return EINA_FALSE;
     }

   evas_textblock_cursor_pos_set(cur1, start_offset);
   evas_textblock_cursor_pos_set(cur2, end_offset);

   ret = evas_textblock_cursor_char_geometry_get(cur1, &x, &y, NULL, NULL);
   ret += evas_textblock_cursor_char_geometry_get(cur2, &xx, &yy, NULL, NULL);

   evas_textblock_cursor_free(cur1);
   evas_textblock_cursor_free(cur2);

   if (ret != 0) return EINA_FALSE;

   rect->x = x < xx ? x : xx;
   rect->y = y < yy ? y : yy;
   rect->w = abs(x - xx);
   rect->h = abs(y - yy);

   if (screen_coods)
     {
        int ee_x, ee_y;
        Ecore_Evas *ee= ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        ecore_evas_geometry_get(ee, &ee_x, &ee_y, NULL, NULL);
        rect->x += ee_x;
        rect->y += ee_y;
     }

   return EINA_TRUE;
}

static Efl_Access_Text_Attribute*
_textblock_node_format_to_atspi_text_attr(Efl_Text_Attribute_Handle *annotation)
{
   Efl_Access_Text_Attribute *ret;
   const char *txt;

   txt = efl_text_formatter_attribute_get(annotation);
   if (!txt) return NULL;

   ret = calloc(1, sizeof(Efl_Access_Text_Attribute));
   if (!ret) return NULL;

   ret->value = eina_stringshare_add(txt);
   int size = strlen(txt);
   ret->name = eina_stringshare_add_length(txt, size);

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_text_attribute_get(const Eo *obj, Efl_Ui_Textbox_Data *_pd EINA_UNUSED, const char *attr_name EINA_UNUSED, int *start_offset, int *end_offset, char **value)
{
   Efl_Text_Cursor_Object *cur1, *cur2;
   Efl_Access_Text_Attribute *attr;
   Eina_Iterator *annotations;
   Efl_Text_Attribute_Handle *an;

   Eo *mobj = (Eo *)obj;
   cur1 = efl_ui_textbox_cursor_create(mobj);
   if (!cur1) return EINA_FALSE;

   cur2 = efl_ui_textbox_cursor_create(mobj);
   if (!cur2)
     {
        efl_del(cur1);
        return EINA_FALSE;
     }

   efl_text_cursor_object_position_set(cur1, *start_offset);
   efl_text_cursor_object_position_set(cur2, *end_offset);

   annotations = efl_text_formatter_range_attributes_get(cur1, cur2);

   efl_del(cur1);
   efl_del(cur2);

   if (!annotations) return EINA_FALSE;

   EINA_ITERATOR_FOREACH(annotations, an)
     {
        attr = _textblock_node_format_to_atspi_text_attr(an);
        if (!attr) continue;
        if (!strcmp(attr->name, attr_name))
          {
             *value = attr->value ? strdup(attr->value) : NULL;
             elm_atspi_text_text_attribute_free(attr);
             return EINA_TRUE;
          }
        elm_atspi_text_text_attribute_free(attr);
     }
   eina_iterator_free(annotations);

   return EINA_FALSE;
}

EOLIAN static void
_efl_ui_textbox_efl_access_text_text_attributes_get(const Eo *obj, Efl_Ui_Textbox_Data *pd EINA_UNUSED, int *start_offset, int *end_offset, Eina_List **ret EFL_TRANSFER_OWNERSHIP)
{
   Efl_Text_Cursor_Object *cur1, *cur2;
   Efl_Access_Text_Attribute *attr;
   Eina_Iterator *annotations;
   Efl_Text_Attribute_Handle *an;
   Eo *mobj = (Eo *)obj;

   EINA_SAFETY_ON_NULL_RETURN(ret);
   *ret = NULL;

   cur1 = efl_ui_textbox_cursor_create(mobj);
   if (!cur1) return;

   cur2 = efl_ui_textbox_cursor_create(mobj);
   if (!cur2)
     {
        efl_del(cur1);
        return;
     }

   efl_text_cursor_object_position_set(cur1, *start_offset);
   efl_text_cursor_object_position_set(cur2, *end_offset);

   annotations = efl_text_formatter_range_attributes_get(cur1, cur2);

   efl_del(cur1);
   efl_del(cur2);

   if (!annotations) return;

   EINA_ITERATOR_FOREACH(annotations, an)
     {
        attr = _textblock_node_format_to_atspi_text_attr(an);
        if (!attr) continue;
        *ret = eina_list_append(*ret, attr);
     }
   eina_iterator_free(annotations);
}

EOLIAN static Eina_List*
_efl_ui_textbox_efl_access_text_default_attributes_get(const Eo *obj, Efl_Ui_Textbox_Data *_pd EINA_UNUSED)
{
   Eina_List *ret = NULL;
   Efl_Access_Text_Attribute *attr;
   Efl_Text_Cursor_Object *start, *end;
   Eina_Iterator *annotations;
   Efl_Text_Attribute_Handle *an;

   /* Retrieve all annotations in the text. */
   Eo *mobj = (Eo *)obj; /* XXX const */
   start = efl_ui_textbox_cursor_create(mobj);
   end = efl_ui_textbox_cursor_create(mobj);

   efl_text_cursor_object_move(start, EFL_TEXT_CURSOR_MOVE_TYPE_FIRST);
   efl_text_cursor_object_move(end, EFL_TEXT_CURSOR_MOVE_TYPE_LAST);

   annotations = efl_text_formatter_range_attributes_get(start, end);

   EINA_ITERATOR_FOREACH(annotations, an)
     {
        attr = _textblock_node_format_to_atspi_text_attr(an);
        if (!attr) continue;
        ret = eina_list_append(ret, attr);
     }
   eina_iterator_free(annotations);

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_editable_text_text_content_set(Eo *obj, Efl_Ui_Textbox_Data *pd EINA_UNUSED, const char *content)
{
   efl_text_set(obj, content);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_editable_text_insert(Eo *obj, Efl_Ui_Textbox_Data *pd, const char *string, int position)
{
   Efl_Text_Cursor_Object *cur_obj = efl_text_interactive_main_cursor_get(obj);
   efl_text_cursor_object_position_set(cur_obj, position);
   _efl_ui_textbox_entry_insert(obj, pd, string);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_editable_text_copy(Eo *obj, Efl_Ui_Textbox_Data *pd, int start, int end)
{
   _efl_ui_textbox_select_region_set(obj, pd, start, end);
   efl_ui_textbox_selection_copy(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_editable_text_delete(Eo *obj, Efl_Ui_Textbox_Data *pd, int start_offset, int end_offset)
{
   Evas_Textblock_Cursor *cur1, *cur2;
   Eo *text_obj = pd->text_obj;

   if (!text_obj) return EINA_FALSE;

   cur1 = evas_object_textblock_cursor_new(text_obj);
   if (!cur1) return EINA_FALSE;

   cur2 = evas_object_textblock_cursor_new(text_obj);
   if (!cur2)
     {
        evas_textblock_cursor_free(cur1);
        return EINA_FALSE;
     }

   evas_textblock_cursor_pos_set(cur1, start_offset);
   evas_textblock_cursor_pos_set(cur2, end_offset);

   evas_textblock_cursor_range_delete(cur1, cur2);

   evas_textblock_cursor_free(cur1);
   evas_textblock_cursor_free(cur2);

   _efl_ui_textbox_calc_force(obj, pd);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_editable_text_paste(Eo *obj, Efl_Ui_Textbox_Data *_pd EINA_UNUSED, int position)
{
   Efl_Text_Cursor_Object *cur_obj = efl_text_interactive_main_cursor_get(obj);
   efl_text_cursor_object_position_set(cur_obj, position);
   efl_ui_textbox_selection_paste(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_access_editable_text_cut(Eo *obj, Efl_Ui_Textbox_Data *pd EINA_UNUSED, int start, int end)
{
   _efl_ui_textbox_select_region_set(obj, pd, start, end);
   efl_ui_textbox_selection_cut(obj);
   return EINA_TRUE;
}

EOLIAN static Efl_Access_State_Set
_efl_ui_textbox_efl_access_object_state_set_get(const Eo *obj, Efl_Ui_Textbox_Data *_pd EINA_UNUSED)
{
   Efl_Access_State_Set ret;
   ret = efl_access_object_state_set_get(efl_super(obj, EFL_UI_TEXTBOX_CLASS));

   if (efl_text_interactive_editable_get(obj))
     STATE_TYPE_SET(ret, EFL_ACCESS_STATE_TYPE_EDITABLE);

   return ret;
}

EOLIAN static const char*
_efl_ui_textbox_efl_access_object_i18n_name_get(const Eo *obj, Efl_Ui_Textbox_Data *pd)
{
   const char *name;
   name = efl_access_object_i18n_name_get(efl_super(obj, EFL_UI_TEXTBOX_CLASS));
   if (name && strncmp("", name, 1)) return name;
   const char *ret = edje_object_part_text_get(pd->entry_edje, "efl.guide");
   return ret;
}

static void
_edje_signal_emit(Efl_Ui_Textbox_Data *sd, const char *sig, const char *src)
{
   efl_layout_signal_emit(sd->entry_edje, sig, src);
   efl_layout_signal_emit(sd->cursor, sig, src);
   efl_layout_signal_emit(sd->cursor_bidi, sig, src);
}

static inline Eo *
_decoration_create(Eo *obj, Efl_Ui_Textbox_Data *sd,
      const char *group_name, Eina_Bool above)
{
   Eo *ret = NULL;
   Eo *clip = efl_ui_internal_text_scroller_viewport_clip_get(sd->scroller);

   ret = efl_add(EFL_CANVAS_LAYOUT_CLASS, obj);
   elm_widget_element_update(obj, ret, group_name);
   efl_canvas_group_member_add(sd->entry_edje, ret);
   if (above)
     {
        efl_gfx_stack_above(ret, sd->text_table);
     }
   else
     {
        efl_gfx_stack_below(ret, NULL);
     }
   efl_canvas_object_clipper_set(ret, clip);
   efl_canvas_object_pass_events_set(ret, EINA_TRUE);
   return ret;
}

static void
_create_text_cursors(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   sd->cursor = _decoration_create(obj, sd, PART_NAME_CURSOR, EINA_TRUE);
   sd->cursor_bidi = _decoration_create(obj, sd, PART_NAME_CURSOR, EINA_TRUE);

   if (!efl_text_interactive_editable_get(obj))
     {
        efl_gfx_entity_visible_set(sd->cursor, EINA_FALSE);
        efl_gfx_entity_visible_set(sd->cursor_bidi, EINA_FALSE);
     }
}

static Eina_Position2D
_decoration_calc_offset(Efl_Ui_Textbox_Data *sd)
{
   Eina_Position2D ret;
   Eina_Position2D text;
   Eina_Position2D ed = EINA_POSITION2D(0,0), scr = EINA_POSITION2D(0, 0);

   text = efl_gfx_entity_position_get(sd->text_obj);

   ret.x = ed.x + scr.x + text.x;
   ret.y = ed.y + scr.y + text.y;

   return ret;
}

static void
_update_text_cursors(Eo *obj)
{
   Evas_Coord xx, yy, ww, hh;
   Eina_Position2D off;
   Eo *text_obj;
   Eina_Rect rc_tmp1;
   Eina_Rect rc_tmp2;
   Eina_Bool bidi_cursor;

   EFL_UI_TEXT_DATA_GET(obj, sd);
   if (!sd->deferred_decoration_cursor) return;
   sd->deferred_decoration_cursor = EINA_FALSE;

   text_obj = sd->text_obj;

   xx = yy = ww = hh = -1;
   off =_decoration_calc_offset(sd);
   rc_tmp1 = efl_text_cursor_object_cursor_geometry_get(efl_text_interactive_main_cursor_get(text_obj), EFL_TEXT_CURSOR_TYPE_BEFORE);
   bidi_cursor = efl_text_cursor_object_lower_cursor_geometry_get(efl_text_interactive_main_cursor_get(text_obj), &rc_tmp2);
   xx = rc_tmp1.x;
   yy = rc_tmp1.y;
   ww = rc_tmp1.w;
   hh = rc_tmp1.h;

   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;
   if (sd->cursor)
     {
        efl_gfx_entity_geometry_set(sd->cursor, EINA_RECT(off.x + xx, off.y + yy, ww, hh));
     }
   if (sd->cursor_bidi)
     {
        if (bidi_cursor)
          {
             efl_gfx_entity_geometry_set(sd->cursor_bidi,
                                      EINA_RECT(off.x + rc_tmp2.x, off.y + rc_tmp2.y + (hh / 2),
                                      ww, hh / 2));
             efl_gfx_entity_size_set(sd->cursor, EINA_SIZE2D(ww, hh / 2));
             efl_gfx_entity_visible_set(sd->cursor_bidi, EINA_TRUE);
          }
        else
          {
             efl_gfx_entity_visible_set(sd->cursor_bidi, EINA_FALSE);
          }
     }
   if (sd->cursor_update)
     {
        sd->cursor_update = EINA_FALSE;
        _cursor_geometry_recalc(obj);
     }
}

static void
_clear_text_selection(Efl_Ui_Textbox_Data *sd)
{
   Efl_Ui_Text_Rectangle *r;

   EINA_LIST_FREE(sd->sel, r)
     {
        free(r);
     }

}

static void
_update_text_selection(Eo *obj, Eo *text_obj)
{
   Eina_Position2D off;
   Efl_Text_Cursor_Object *sel_start, *sel_end;

   Eina_List *l;
   Eina_Iterator *range;
   Efl_Ui_Text_Rectangle *rect;
   Eina_Rectangle *r;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (!sd->deferred_decoration_selection) return;
   sd->deferred_decoration_selection = EINA_FALSE;

   off = _decoration_calc_offset(sd);

   efl_text_interactive_selection_cursors_get(text_obj, &sel_start, &sel_end);

   range = efl_text_cursor_object_range_geometry_get(sel_start, sel_end);

   l = sd->sel;
   EINA_ITERATOR_FOREACH(range, r)
     {
        /* Create if there isn't a rectangle to populate. */
        if (!l)
          {
             rect = calloc(1, sizeof(Efl_Ui_Text_Rectangle));
             sd->sel = eina_list_append(sd->sel, rect);

             rect->obj_bg = _decoration_create(obj, sd, PART_NAME_SELECTION, EINA_FALSE);
             efl_gfx_entity_visible_set(rect->obj_bg, EINA_TRUE);
          }
        else
          {
             rect = eina_list_data_get(l);
             l = l->next;
          }

        if (rect->obj_bg)
          {
             efl_gfx_entity_geometry_set(rect->obj_bg, EINA_RECT(off.x + r->x, off.y + r->y,
                                      r->w, r->h));
          }
     }
   eina_iterator_free(range);

   /* delete redundant rectection rects */
   while (l)
     {
        Eina_List *temp = l->next;
        rect = eina_list_data_get(l);
        if (rect)
          {
             if (rect->obj_bg) efl_del(rect->obj_bg);
             free(rect);
          }
        sd->sel = eina_list_remove_list(sd->sel, l);
        l = temp;
     }

   /* Update selection handlers */
   _update_selection_handler(obj);
}

static void
_anchors_free(Efl_Ui_Textbox_Data *sd)
{
   Anchor *an;

   EINA_LIST_FREE(sd->anchors, an)
     {
        Efl_Ui_Text_Rectangle *rect;
        EINA_LIST_FREE(an->rects, rect)
          {
             free(rect);
          }
        free(an->name);
        free(an);
     }
}

static char *
_anchor_format_parse(const char *item)
{
   const char *start, *end;
   char *tmp;
   size_t len;

   start = strchr(item, '=');
   if (!start) return NULL;

   start++; /* Advance after the '=' */
   /* If we can find a quote as the first non-space char,
    * our new delimiter is a quote, not a space. */
   while (*start == ' ')
     start++;

   if (*start == '\'')
     {
        start++;
        end = strchr(start, '\'');
        while ((end) && (end > start) && (end[-1] == '\\'))
          end = strchr(end + 1, '\'');
     }
   else
     {
        end = strchr(start, ' ');
        while ((end) && (end > start) && (end[-1] == '\\'))
          end = strchr(end + 1, ' ');
     }

   /* Null terminate before the spaces */
   if (end) len = end - start;
   else len = strlen(start);

   tmp = malloc(len + 1);
   strncpy(tmp, start, len);
   tmp[len] = '\0';

   return tmp;
}

static Anchor *
_anchor_get(Eo *obj, Efl_Ui_Textbox_Data *sd, Efl_Text_Attribute_Handle *an)
{
   Anchor *anc;
   Eina_List *i;
   const char *str;

   str = efl_text_formatter_attribute_get(an);

   EINA_LIST_FOREACH(sd->anchors, i, anc)
     {
        if (anc->annotation == an) break;
     }

   if (!anc && (efl_text_formatter_attribute_is_item(an) || !strncmp(str, "a ", 2)))
     {
        const char *p;

        anc = calloc(1, sizeof(Anchor));
        if (anc)
          {
             anc->obj = obj;
             anc->annotation = an;
             anc->item = efl_text_formatter_attribute_is_item(an);
             p = strstr(str, "href=");
             if (p)
               {
                  anc->name = _anchor_format_parse(p);
               }
             sd->anchors = eina_list_append(sd->anchors, anc);
          }
     }

   return anc;
}

/**
 * @internal
 * Recreates and updates the anchors in the text.
 */
static void
_anchors_update(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   Evas_Object *smart, *clip;
   Eina_Iterator *it;
   Eina_Position2D off;
   Efl_Text_Cursor_Object *start, *end;
   Efl_Text_Attribute_Handle *an;
   Eina_List *i, *ii;
   Anchor *anc;

   if (!sd->deferred_decoration_anchor) return;
   sd->deferred_decoration_anchor = EINA_FALSE;

   sd->gen++;

   start = efl_canvas_textblock_cursor_create(sd->text_obj);
   end = efl_canvas_textblock_cursor_create(sd->text_obj);

   /* Retrieve all annotations in the text. */
   efl_text_cursor_object_move(start, EFL_TEXT_CURSOR_MOVE_TYPE_FIRST);
   efl_text_cursor_object_move(end, EFL_TEXT_CURSOR_MOVE_TYPE_LAST);

   it = efl_text_formatter_range_attributes_get(start, end);
   efl_del(start);
   efl_del(end);

   smart = efl_canvas_object_render_parent_get(obj);
   clip = efl_canvas_object_clipper_get(sd->text_obj);
   off = _decoration_calc_offset(sd);

   EINA_ITERATOR_FOREACH(it, an)
     {
        anc = _anchor_get(obj, sd, an);

        if (anc)
          {
             anc->gen = sd->gen;

             if (anc->item)
               {
                  Efl_Ui_Text_Rectangle *rect;
                  Evas_Coord cx, cy, cw, ch;

                  // Item anchor (one rectangle)
                  if (!anc->rects)
                    {
                       Eo *ob;

                       rect = calloc(1, sizeof(Efl_Ui_Text_Rectangle));
                       anc->rects = eina_list_append(anc->rects, rect);

                       ob = _item_get(obj, anc->name);

                       if (ob)
                         {
                            efl_canvas_group_member_add(smart, ob);
                            efl_gfx_stack_above(ob, obj);
                            efl_canvas_object_clipper_set(ob, clip);
                            efl_canvas_object_pass_events_set(ob, EINA_TRUE);
                            rect->obj = ob;
                         }
                    }

                  rect = eina_list_data_get(anc->rects);
                  efl_text_formatter_item_geometry_get(anc->annotation, &cx, &cy, &cw, &ch);
                  efl_gfx_entity_size_set(rect->obj, EINA_SIZE2D(cw, ch));
                  efl_gfx_entity_position_set(rect->obj,
                        EINA_POSITION2D(off.x + cx, off.y + cy));
               }
             else
               {
                  // Link anchor (multiple rectangles) i.e. "a href=..."
                  Eina_Iterator *range;
                  Eina_List *l;
                  Eina_Rectangle *r;
                  size_t count;
                  start = efl_ui_textbox_cursor_create(obj);
                  end = efl_ui_textbox_cursor_create(obj);
                  efl_text_formatter_attribute_cursors_get(anc->annotation, start, end);

                  range = efl_text_cursor_object_range_geometry_get(start, end);
                  count = eina_list_count(eina_iterator_container_get(range));

                  // Add additional rectangles if needed
                  while (count > eina_list_count(anc->rects))
                    {
                       Efl_Ui_Text_Rectangle *rect;
                       Eo *ob;
                       rect = calloc(1, sizeof(Efl_Ui_Text_Rectangle));
                       anc->rects = eina_list_append(anc->rects, rect);

                       ob = _decoration_create(obj, sd, PART_NAME_ANCHOR, EINA_TRUE);
                       rect->obj_fg = ob;
                       // hit-rectangle
                       ob = efl_add(EFL_CANVAS_RECTANGLE_CLASS, obj);
                       efl_gfx_color_set(ob, 0, 0, 0, 0);
                       efl_canvas_group_member_add(smart, ob);
                       efl_gfx_stack_above(ob, obj);
                       efl_canvas_object_clipper_set(ob, clip);
                       efl_canvas_object_repeat_events_set(ob, EINA_TRUE);
                       rect->obj = ob;
                       //FIXME: add event handlers
                    }

                  // Remove extra rectangles if needed
                  while (count < eina_list_count(anc->rects))
                    {
                       Efl_Ui_Text_Rectangle *rect;

                       rect = eina_list_data_get(anc->rects);
                       if (rect->obj) efl_del(rect->obj);
                       if (rect->obj_fg) efl_del(rect->obj_fg);
                       if (rect->obj_bg) efl_del(rect->obj_bg);
                       free(rect);
                       anc->rects = eina_list_remove_list(anc->rects, anc->rects);
                    }

                  l = anc->rects;
                  EINA_ITERATOR_FOREACH(range, r)
                    {
                       Efl_Ui_Text_Rectangle *rect;

                       rect = eina_list_data_get(l);
                       if (rect->obj_bg)
                         {
                            efl_gfx_entity_geometry_set(rect->obj_bg,
                                                     EINA_RECT(off.x + r->x, off.y + r->y,
                                                     r->w, r->h));
                            efl_gfx_entity_visible_set(rect->obj_bg, EINA_TRUE);
                         }
                       if (rect->obj_fg)
                         {
                            efl_gfx_entity_geometry_set(rect->obj_fg,
                                                     EINA_RECT(off.x + r->x, off.y + r->y,
                                                     r->w, r->h));
                            efl_gfx_entity_visible_set(rect->obj_fg, EINA_TRUE);
                         }
                       if (rect->obj)
                         {
                            efl_gfx_entity_geometry_set(rect->obj,
                                                     EINA_RECT(off.x + r->x, off.y + r->y,
                                                     r->w, r->h));
                            efl_gfx_entity_visible_set(rect->obj, EINA_TRUE);
                         }

                       l = eina_list_next(l);
                    }
                  eina_iterator_free(range);
               }
          }
     }
   eina_iterator_free(it);

   // Remove anchors that weren't matched to any annotation
   EINA_LIST_FOREACH_SAFE(sd->anchors, i, ii, anc)
     {
        if (anc->gen != sd->gen)
          {
             Efl_Ui_Text_Rectangle *rect;
             sd->anchors = eina_list_remove_list(sd->anchors, i);
             EINA_LIST_FREE(anc->rects, rect)
               {
                  efl_del(rect->obj);
                  efl_del(rect->obj_bg);
                  efl_del(rect->obj_fg);
                  free(rect);
               }
             free(anc->name);
             free(anc);
          }
     }
}

static void
_update_decorations(Eo *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);
   Eo *text_obj = sd->text_obj;

   efl_event_freeze(sd->text_obj);
   _update_text_cursors(obj);
   _update_text_selection(obj, text_obj);
   _anchors_update(obj, sd);
   efl_event_thaw(sd->text_obj);
}

static Eina_Value
_deferred_decoration_job(Eo *o, void *data EINA_UNUSED, const Eina_Value value EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(o, sd);
   _update_decorations(o);
   sd->deferred_decoration_job = NULL;

   return EINA_VALUE_EMPTY;
}

static void
_decoration_defer(Eo *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);
   if (sd->deferred_decoration_job) return;

   Eina_Future *f = efl_loop_job(efl_main_loop_get());
   sd->deferred_decoration_job = efl_future_then(obj, f, _deferred_decoration_job);
}

static void
_selection_defer(Eo *obj, Efl_Ui_Textbox_Data *sd)
{
   sd->deferred_decoration_selection = EINA_TRUE;
   _decoration_defer(obj);
}

static void
_decoration_defer_all(Eo *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);
   sd->deferred_decoration_anchor = EINA_TRUE;
   sd->deferred_decoration_cursor = EINA_TRUE;
   sd->deferred_decoration_selection = EINA_TRUE;
   _decoration_defer(obj);
}

static void
_efl_ui_textbox_changed_cb(void *data, const Efl_Event *event)
{
   if (efl_invalidated_get(event->object)) return;
   EFL_UI_TEXT_DATA_GET(data, sd);
   sd->text_changed = EINA_TRUE;
   sd->cursor_update = EINA_TRUE;
   _update_guide_text(data, sd);
   efl_event_callback_call(data, EFL_UI_TEXTBOX_EVENT_CHANGED, NULL);
   efl_canvas_group_change(data);
   _decoration_defer(data);
}

static void
_efl_ui_textbox_changed_user_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;

   if (efl_invalidated_get(event->object)) return;
   EFL_UI_TEXT_DATA_GET(obj, sd);
   sd->text_changed = EINA_TRUE;
   _update_guide_text(data, sd);
   efl_canvas_group_change(obj);
   _decoration_defer_all(obj);
}

static void
_efl_ui_textbox_cursor_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   if (efl_invalidated_get(event->object)) return;
   EFL_UI_TEXT_DATA_GET(data, sd);
   sd->cursor_update = EINA_TRUE;
   sd->deferred_decoration_cursor = EINA_TRUE;
   _decoration_defer(data);
}

static void
_scroller_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   if (efl_invalidated_get(event->object)) return;

   _decoration_defer_all(data);
}

static void
_text_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   if (efl_invalidated_get(event->object)) return;

   _decoration_defer_all(data);
}

static void
_text_position_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   if (efl_invalidated_get(event->object)) return;

   _decoration_defer_all(data);
}

static void
_efl_ui_textbox_selection_start_clear_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   if (efl_invalidated_get(event->object)) return;
   Eo *obj = data;
   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (efl_text_interactive_have_selection_get(data))
     {
        if (efl_invalidated_get(event->object)) return;
        _edje_signal_emit(sd, "selection,start", "efl.text");
        _selection_defer(obj, sd);
     }
   else
     {
        Eo *obj = data;
        _edje_signal_emit(sd, "selection,cleared", "efl.text");
        _selection_defer(obj, sd);
     }
}

static void
_efl_ui_textbox_selection_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   if (efl_invalidated_get(event->object)) return;
   Eo *obj = data;
   EFL_UI_TEXT_DATA_GET(obj, sd);
   _edje_signal_emit(sd, "selection,changed", "efl.text");
   _selection_store(EFL_UI_CNP_BUFFER_SELECTION, obj);
   _selection_defer(obj, sd);
}

static void
_efl_ui_textbox_move_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   _decoration_defer_all(data);
}

static void
_efl_ui_textbox_item_factory_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *pd,
      Efl_Canvas_Textblock_Factory *item_factory)
{
   if (pd->item_factory) efl_unref(pd->item_factory);
   pd->item_factory = efl_ref(item_factory);
}

static Eo *
_efl_ui_textbox_item_factory_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *pd)
{
   return pd->item_factory;
}

/*Efl.Ui.Scrollable*/
EOLIAN static Eina_Size2D
_efl_ui_textbox_efl_ui_scrollable_content_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->scroller, EINA_SIZE2D(0, 0));
   return efl_ui_scrollable_content_size_get(sd->scroller);
}

EOLIAN static Eina_Rect
_efl_ui_textbox_efl_ui_scrollable_viewport_geometry_get(const Eo *obj EINA_UNUSED,
                                                                       Efl_Ui_Textbox_Data *sd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->scroller, EINA_RECT_EMPTY());
   return efl_ui_scrollable_viewport_geometry_get(sd->scroller);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_match_content_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Eina_Bool w, Eina_Bool h)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   return efl_ui_scrollable_match_content_set(sd->scroller, !!w, !!h);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_step_size_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Eina_Position2D step)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   efl_ui_scrollable_step_size_set(sd->scroller, step);
}

EOLIAN static Eina_Position2D
_efl_ui_textbox_efl_ui_scrollable_step_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->scroller, EINA_POSITION2D(0, 0));
   return efl_ui_scrollable_step_size_get(sd->scroller);
}

EOLIAN static Eina_Position2D
_efl_ui_textbox_efl_ui_scrollable_content_pos_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->scroller, EINA_POSITION2D(0, 0));
   return efl_ui_scrollable_content_pos_get(sd->scroller);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_content_pos_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Eina_Position2D pos)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   efl_ui_scrollable_content_pos_set(sd->scroller, pos);
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_ui_scrollable_scroll_hold_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->scroller, EINA_FALSE);
   return efl_ui_scrollable_scroll_hold_get(sd->scroller);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_scroll_hold_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Eina_Bool hold)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   efl_ui_scrollable_scroll_hold_set(sd->scroller, !!hold);
}

EOLIAN static Eina_Bool
_efl_ui_textbox_efl_ui_scrollable_scroll_freeze_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->scroller, EINA_FALSE);
   return efl_ui_scrollable_scroll_freeze_get(sd->scroller);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_scroll_freeze_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Eina_Bool freeze)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   efl_ui_scrollable_scroll_freeze_set(sd->scroller, !!freeze);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_bounce_enabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Eina_Bool horiz, Eina_Bool vert)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   efl_ui_scrollable_bounce_enabled_set(sd->scroller, !!horiz, !!vert);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_bounce_enabled_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Eina_Bool *horiz, Eina_Bool *vert)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   efl_ui_scrollable_bounce_enabled_get(sd->scroller, horiz, vert);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_scroll(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Eina_Rect rect, Eina_Bool animation)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   efl_ui_scrollable_scroll(sd->scroller, rect, animation);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_gravity_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, double *x, double *y)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   efl_ui_scrollable_gravity_get(sd->scroller, x, y);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_gravity_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, double x, double y)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   efl_ui_scrollable_gravity_set(sd->scroller, x, y);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_movement_block_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Efl_Ui_Layout_Orientation block)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   efl_ui_scrollable_movement_block_set(sd->scroller, block);
}

EOLIAN static Efl_Ui_Layout_Orientation
_efl_ui_textbox_efl_ui_scrollable_movement_block_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sd->scroller, EFL_UI_LAYOUT_ORIENTATION_DEFAULT);
   return efl_ui_scrollable_movement_block_get(sd->scroller);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_looping_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Eina_Bool loop_h, Eina_Bool loop_v)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   efl_ui_scrollable_looping_set(sd->scroller, !!loop_h, !!loop_v);
}

EOLIAN static void
_efl_ui_textbox_efl_ui_scrollable_looping_get(const Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *sd, Eina_Bool *loop_h, Eina_Bool *loop_v)
{
   EINA_SAFETY_ON_NULL_RETURN(sd->scroller);
   efl_ui_scrollable_looping_get(sd->scroller, loop_h, loop_v);
}

/* Efl.Part begin */

static Eina_Bool
_efl_ui_textbox_text_set(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *pd,
      const char *part, const char *text)
{
   if (!part) return EINA_FALSE;

   if (!strcmp("efl.text_guide", part))
     {
        efl_text_set(pd->text_guide_obj, text);
        return EINA_TRUE;
     }
   else if (!strcmp("efl.text", part))
     {
        efl_text_set(pd->text_obj, text);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static const char *
_efl_ui_textbox_text_get(Eo *obj EINA_UNUSED, Efl_Ui_Textbox_Data *pd,
      const char *part)
{
   if (!part) return NULL;

   if (!strcmp("efl.text_guide", part))
     {
        return efl_text_get(pd->text_guide_obj);
     }
   else if (!strcmp("efl.text", part))
     {
        return efl_text_get(pd->text_obj);
     }

   return NULL;
}

static Eina_Bool
_part_is_efl_ui_textbox_part(const Eo *obj EINA_UNUSED, const char *part)
{
   if (eina_streq(part, "efl.text_guide") || eina_streq(part, "efl.text"))
     return EINA_TRUE;

   return EINA_FALSE;
}

/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_textbox, Efl_Ui_Textbox_Data)

ELM_PART_OVERRIDE_PARTIAL(efl_ui_textbox, EFL_UI_TEXTBOX, Efl_Ui_Textbox_Data, _part_is_efl_ui_textbox_part)
ELM_PART_OVERRIDE_TEXT_SET(efl_ui_textbox, EFL_UI_TEXTBOX, Efl_Ui_Textbox_Data)
ELM_PART_OVERRIDE_TEXT_GET(efl_ui_textbox, EFL_UI_TEXTBOX, Efl_Ui_Textbox_Data)

#include "efl_ui_textbox_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

//EFL_UI_LAYOUT_CONTENT_ALIASES_IMPLEMENT(MY_CLASS_PFX)

#include "efl_ui_textbox.eo.c"

EOLIAN static Eo *
_efl_ui_textbox_async_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   sd->async.enabled = EINA_TRUE;

   // FIXME: should we have to keep this efl_ui_text_xxx classes?
   // Then, going to make new theme for these classes? ex) efl/text_async?
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "text");
   obj = efl_constructor(efl_super(obj, EFL_UI_TEXTBOX_ASYNC_CLASS));

   _update_text_theme(obj, sd);
   return obj;
}

#include "efl_ui_textbox_async.eo.c"
