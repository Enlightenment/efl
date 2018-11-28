#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_TEXT_PROTECTED
#define EFL_ACCESS_EDITABLE_TEXT_PROTECTED
#define ELM_LAYOUT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

#include "elm_entry_common.h"
#include "elm_widget_entry.h"
#include "efl_ui_text.eo.h"
#include "elm_hoversel.eo.h"
#include "efl_ui_text_part.eo.h"
#include "elm_part_helper.h"

typedef struct _Efl_Ui_Text_Data        Efl_Ui_Text_Data;
typedef struct _Efl_Ui_Text_Rectangle   Efl_Ui_Text_Rectangle;
typedef struct _Anchor                  Anchor;
typedef struct _Selection_Loss_Data     Selection_Loss_Data;

/**
 * Base widget smart data extended with entry instance data.
 */
struct _Efl_Ui_Text_Data
{
   Evas_Object                          *hit_rect, *entry_edje, *scr_edje;

   Evas_Object                          *hoversel;
   Evas_Object                          *mgf_bg;
   Evas_Object                          *mgf_clip;
   Evas_Object                          *mgf_proxy;
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
   Ecore_Job                            *deferred_decoration_job;
   Ecore_Timer                          *longpress_timer;
   Ecore_Timer                          *delay_write;
   /* for deferred appending */
   Ecore_Idler                          *append_text_idler;
   char                                 *append_text_left;
   int                                   append_text_position;
   int                                   append_text_len;
   /* Only for clipboard */
   const char                           *cut_sel;
   const char                           *text;
   const char                           *file;
   Elm_Text_Format                       format;
   Evas_Coord                            ent_w, ent_h;
   Evas_Coord                            downx, downy;
   Evas_Coord                            ox, oy;
   Eina_List                            *anchors;
   int                                  gen;
   Eina_List                            *sel;
   Eina_List                            *items; /** context menu item list */
   Efl_Canvas_Text_Factory              *item_factory;
   Efl_Canvas_Text_Factory              *item_fallback_factory;
   Eina_List                            *markup_filters;
   Ecore_Job                            *hov_deljob;
   Mod_Api                              *api; // module api if supplied
   int                                   cursor_pos;
   Elm_Scroller_Policy                   policy_h, policy_v;
   Elm_Wrap_Type                         line_wrap;
   Elm_Input_Panel_Layout                input_panel_layout;
   Elm_Autocapital_Type                  autocapital_type;
   Elm_Input_Panel_Lang                  input_panel_lang;
   Elm_Input_Panel_Return_Key_Type       input_panel_return_key_type;
   Elm_Input_Hints                       input_hints;
   Efl_Text_Cursor_Cursor               *sel_handler_cursor;
   void                                 *input_panel_imdata;
   int                                   input_panel_imdata_len;
   int                                   input_panel_layout_variation;
   int                                   validators;
   struct
     {
        Evas_Object *hover_parent; /**< hover parent object. entry is a hover parent object by default */
        Evas_Object *pop; /**< hidden icon for hover target */
        Evas_Object *hover; /**< hover object */
        const char  *hover_style; /**< style of a hover object */
     } anchor_hover;

   Elm_Sel_Format                        cnp_mode;
   Elm_Sel_Format                        drop_format;

   struct {
        char                             *text;
        Eina_Bool                        enabled;
   } async;

   struct {
        Eina_Size2D                      scroll;
        Eina_Size2D                      layout;
   } last;
   struct
     {
        Eina_Future                      *primary;
        Eina_Future                      *clipboard;
     } sel_future;
   Eina_Bool                             input_panel_return_key_disabled : 1;
   Eina_Bool                             drag_selection_asked : 1;
   Eina_Bool                             sel_handler_disabled : 1;
   Eina_Bool                             start_handler_down : 1;
   Eina_Bool                             start_handler_shown : 1;
   Eina_Bool                             end_handler_down : 1;
   Eina_Bool                             end_handler_shown : 1;
   Eina_Bool                             input_panel_enable : 1;
   Eina_Bool                             prediction_allow : 1;
   Eina_Bool                             selection_asked : 1;
   Eina_Bool                             auto_return_key : 1;
   Eina_Bool                             have_selection : 1;
   Eina_Bool                             deferred_decoration_selection : 1;
   Eina_Bool                             deferred_decoration_cursor : 1;
   Eina_Bool                             deferred_decoration_anchor : 1;
   Eina_Bool                             context_menu : 1;
   Eina_Bool                             long_pressed : 1;
   Eina_Bool                             cur_changed : 1;
   Eina_Bool                             single_line : 1;
   Eina_Bool                             can_write : 1;
   Eina_Bool                             auto_save : 1;
   Eina_Bool                             password : 1;
   Eina_Bool                             editable : 1; // FIXME: This is redundant because of text interactive and should be removed
   Eina_Bool                             disabled : 1;
   Eina_Bool                             h_bounce : 1;
   Eina_Bool                             v_bounce : 1;
   Eina_Bool                             has_text : 1;
   Eina_Bool                             use_down : 1;
   Eina_Bool                             sel_mode : 1;
   Eina_Bool                             sel_allow : 1;
   Eina_Bool                             changed : 1;
   Eina_Bool                             scroll : 1;
   Eina_Bool                             input_panel_show_on_demand : 1;
   Eina_Bool                             anchors_updated : 1;
   Eina_Bool                             fallback_item_provider_disabled : 1;
   Eina_Bool                             text_changed : 1;
   Eina_Bool                             text_resized : 1;
   Eina_Bool                             calc_force : 1;
   Eina_Bool                             cursor_update : 1;
};

struct _Anchor
{
   Eo                    *obj;
   char                  *name;
   Efl_Text_Annotate_Annotation *annotation;
   Eina_List             *rects;
   int                   gen;
   Eina_Bool              item : 1;
   Eina_Bool              updated : 1;
};

#define EFL_UI_TEXT_DATA_GET(o, sd) \
  Efl_Ui_Text_Data * sd = efl_data_scope_get(o, EFL_UI_TEXT_CLASS)

#define EFL_UI_TEXT_DATA_GET_OR_RETURN(o, ptr)         \
  EFL_UI_TEXT_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_TEXT_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_TEXT_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       ERR("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return val;                                    \
    }

#define EFL_UI_TEXT_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), EFL_UI_TEXT_CLASS))) \
    return

struct _Efl_Ui_Text_Rectangle
{
   Evas_Object             *obj_bg, *obj_fg, *obj;
};

struct _Selection_Loss_Data
{
   Eo *obj;
   Efl_Selection_Type stype;
};

#define MY_CLASS EFL_UI_TEXT_CLASS
#define MY_CLASS_PFX efl_ui_text
#define MY_CLASS_NAME "Efl.Ui.Text"
#define MY_CLASS_NAME_LEGACY "elm_entry"

#include "efl_ui_internal_text_interactive.h"
#include "efl_ui_internal_text_scroller.h"

/* Maximum chunk size to be inserted to the entry at once
 * FIXME: This size is arbitrary, should probably choose a better size.
 * Possibly also find a way to set it to a low value for weak computers,
 * and to a big value for better computers. */
#define EFL_UI_TEXT_CHUNK_SIZE 10000
#define EFL_UI_TEXT_DELAY_WRITE_TIME 2.0

#define ENTRY_PASSWORD_MASK_CHARACTER 0x002A

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

static void _create_selection_handlers(Evas_Object *obj, Efl_Ui_Text_Data *sd);
static void _update_decorations(Eo *obj);
static void _create_text_cursors(Eo *obj, Efl_Ui_Text_Data *sd);
static void _efl_ui_text_changed_cb(void *data, const Efl_Event *event);
static void _efl_ui_text_changed_user_cb(void *data, const Efl_Event *event);
static void _efl_ui_text_selection_changed_cb(void *data, const Efl_Event *event);
static void _efl_ui_text_cursor_changed_cb(void *data, const Efl_Event *event);
static void _text_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED);
static void _scroller_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED);
static void _text_position_changed_cb(void *data, const Efl_Event *event EINA_UNUSED);
static void _efl_ui_text_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _efl_ui_text_select_none(Eo *obj, Efl_Ui_Text_Data *sd);
static const char* _efl_ui_text_selection_get(const Eo *obj, Efl_Ui_Text_Data *sd);
static void _edje_signal_emit(Efl_Ui_Text_Data *obj, const char *sig, const char *src);
static void _decoration_defer_all(Eo *obj);
static inline Eo * _decoration_create(Eo *obj, Efl_Ui_Text_Data *sd, const char *source, Eina_Bool above);
static void _decoration_defer(Eo *obj);
static void _clear_text_selection(Efl_Ui_Text_Data *sd);
static void _anchors_free(Efl_Ui_Text_Data *sd);
static void _selection_defer(Eo *obj, Efl_Ui_Text_Data *sd);
static Eina_Position2D _decoration_calc_offset(Efl_Ui_Text_Data *sd);

static char *
_file_load(const char *file)
{
   Eina_File *f;
   char *text = NULL;
   void *tmp = NULL;

   f = eina_file_open(file, EINA_FALSE);
   if (!f) return NULL;

   tmp = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!tmp) goto on_error;

   text = malloc(eina_file_size_get(f) + 1);
   if (!text) goto on_error;

   memcpy(text, tmp, eina_file_size_get(f));
   text[eina_file_size_get(f)] = 0;

   if (eina_file_map_faulted(f, tmp))
     {
        ELM_SAFE_FREE(text, free);
     }

 on_error:
   if (tmp) eina_file_map_free(f, tmp);
   eina_file_close(f);

   return text;
}

static char *
_plain_load(const char *file)
{
   return _file_load(file);
}

static Eina_Bool
_load_do(Evas_Object *obj)
{
   char *text;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (!sd->file)
     {
        elm_object_text_set(obj, "");
        return EINA_TRUE;
     }

   switch (sd->format)
     {
      /* Only available format */
      case ELM_TEXT_FORMAT_PLAIN_UTF8:
         text = _plain_load(sd->file);
         break;

      default:
         text = NULL;
         break;
     }

   if (text)
     {
        efl_text_set(obj, text);
        free(text);
        return EINA_TRUE;
     }
   else
     {
        efl_text_set(obj, "");
        return EINA_FALSE;
     }
}

static void
_text_save(const char *file,
           const char *text)
{
   FILE *f;

   if (!text)
     {
        ecore_file_unlink(file);
        return;
     }

   f = fopen(file, "wb");
   if (!f)
     {
        ERR("Failed to open %s for writing", file);
        return;
     }

   if (fputs(text, f) == EOF)
     ERR("Failed to write text to file %s", file);
   fclose(f);
}

static void
_save_do(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (!sd->file) return;
   switch (sd->format)
     {
      /* Only supported format */
      case ELM_TEXT_FORMAT_PLAIN_UTF8:
        _text_save(sd->file, efl_text_get(obj));
        break;

      case ELM_TEXT_FORMAT_MARKUP_UTF8:
      default:
        break;
     }
}

static void
_efl_ui_text_guide_update(Evas_Object *obj,
                        Eina_Bool has_text)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   if ((has_text) && (!sd->has_text))
     edje_object_signal_emit(sd->entry_edje, "efl,guide,disabled", "efl");
   else if ((!has_text) && (sd->has_text))
     edje_object_signal_emit(sd->entry_edje, "efl,guide,enabled", "efl");

   sd->has_text = has_text;
}

static void
_validate(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);
   Eina_Bool res;
   Elm_Validate_Content vc;
   Eina_Strbuf *buf;

   if (sd->validators == 0) return;

   vc.text = edje_object_part_text_get(sd->entry_edje, "efl.text");
   res = efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_VALIDATE, (void *)&vc);
   buf = eina_strbuf_new();
   eina_strbuf_append_printf(buf, "validation,%s,%s", vc.signal, res == EINA_FALSE ? "fail" : "pass");
   edje_object_signal_emit(sd->scr_edje, eina_strbuf_string_get(buf), "efl");
   eina_tmpstr_del(vc.signal);
   eina_strbuf_free(buf);
}

static void
_filter_free(Elm_Entry_Markup_Filter *tf)
{
   if (tf->func == elm_entry_filter_limit_size)
     {
        Elm_Entry_Filter_Limit_Size *lim = tf->data;

        free(lim);
     }
   else if (tf->func == elm_entry_filter_accept_set)
     {
        Elm_Entry_Filter_Accept_Set *as = tf->data;

        if (as)
          {
             eina_stringshare_del(as->accepted);
             eina_stringshare_del(as->rejected);

             free(as);
          }
     }
   free(tf);
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   edje_object_mirrored_set(sd->entry_edje, rtl);

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
        edje_object_signal_emit(sd->start_handler, "efl,handler,hide", "efl");
        sd->start_handler_shown = EINA_FALSE;
     }
   if (sd->end_handler_shown)
     {
        edje_object_signal_emit(sd->end_handler, "efl,handler,hide", "efl");
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

   parent = elm_widget_parent_get(obj);
   while (parent)
     {
        if (efl_isa(parent, ELM_INTERFACE_SCROLLABLE_MIXIN))
          {
             Eina_Rectangle r;
             EINA_RECTANGLE_SET(&r, 0, 0, 0, 0);
             evas_object_geometry_get(parent, &r.x, &r.y, &r.w, &r.h);
             if (!eina_rectangle_intersection(&rect.rect, &r))
               {
                  rect = EINA_RECT_EMPTY();
                  break;
               }
          }
        parent = elm_widget_parent_get(parent);
     }

   return rect;
}

static void
_update_selection_handler(Eo *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   Evas_Coord sx, sy, sh;
   Evas_Coord ex, ey, eh;

   if (!sd->have_selection)
     {
        _hide_selection_handler(obj);
        return;
     }

   if (!sd->sel_handler_disabled)
     {
        Eina_Rect rect;
        Eina_Position2D off;
        Evas_Coord hx, hy;
        Eina_Bool hidden = EINA_FALSE;
        Efl_Text_Cursor_Cursor *sel_start, *sel_end;

        efl_text_interactive_selection_cursors_get(obj, &sel_start, &sel_end);

        if (!sd->start_handler)
          _create_selection_handlers(obj, sd);

        //evas_object_geometry_get(sd->entry_edje, &ent_x, &ent_y, NULL, NULL);

        efl_text_cursor_geometry_get(obj, sel_start,
              EFL_TEXT_CURSOR_TYPE_BEFORE,
              &sx, &sy, NULL, &sh,
              NULL, NULL, NULL, NULL);
        off = _decoration_calc_offset(sd);
        hx = off.x + sx;
        hy = off.y + sy + sh;
        evas_object_move(sd->start_handler, hx, hy);

        rect = _viewport_region_get(obj);

        if (!eina_rectangle_xcoord_inside(&rect.rect, hx) ||
            !eina_rectangle_ycoord_inside(&rect.rect, hy))
          {
             hidden = EINA_TRUE;
          }
        if (!sd->start_handler_shown && !hidden)
          {
             edje_object_signal_emit(sd->start_handler,
                                     "efl,handler,show", "efl");
             sd->start_handler_shown = EINA_TRUE;
          }
        else if (sd->start_handler_shown && hidden)
          {
             edje_object_signal_emit(sd->start_handler,
                                     "efl,handler,hide", "efl");
             sd->start_handler_shown = EINA_FALSE;
          }

        hidden = EINA_FALSE;
        efl_text_cursor_geometry_get(obj, sel_end,
              EFL_TEXT_CURSOR_TYPE_BEFORE,
              &ex, &ey, NULL, &eh,
              NULL, NULL, NULL, NULL);
        hx = off.x + ex;
        hy = off.y + ey + eh;
        evas_object_move(sd->end_handler, hx, hy);

        if (!eina_rectangle_xcoord_inside(&rect.rect, hx) ||
            !eina_rectangle_ycoord_inside(&rect.rect, hy))
          {
             hidden = EINA_TRUE;
          }
        if (!sd->end_handler_shown && !hidden)
          {
             edje_object_signal_emit(sd->end_handler,
                                     "efl,handler,show", "efl");
             sd->end_handler_shown = EINA_TRUE;
          }
        else if (sd->end_handler_shown && hidden)
          {
             edje_object_signal_emit(sd->end_handler,
                                     "efl,handler,hide", "efl");
             sd->end_handler_shown = EINA_FALSE;
          }
     }
   else
     {
        if (sd->start_handler_shown)
          {
             edje_object_signal_emit(sd->start_handler,
                                     "efl,handler,hide", "efl");
             sd->start_handler_shown = EINA_FALSE;
          }
        if (sd->end_handler_shown)
          {
             edje_object_signal_emit(sd->end_handler,
                                     "efl,handler,hide", "efl");
             sd->end_handler_shown = EINA_FALSE;
          }
     }
}

static void
_selection_data_cb(void *data EINA_UNUSED, Eo *obj,
                   Efl_Selection_Data *sel_data)
{
   Efl_Text_Cursor_Cursor *cur, *start, *end;

   char *buf = eina_slice_strdup(sel_data->content);

   efl_text_interactive_selection_cursors_get(obj, &start, &end);
   if (!efl_text_cursor_equal(obj, start, end))
     {
        efl_canvas_text_range_delete(obj, start, end);
     }
   cur = efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN);
   if (sel_data->format == EFL_SELECTION_FORMAT_MARKUP)
     {
        efl_text_markup_cursor_markup_insert(obj, cur, buf);
     }
   else // TEXT
     {
        efl_text_cursor_text_insert(obj, cur, buf);
     }
   free(buf);
}

static void
_dnd_enter_cb(void *data EINA_UNUSED,
              Evas_Object *obj)
{
   elm_object_focus_set(obj, EINA_TRUE);
}

static void
_dnd_leave_cb(void *data EINA_UNUSED,
              Evas_Object *obj)
{
   if (_elm_config->desktop_entry)
     elm_object_focus_set(obj, EINA_FALSE);
}

static void
_dnd_pos_cb(void *data EINA_UNUSED,
            Evas_Object *obj,
            Evas_Coord x,
            Evas_Coord y,
            Elm_Xdnd_Action action EINA_UNUSED)
{
   int pos;
   Evas_Coord ox, oy, ex, ey;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   evas_object_geometry_get(obj, &ox, &oy, NULL, NULL);
   evas_object_geometry_get(sd->entry_edje, &ex, &ey, NULL, NULL);
   x = x + ox - ex;
   y = y + oy - ey;

   edje_object_part_text_cursor_coord_set
      (sd->entry_edje, "efl.text", EDJE_CURSOR_USER, x, y);
   pos = edje_object_part_text_cursor_pos_get
      (sd->entry_edje, "efl.text", EDJE_CURSOR_USER);
   edje_object_part_text_cursor_pos_set(sd->entry_edje, "efl.text",
                                        EDJE_CURSOR_MAIN, pos);
}

static Eina_Bool
_dnd_drop_cb(void *data EINA_UNUSED,
              Evas_Object *obj,
              Elm_Selection_Data *drop)
{
   Eina_Bool rv;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   rv = edje_object_part_text_cursor_coord_set
       (sd->entry_edje, "efl.text", EDJE_CURSOR_MAIN, drop->x, drop->y);

   if (!rv) WRN("Warning: Failed to position cursor: paste anyway");

   //rv = _selection_data_cb(NULL, obj, drop);

   return rv;
}

static Elm_Sel_Format
_get_drop_format(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   if ((sd->editable) && (!sd->single_line) && (!sd->password) && (!sd->disabled))
     return EFL_SELECTION_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE;
   return EFL_SELECTION_FORMAT_MARKUP;
}

/* we can't reuse layout's here, because it's on entry_edje only */
EOLIAN static Eina_Bool
_efl_ui_text_efl_ui_widget_on_disabled_update(Eo *obj, Efl_Ui_Text_Data *sd, Eina_Bool disabled)
{
   const char *emission ;

   elm_drop_target_del(obj, sd->drop_format,
                       _dnd_enter_cb, NULL,
                       _dnd_leave_cb, NULL,
                       _dnd_pos_cb, NULL,
                       _dnd_drop_cb, NULL);

   emission = disabled ? "efl,state,disabled" : "efl,state,enabled";
   edje_object_signal_emit(sd->entry_edje, emission, "efl");
   if (sd->scroll)
     {
        edje_object_signal_emit(sd->scr_edje, emission, "efl");
        //elm_interface_scrollable_freeze_set(obj, disabled);
     }
   sd->disabled = disabled;

   if (!disabled)
     {
        sd->drop_format = _get_drop_format(obj);
        elm_drop_target_add(obj, sd->drop_format,
                            _dnd_enter_cb, NULL,
                            _dnd_leave_cb, NULL,
                            _dnd_pos_cb, NULL,
                            _dnd_drop_cb, NULL);
     }

   return EINA_TRUE;
}

/* we can't issue the layout's theming code here, cause it assumes an
 * unique edje object, always */
EOLIAN static Efl_Ui_Theme_Apply_Result
_efl_ui_text_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Text_Data *sd)
{
   Efl_Ui_Theme_Apply_Result theme_apply;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   // Note: We are skipping elm_layout here! This is by design.
   // This assumes the following inheritance: my_class -> layout -> widget ...
   theme_apply = efl_ui_widget_theme_apply(efl_cast(obj, EFL_UI_WIDGET_CLASS));
   if (!theme_apply) return EFL_UI_THEME_APPLY_RESULT_FAIL;

   efl_event_freeze(obj);

   edje_object_mirrored_set
     (wd->resize_obj, efl_ui_mirrored_get(obj));

   edje_object_scale_set
     (wd->resize_obj,
     efl_gfx_entity_scale_get(obj) * elm_config_scale_get());

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   elm_widget_element_update(obj, sd->entry_edje,
                                   elm_widget_theme_element_get(obj));

   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(sd->entry_edje, "efl,state,disabled", "efl");

   edje_object_part_text_input_panel_layout_set
     (sd->entry_edje, "efl.text", (Edje_Input_Panel_Layout)sd->input_panel_layout);
   edje_object_part_text_input_panel_layout_variation_set
     (sd->entry_edje, "efl.text", sd->input_panel_layout_variation);
   edje_object_part_text_autocapital_type_set
     (sd->entry_edje, "efl.text", (Edje_Text_Autocapital_Type)sd->autocapital_type);
   edje_object_part_text_prediction_allow_set
     (sd->entry_edje, "efl.text", sd->prediction_allow);
   edje_object_part_text_input_hint_set
     (sd->entry_edje, "efl.text", (Edje_Input_Hints)sd->input_hints);
   edje_object_part_text_input_panel_enabled_set
     (sd->entry_edje, "efl.text", sd->input_panel_enable);
   edje_object_part_text_input_panel_imdata_set
     (sd->entry_edje, "efl.text", sd->input_panel_imdata,
     sd->input_panel_imdata_len);
   edje_object_part_text_input_panel_return_key_type_set
     (sd->entry_edje, "efl.text", (Edje_Input_Panel_Return_Key_Type)sd->input_panel_return_key_type);
   edje_object_part_text_input_panel_return_key_disabled_set
     (sd->entry_edje, "efl.text", sd->input_panel_return_key_disabled);
   edje_object_part_text_input_panel_show_on_demand_set
     (sd->entry_edje, "efl.text", sd->input_panel_show_on_demand);

   // elm_entry_cursor_pos_set -> cursor,changed -> widget_show_region_set
   // -> smart_objects_calculate will call all smart calculate functions,
   // and one of them can delete elm_entry.
   evas_object_ref(obj);

   if (efl_ui_focus_object_focus_get(obj))
     {
        edje_object_signal_emit(sd->entry_edje, "efl,action,focus", "efl");
        if (sd->scroll)
          edje_object_signal_emit(sd->scr_edje, "efl,action,focus", "efl");
     }

   edje_object_message_signal_process(sd->entry_edje);

   Evas_Object* clip = evas_object_clip_get(sd->entry_edje);
   evas_object_clip_set(sd->hit_rect, clip);

   if (sd->start_handler)
     {
        elm_widget_element_update(obj, sd->start_handler, PART_NAME_HANDLER_START);
        elm_widget_element_update(obj, sd->end_handler, PART_NAME_HANDLER_END);
     }

   sd->has_text = !sd->has_text;
   _efl_ui_text_guide_update(obj, !sd->has_text);
   efl_event_thaw(obj);

   efl_event_callback_call(obj, EFL_UI_LAYOUT_EVENT_THEME_CHANGED, NULL);

   evas_object_unref(obj);

   return theme_apply;
}

static void
_cursor_geometry_recalc(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   Evas_Coord x, y, w, h;
   Evas_Coord x2, y2, w2, h2;
   Evas_Coord cx, cy, cw, ch;

   if (!sd->editable) return;

   cx = cy = cw = ch = 0;
   x2 = y2 = w2 = h2 = 0;
   x = y = w = h = 0;

   Efl_Text_Cursor_Cursor *main_cur =
      efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN);

   efl_text_cursor_geometry_get(obj, main_cur,
         EFL_TEXT_CURSOR_TYPE_BEFORE,
         &cx, &cy, &cw, &ch, NULL, NULL, NULL, NULL);
   edje_object_size_min_restricted_calc(sd->cursor, &cw, NULL, cw, 0);
   if (cw < 1) cw = 1;
   if (ch < 1) ch = 1;
   edje_object_size_min_restricted_calc(sd->cursor, &cw, NULL, cw, 0);
   evas_object_geometry_get(sd->entry_edje, &x, &y, &w, &h);
   evas_object_geometry_get(
         sd->text_obj,
         &x2, &y2, &w2, &h2);

   efl_ui_scrollable_scroll(sd->scroller, EINA_RECT(cx, cy, cw, ch), EINA_FALSE);

}

#define SIZE2D_EQ(X, Y) (((X).w == (Y).w) && ((X).h == (Y).h))

EOLIAN static void
_efl_ui_text_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Text_Data *sd)
{
   Eina_Size2D min = EINA_SIZE2D(0, 0);
   Eina_Size2D edmin = EINA_SIZE2D(0, 0);
   Eina_Size2D sz = EINA_SIZE2D(0, 0);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   sz = efl_gfx_entity_size_get(obj);
   if (!sd->calc_force && SIZE2D_EQ(sd->last.layout, sz) &&
         !sd->text_changed)
     {
        return;
     }

   sd->single_line = !efl_text_multiline_get(sd->text_obj);

   sd->calc_force = EINA_FALSE;
   sd->last.layout.w = sz.w;
   sd->last.layout.h = sz.h;
   sd->text_changed = EINA_FALSE;

   if (sd->scroll)
     {
        if (sd->single_line)
          {
             efl_ui_internal_text_scroller_mode_set(sd->scroller,
                   EFL_UI_TEXT_SCROLLER_MODE_SINGLELINE);
          }
        else
          {
             efl_ui_internal_text_scroller_mode_set(sd->scroller,
                   EFL_UI_TEXT_SCROLLER_MODE_MULTILINE);

          }

        elm_layout_sizing_eval(sd->scroller);
        min = efl_gfx_size_hint_min_get(sd->scroller);
        if (sd->single_line)
          {
             efl_ui_internal_text_scroller_mode_set(sd->scroller,
                   EFL_UI_TEXT_SCROLLER_MODE_SINGLELINE);
             edje_object_size_min_calc(wd->resize_obj, &edmin.w, &edmin.h);
             min.w = edmin.w;
             min.h = edmin.h;
             efl_gfx_size_hint_restricted_min_set(obj, min);
          }

        sd->cursor_update = EINA_TRUE;
        _decoration_defer_all(obj);
     }
   else
     {
        edje_object_size_min_calc(wd->resize_obj, &edmin.w, &edmin.h);
        efl_canvas_text_size_formatted_get(sd->text_obj, &min.w, &min.h);
        min.w += edmin.w;
        min.h += edmin.h;
        efl_gfx_size_hint_restricted_min_set(obj, min);
     }
}

static void
_return_key_enabled_check(Evas_Object *obj)
{
   Eina_Bool return_key_disabled = EINA_FALSE;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (!sd->auto_return_key) return;

   if (efl_canvas_text_is_empty_get(obj) == EINA_TRUE)
     return_key_disabled = EINA_TRUE;

   efl_ui_text_input_panel_return_key_disabled_set(obj, return_key_disabled);
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_ui_focus_object_on_focus_update(Eo *obj, Efl_Ui_Text_Data *sd)
{
   Evas_Object *top;
   Eina_Bool top_is_win = EINA_FALSE;

   if (!sd->editable) return EINA_FALSE;

   top = elm_widget_top_get(obj);
   if (top && efl_isa(top, EFL_UI_WIN_CLASS))
     top_is_win = EINA_TRUE;

   if (efl_ui_focus_object_focus_get(obj))
     {
        evas_object_focus_set(sd->text_obj, EINA_TRUE);

        _edje_signal_emit(sd, "efl,action,focus", "efl");
        if (sd->scroll)
          edje_object_signal_emit(sd->scr_edje, "efl,action,focus", "efl");

        if (top && top_is_win && sd->input_panel_enable && !sd->input_panel_show_on_demand)
          elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_ON);
        if (_elm_config->atspi_mode)
          efl_access_state_changed_signal_emit(obj, EFL_ACCESS_STATE_FOCUSED, EINA_TRUE);
        _return_key_enabled_check(obj);
        _validate(obj);
     }
   else
     {
        Eo *sw = sd->text_obj;

        _edje_signal_emit(sd, "efl,action,unfocus", "efl");
        if (sd->scroll)
          edje_object_signal_emit(sd->scr_edje, "efl,action,unfocus", "efl");
        evas_object_focus_set(sw, EINA_FALSE);

        if (top && top_is_win && sd->input_panel_enable)
          elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_OFF);
        if (_elm_config->atspi_mode)
          efl_access_state_changed_signal_emit(obj, EFL_ACCESS_STATE_FOCUSED, EINA_FALSE);

        if (_elm_config->selection_clear_enable)
          {
             if ((sd->have_selection) && (!sd->hoversel))
               {
                  sd->sel_mode = EINA_FALSE;
                  elm_widget_scroll_hold_pop(obj);
                  edje_object_part_text_select_allow_set(sd->entry_edje, "efl.text", EINA_FALSE);
                  edje_object_signal_emit(sd->entry_edje, "efl,state,select,off", "efl");
                  edje_object_part_text_select_none(sd->entry_edje, "efl.text");
               }
          }
        edje_object_signal_emit(sd->scr_edje, "validation,default", "efl");
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Rect
_efl_ui_text_efl_ui_widget_interest_region_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   Evas_Coord edje_x, edje_y, elm_x, elm_y;
   Eina_Rect r = {};

   efl_text_cursor_geometry_get(obj,
         efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN),
         EFL_TEXT_CURSOR_TYPE_BEFORE,
         &r.x, &r.y, &r.w, &r.h,
         NULL, NULL, NULL, NULL);

   if (sd->single_line)
     {
        evas_object_geometry_get(sd->entry_edje, NULL, NULL, NULL, &r.h);
        r.y = 0;
     }

   evas_object_geometry_get(sd->entry_edje, &edje_x, &edje_y, NULL, NULL);
   evas_object_geometry_get(obj, &elm_x, &elm_y, NULL, NULL);

   r.x += edje_x - elm_x;
   r.y += edje_y - elm_y;
   if (r.w < 1) r.w = 1;
   if (r.h < 1) r.h = 1;

   return r;
}

static void
_hoversel_position(Evas_Object *obj)
{
   Evas_Coord cx, cy, cw, ch, x, y, mw, mh, w, h;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   cx = cy = 0;
   cw = ch = 1;
   evas_object_geometry_get(sd->entry_edje, &x, &y, &w, &h);
   if (sd->use_down)
     {
        cx = sd->downx - x;
        cy = sd->downy - y;
        cw = 1;
        ch = 1;
     }
   else
     edje_object_part_text_cursor_geometry_get
       (sd->entry_edje, "efl.text", &cx, &cy, &cw, &ch);

   evas_object_size_hint_min_get(sd->hoversel, &mw, &mh);
   if (cx + mw > w)
     cx = w - mw;
   if (cy + mh > h)
     cy = h - mh;
   evas_object_geometry_set(sd->hoversel, x + cx, y + cy, mw, mh);
}

static void
_hover_del_job(void *data)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   ELM_SAFE_FREE(sd->hoversel, evas_object_del);
   sd->hov_deljob = NULL;
}

static void
_hover_dismissed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   sd->use_down = 0;
   if (sd->hoversel) evas_object_hide(sd->hoversel);
   if (sd->sel_mode)
     {
        if (!_elm_config->desktop_entry)
          {
             if (!sd->password)
               edje_object_part_text_select_allow_set
                 (sd->entry_edje, "efl.text", EINA_TRUE);
          }
     }
   elm_widget_scroll_freeze_pop(data);
   ecore_job_del(sd->hov_deljob);
   sd->hov_deljob = ecore_job_add(_hover_del_job, data);
}

static void
_hover_selected_cb(void *data,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   if (!sd->sel_allow) return;

   sd->sel_mode = EINA_TRUE;
   edje_object_part_text_select_none(sd->entry_edje, "efl.text");

   if (!_elm_config->desktop_entry)
     {
        if (!sd->password)
          edje_object_part_text_select_allow_set
            (sd->entry_edje, "efl.text", EINA_TRUE);
     }
   edje_object_signal_emit(sd->entry_edje, "efl,state,select,on", "efl");

   if (!_elm_config->desktop_entry)
     elm_widget_scroll_hold_push(data);
}

static void
_paste_cb(Eo *obj)
{
   Efl_Selection_Format formats = EFL_SELECTION_FORMAT_TEXT |
      EFL_SELECTION_FORMAT_MARKUP;

   efl_selection_get(obj, EFL_SELECTION_TYPE_CLIPBOARD, formats,
         NULL, _selection_data_cb, NULL, 1);

}

static void
_hoversel_item_paste_cb(void *data,
          Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   _paste_cb(data);
}

static void
_selection_clear(void *data, Elm_Sel_Type selection)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   if (!sd->have_selection) return;
   if ((selection == EFL_SELECTION_TYPE_CLIPBOARD) ||
       (selection == EFL_SELECTION_TYPE_PRIMARY))
     {
        _efl_ui_text_select_none(data, sd);
     }
   _selection_defer(data, sd);
}

static Eina_Value
_selection_lost_cb(void *data, const Eina_Value value)
{
   Selection_Loss_Data *sdata = data;
   _selection_clear(sdata->obj, sdata->stype);
   EFL_UI_TEXT_DATA_GET(sdata->obj, sd);
   switch (sdata->stype)
     {
      case EFL_SELECTION_TYPE_CLIPBOARD:
         sd->sel_future.clipboard = NULL;
         break;
      case EFL_SELECTION_TYPE_PRIMARY:
      default:
         sd->sel_future.primary = NULL;
         break;
     }

   return value;
}

static void
_selection_store(Efl_Selection_Type seltype,
                 Evas_Object *obj)
{
   char *sel;
   Efl_Text_Cursor_Cursor *start, *end;
   Efl_Selection_Format selformat = EFL_SELECTION_FORMAT_MARKUP;
   Eina_Slice slice;
   Selection_Loss_Data *ldata;
   Eina_Future *f;

   ldata = calloc(1, sizeof(Selection_Loss_Data));
   if (!ldata) return;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   efl_text_interactive_selection_cursors_get(obj, &start, &end);
   sel = efl_text_markup_range_get(obj, start, end);

   if ((!sel) || (!sel[0])) return;  /* avoid deleting our own selection */

   slice.len = strlen(sel);
   slice.mem = sel;


   switch (seltype)
     {
      case EFL_SELECTION_TYPE_CLIPBOARD:
         if (sd->sel_future.clipboard)
           {
              eina_future_cancel(sd->sel_future.clipboard);
           }

         f = sd->sel_future.clipboard = efl_selection_set(obj, seltype,
               selformat, slice, 1);
         break;

      case EFL_SELECTION_TYPE_PRIMARY:
      default:
         if (sd->sel_future.primary)
           {
              eina_future_cancel(sd->sel_future.primary);
           }

         f = sd->sel_future.primary = efl_selection_set(obj, seltype,
               selformat, slice, 1);
         break;
     }

   ldata->obj = obj;
   eina_future_then_easy(f, _selection_lost_cb, NULL, NULL, EINA_VALUE_TYPE_UINT, ldata);

   //if (seltype == EFL_SELECTION_TYPE_CLIPBOARD)
   //  eina_stringshare_replace(&sd->cut_sel, sel);

   free(sel);
}

static void
_cut_cb(Eo *obj)
{
   Efl_Text_Cursor_Cursor *start, *end;
   EFL_UI_TEXT_DATA_GET(obj, sd);

   efl_event_callback_call(obj, EFL_UI_EVENT_SELECTION_CUT, NULL);
   /* Store it */
   sd->sel_mode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set
       (sd->entry_edje, "efl.text", EINA_FALSE);
   edje_object_signal_emit(sd->entry_edje, "efl,state,select,off", "efl");

   if (!_elm_config->desktop_entry)
     elm_widget_scroll_hold_pop(obj);

   _selection_store(EFL_SELECTION_TYPE_CLIPBOARD, obj);
   efl_text_interactive_selection_cursors_get(obj, &start, &end);
   efl_canvas_text_range_delete(obj, start, end);
}

static void
_hoversel_item_cut_cb(void *data,
        Evas_Object *obj EINA_UNUSED,
        void *event_info EINA_UNUSED)
{
   _cut_cb(data);
}

static void
_copy_cb(Eo *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   efl_event_callback_call(obj, EFL_UI_EVENT_SELECTION_COPY, NULL);
   sd->sel_mode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     {
        edje_object_part_text_select_allow_set
          (sd->entry_edje, "efl.text", EINA_FALSE);
        edje_object_signal_emit(sd->entry_edje, "efl,state,select,off", "efl");
        elm_widget_scroll_hold_pop(obj);
     }
   _selection_store(EFL_SELECTION_TYPE_CLIPBOARD, obj);
}

static void
_hoversel_item_copy_cb(void *data,
         Evas_Object *obj EINA_UNUSED,
         void *event_info EINA_UNUSED)
{
   _copy_cb(data);
}

static void
_hover_cancel_cb(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   sd->sel_mode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set
       (sd->entry_edje, "efl.text", EINA_FALSE);
   edje_object_signal_emit(sd->entry_edje, "efl,state,select,off", "efl");
   if (!_elm_config->desktop_entry)
     elm_widget_scroll_hold_pop(data);
   edje_object_part_text_select_none(sd->entry_edje, "efl.text");
}

static void
_hover_item_clicked_cb(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   Elm_Entry_Context_Menu_Item *it = data;
   if (!it) return;

   if (it->func) it->func(it->data, it->obj, it);
}

static void
_menu_call(Evas_Object *obj)
{
   Evas_Object *top;
   const Eina_List *l;
   const Elm_Entry_Context_Menu_Item *it;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (sd->anchor_hover.hover) return;

   efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CONTEXT_OPEN, NULL);

   if ((sd->api) && (sd->api->obj_longpress))
     {
        sd->api->obj_longpress(obj);
     }
   else if (sd->context_menu)
     {
        const char *context_menu_orientation;
        Eina_Bool ownersel;

        ownersel = elm_selection_selection_has_owner(obj);
        if (!sd->items)
          {
             /* prevent stupid blank hoversel */
             if (sd->have_selection && sd->password) return;
             if (_elm_config->desktop_entry && (!sd->have_selection) && ((!sd->editable) || (!ownersel)))
               return;
          }
        if (sd->hoversel) evas_object_del(sd->hoversel);
        else elm_widget_scroll_freeze_push(obj);

        sd->hoversel = elm_hoversel_add(obj);
        context_menu_orientation = edje_object_data_get
            (sd->entry_edje, "context_menu_orientation");

        if ((context_menu_orientation) &&
            (!strcmp(context_menu_orientation, "horizontal")))
          elm_hoversel_horizontal_set(sd->hoversel, EINA_TRUE);

        elm_object_style_set(sd->hoversel, "entry");
        elm_widget_sub_object_add(obj, sd->hoversel);
        elm_object_text_set(sd->hoversel, "Text");
        top = elm_widget_top_get(obj);

        if (top) elm_hoversel_hover_parent_set(sd->hoversel, top);

        efl_event_callback_add
          (sd->hoversel, ELM_HOVERSEL_EVENT_DISMISSED, _hover_dismissed_cb, obj);
        if (sd->have_selection)
          {
             if (!sd->password)
               {
                  if (sd->editable)
                    elm_hoversel_item_add
                       (sd->hoversel, E_("Cut"), NULL, ELM_ICON_NONE,
                        _hoversel_item_cut_cb, obj);
                  elm_hoversel_item_add
                     (sd->hoversel, E_("Copy"), NULL, ELM_ICON_NONE,
                      _hoversel_item_copy_cb, obj);
                  if (sd->editable && ownersel)
                    elm_hoversel_item_add
                       (sd->hoversel, E_("Paste"), NULL, ELM_ICON_NONE,
                        _hoversel_item_paste_cb, obj);
                  elm_hoversel_item_add
                    (sd->hoversel, E_("Cancel"), NULL, ELM_ICON_NONE,
                    _hover_cancel_cb, obj);
               }
          }
        else
          {
             if (!sd->sel_mode)
               {
                  if (sd->sel_allow && !_elm_config->desktop_entry)
                    {
                       if (!sd->password)
                         elm_hoversel_item_add
                           (sd->hoversel, E_("Select"), NULL, ELM_ICON_NONE,
                           _hover_selected_cb, obj);
                    }
                  if (ownersel)
                    {
                       if (sd->editable)
                         elm_hoversel_item_add
                           (sd->hoversel, E_("Paste"), NULL, ELM_ICON_NONE,
                           _hoversel_item_paste_cb, obj);
                    }
               }
             else
               elm_hoversel_item_add
                  (sd->hoversel, E_("Cancel"), NULL, ELM_ICON_NONE,
                   _hover_cancel_cb, obj);
          }

        EINA_LIST_FOREACH(sd->items, l, it)
          {
             elm_hoversel_item_add(sd->hoversel, it->label, it->icon_file,
                                   it->icon_type, _hover_item_clicked_cb, it);
          }

        if (sd->hoversel)
          {
             _hoversel_position(obj);
             evas_object_show(sd->hoversel);
             elm_hoversel_hover_begin(sd->hoversel);
          }

        if (!_elm_config->desktop_entry)
          {
             edje_object_part_text_select_allow_set
               (sd->entry_edje, "efl.text", EINA_FALSE);
             edje_object_part_text_select_abort(sd->entry_edje, "efl.text");
          }
     }
}

static Eina_Bool
_long_press_cb(void *data)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   /* Context menu will not appear if context menu disabled is set
    * as false on a long press callback */
   if (!_elm_config->context_menu_disabled &&
            (!_elm_config->desktop_entry))
     _menu_call(data);

   sd->long_pressed = EINA_TRUE;

   sd->longpress_timer = NULL;
   efl_event_callback_call(data, EFL_UI_EVENT_LONGPRESSED, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_key_down_cb(void *data,
               Evas *evas EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Eina_Bool on_hold = EINA_FALSE;

   /* First check if context menu disabled is false or not, and
    * then check for key id */
   if ((!_elm_config->context_menu_disabled) && !strcmp(ev->key, "Menu"))
     {
        _menu_call(data);
        on_hold = EINA_TRUE;
     }
   else
     {
#if defined(__APPLE__) && defined(__MACH__)
        Eina_Bool control = evas_key_modifier_is_set(ev->modifiers, "Super");
#else
        Eina_Bool control = evas_key_modifier_is_set(ev->modifiers, "Control");
#endif

        /* Ctrl operations */
        if (control)
          {
             if (!strncmp(ev->key, "c", 1))
               {
                  _copy_cb(data);
                  on_hold = EINA_TRUE;
               }
             else if (!strncmp(ev->key, "x", 1))
               {
                  _cut_cb(data);
                  on_hold = EINA_TRUE;
               }
             else if (!strncmp(ev->key, "v", 1))
               {
                  _paste_cb(data);
                  on_hold = EINA_TRUE;
               }
          }
     }

   if (on_hold) ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
}

static void
_mouse_down_cb(void *data,
               Evas *evas EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;

   EFL_UI_TEXT_DATA_GET(data, sd);

   if (sd->disabled) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   sd->downx = ev->canvas.x;
   sd->downy = ev->canvas.y;
   sd->long_pressed = EINA_FALSE;

    if (ev->button == 1)
      {
         ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
         sd->longpress_timer = ecore_timer_add
           (_elm_config->longpress_timeout, _long_press_cb, data);
      }
    /* If right button is pressed and context menu disabled is true,
     * then only context menu will appear */
   else if (ev->button == 3 && (!_elm_config->context_menu_disabled))
     {
        if (_elm_config->desktop_entry)
          {
             sd->use_down = 1;
             _menu_call(data);
          }
     }
}

static void
_mouse_up_cb(void *data,
             Evas *evas EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Eina_Bool top_is_win = EINA_FALSE;
   Evas_Object *top;

   EFL_UI_TEXT_DATA_GET(data, sd);

   if (sd->disabled) return;
   if (ev->button == 1)
     {
        ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
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
             top = elm_widget_top_get(data);
             if (top)
               {
                  if (efl_isa(top, EFL_UI_WIN_CLASS))
                    top_is_win = EINA_TRUE;

                  if (top_is_win && sd->input_panel_enable && sd->input_panel_show_on_demand)
                    elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_ON);
               }
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
_mouse_move_cb(void *data,
               Evas *evas EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;

   EFL_UI_TEXT_DATA_GET(data, sd);

   if (sd->disabled) return;
   if (ev->buttons == 1)
     {
        if (sd->long_pressed)
          {
             Evas_Coord x, y;
             Eina_Bool rv;

             evas_object_geometry_get(sd->entry_edje, &x, &y, NULL, NULL);
             rv = edje_object_part_text_cursor_coord_set
               (sd->entry_edje, "efl.text", EDJE_CURSOR_USER,
               ev->cur.canvas.x - x, ev->cur.canvas.y - y);
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
             ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
          }
        else if (sd->longpress_timer)
          {
             Evas_Coord dx, dy;

             dx = sd->downx - ev->cur.canvas.x;
             dx *= dx;
             dy = sd->downy - ev->cur.canvas.y;
             dy *= dy;
             if ((dx + dy) >
                 ((_elm_config->finger_size / 2) *
                  (_elm_config->finger_size / 2)))
               {
                  ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
               }
          }
     }
   else if (sd->longpress_timer)
     {
        Evas_Coord dx, dy;

        dx = sd->downx - ev->cur.canvas.x;
        dx *= dx;
        dy = sd->downy - ev->cur.canvas.y;
        dy *= dy;
        if ((dx + dy) >
            ((_elm_config->finger_size / 2) *
             (_elm_config->finger_size / 2)))
          {
             ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
          }
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
             o = efl_canvas_text_factory_create(sd->item_factory, data, item);
          }
        else if (sd->item_fallback_factory)
          {
             o = efl_canvas_text_factory_create(sd->item_fallback_factory,
                   data, item);
          }
     }
   return o;
}

EOLIAN static void
_efl_ui_text_efl_layout_signal_signal_emit(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, const char *emission, const char *source)
{
   /* always pass to both edje objs */
   efl_layout_signal_emit(sd->entry_edje, emission, source);

   // FIXME: This should not be here!
   efl_layout_signal_process(sd->entry_edje, EINA_TRUE);

   if (sd->scr_edje)
     {
        efl_layout_signal_emit(sd->scr_edje, emission, source);
        efl_layout_signal_process(sd->scr_edje, EINA_TRUE); // FIXME
     }
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_layout_signal_signal_callback_add(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Eina_Bool ok;

   ok = efl_layout_signal_callback_add(sd->entry_edje, emission, source, func_cb, data);
   if (sd->scr_edje)
     ok = efl_layout_signal_callback_add(sd->scr_edje, emission, source, func_cb, data);

   return ok;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_layout_signal_signal_callback_del(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Eina_Bool ok;

   ok = efl_layout_signal_callback_del(sd->entry_edje, emission, source, func_cb, data);
   if (sd->scr_edje)
     ok = efl_layout_signal_callback_del(sd->scr_edje, emission, source, func_cb, data);

   return ok;
}

static void
_selection_handlers_offset_calc(Evas_Object *obj, Evas_Object *handler)
{
   Evas_Coord cx, cy, cw, ch;
   Evas_Coord hh;
   Eina_Position2D pos;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   pos = efl_gfx_entity_position_get(sd->text_obj);
   efl_text_cursor_geometry_get(obj,
         efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN),
         EFL_TEXT_CURSOR_TYPE_BEFORE,
         &cx, &cy, &cw, &ch,
         NULL, NULL, NULL, NULL);
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

   ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
   sd->long_pressed = EINA_FALSE;
}

static void
_start_handler_mouse_down_cb(void *data,
                             Evas *e EINA_UNUSED,
                             Evas_Object *obj EINA_UNUSED,
                             void *event_info EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   int start_pos, end_pos, pos;
   Efl_Text_Cursor_Cursor *sel_start, *sel_end;
   Efl_Text_Cursor_Cursor *main_cur;

   Eo *text_obj = sd->text_obj;

   sd->start_handler_down = EINA_TRUE;

   /* Get the cursors */
   efl_text_interactive_selection_cursors_get(text_obj, &sel_start, &sel_end);
   main_cur = efl_text_cursor_get(text_obj, EFL_TEXT_CURSOR_GET_MAIN);

   start_pos = efl_text_cursor_position_get(text_obj, sel_start);
   end_pos = efl_text_cursor_position_get(text_obj, sel_end);

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
   efl_text_cursor_position_set(text_obj, main_cur, pos);
   _selection_handlers_offset_calc(data, sd->start_handler);
}

static void
_start_handler_mouse_up_cb(void *data,
                           Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
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
_start_handler_mouse_move_cb(void *data,
                             Evas *e EINA_UNUSED,
                             Evas_Object *obj EINA_UNUSED,
                             void *event_info)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   if (!sd->start_handler_down) return;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord ex, ey;
   Evas_Coord cx, cy;
   int pos;

   evas_object_geometry_get(sd->entry_edje, &ex, &ey, NULL, NULL);
   cx = ev->cur.canvas.x - sd->ox - ex;
   cy = ev->cur.canvas.y - sd->oy - ey;
   if (cx <= 0) cx = 1;

   efl_text_cursor_coord_set(sd->text_obj, sd->sel_handler_cursor, cx, cy);
   pos = efl_text_cursor_position_get(sd->text_obj, sd->sel_handler_cursor);

   /* Set the main cursor. */
   efl_text_cursor_position_set(sd->text_obj,
         efl_text_cursor_get(sd->text_obj, EFL_TEXT_CURSOR_GET_MAIN), pos);

   ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
   sd->long_pressed = EINA_FALSE;
}

static void
_end_handler_mouse_down_cb(void *data,
                           Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   Efl_Text_Cursor_Cursor *sel_start, *sel_end;
   Efl_Text_Cursor_Cursor *main_cur;
   int pos, start_pos, end_pos;

   sd->end_handler_down = EINA_TRUE;

   Eo *text_obj = sd->text_obj;

   efl_text_interactive_selection_cursors_get(text_obj, &sel_start, &sel_end);
   main_cur = efl_text_cursor_get(text_obj, EFL_TEXT_CURSOR_GET_MAIN);

   start_pos = efl_text_cursor_position_get(text_obj, sel_start);
   end_pos = efl_text_cursor_position_get(text_obj, sel_end);

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

   efl_text_cursor_position_set(text_obj, main_cur, pos);
   _selection_handlers_offset_calc(data, sd->end_handler);
}

static void
_end_handler_mouse_up_cb(void *data,
                         Evas *e EINA_UNUSED,
                         Evas_Object *obj EINA_UNUSED,
                         void *event_info EINA_UNUSED)
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
_end_handler_mouse_move_cb(void *data,
                           Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   if (!sd->end_handler_down) return;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord ex, ey;
   Evas_Coord cx, cy;
   int pos;

   evas_object_geometry_get(sd->entry_edje, &ex, &ey, NULL, NULL);
   cx = ev->cur.canvas.x - sd->ox - ex;
   cy = ev->cur.canvas.y - sd->oy - ey;
   if (cx <= 0) cx = 1;

   efl_text_cursor_coord_set(sd->text_obj, sd->sel_handler_cursor, cx, cy);
   pos = efl_text_cursor_position_get(sd->text_obj, sd->sel_handler_cursor);
   /* Set the main cursor. */
   efl_text_cursor_position_set(sd->text_obj, efl_text_cursor_get(data, EFL_TEXT_CURSOR_GET_MAIN), pos);
   ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
   sd->long_pressed = EINA_FALSE;
}

static void
_create_selection_handlers(Evas_Object *obj, Efl_Ui_Text_Data *sd)
{
   Evas_Object *handle;

   handle = _decoration_create(obj, sd, PART_NAME_HANDLER_START, EINA_TRUE);
   evas_object_pass_events_set(handle, EINA_FALSE);
   sd->start_handler = handle;
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_DOWN,
                                  _start_handler_mouse_down_cb, obj);
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_MOVE,
                                  _start_handler_mouse_move_cb, obj);
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_UP,
                                  _start_handler_mouse_up_cb, obj);
   evas_object_show(handle);

   handle = _decoration_create(obj, sd, PART_NAME_HANDLER_END, EINA_TRUE);
   evas_object_pass_events_set(handle, EINA_FALSE);
   sd->end_handler = handle;
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_DOWN,
                                  _end_handler_mouse_down_cb, obj);
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_MOVE,
                                  _end_handler_mouse_move_cb, obj);
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_UP,
                                  _end_handler_mouse_up_cb, obj);
   evas_object_show(handle);
}

EOLIAN static void
_efl_ui_text_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Text_Data *sd, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
   efl_gfx_entity_position_set(sd->hit_rect, pos);

   if (sd->hoversel) _hoversel_position(obj);
}

EOLIAN static void
_efl_ui_text_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Text_Data *sd, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(sd->hit_rect, sz);
   _update_selection_handler(obj);

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
}

EOLIAN static void
_efl_ui_text_efl_gfx_entity_visible_set(Eo *obj, Efl_Ui_Text_Data *sd EINA_UNUSED, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), vis);
   if (vis) _update_selection_handler(obj);
}

EOLIAN static void
_efl_ui_text_efl_canvas_group_group_member_add(Eo *obj, Efl_Ui_Text_Data *sd, Evas_Object *member)
{
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

static void
_cb_added(void *data EINA_UNUSED, const Efl_Event *ev)
{
   const Efl_Callback_Array_Item_Full *event = ev->info;

   EFL_UI_TEXT_DATA_GET(ev->object, sd);
   // XXX: BUG - not walking the array until a NULL entry
   if (event->desc == EFL_UI_TEXT_EVENT_VALIDATE)
     sd->validators++;
}

static void
_cb_deleted(void *data EINA_UNUSED, const Efl_Event *ev)
{
   const Efl_Callback_Array_Item_Full *event = ev->info;

   EFL_UI_TEXT_DATA_GET(ev->object, sd);
   // XXX: BUG - not walking the array until a NULL entry
   if (event->desc == EFL_UI_TEXT_EVENT_VALIDATE)
     sd->validators--;
   return;

}

static void
_update_guide_text(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
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
_update_text_theme(Eo *obj, Efl_Ui_Text_Data *sd)
{
   const char *font_name;
   const char *font_size;
   const char *colorcode;

   int font_size_n;
   unsigned char r, g, b, a;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   // Main Text
   // font_set
   font_name = edje_object_data_get(wd->resize_obj, "font.name");
   font_size = edje_object_data_get(wd->resize_obj, "font.size");
   font_size_n = font_size ? atoi(font_size) : 0;
   efl_text_font_set(sd->text_obj, font_name, font_size_n);

   // color
   colorcode = edje_object_data_get(wd->resize_obj, "style.color");
   if (colorcode && _format_color_parse(colorcode, strlen(colorcode), &r, &g, &b, &a))
     {
        efl_text_normal_color_set(sd->text_obj, r, g, b, a);
     }

   // Guide Text
   font_name = edje_object_data_get(wd->resize_obj, "guide.font.name");
   font_size = edje_object_data_get(wd->resize_obj, "guide.font.size");
   font_size_n = font_size ? atoi(font_size) : 0;
   efl_text_font_set(sd->text_guide_obj, font_name, font_size_n);

   // color
   colorcode = edje_object_data_get(wd->resize_obj, "guide.style.color");
   if (colorcode && _format_color_parse(colorcode, strlen(colorcode), &r, &g, &b, &a))
     {
        efl_text_normal_color_set(sd->text_guide_obj, r, g, b, a);
     }
}

EOLIAN static Eo *
_efl_ui_text_efl_object_constructor(Eo *obj, Efl_Ui_Text_Data *sd)
{
   Eo *text_obj;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "text");
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   elm_widget_sub_object_parent_add(obj);

   text_obj = efl_add(EFL_UI_INTERNAL_TEXT_INTERACTIVE_CLASS, obj);
   sd->text_obj = text_obj;
   sd->text_guide_obj = efl_add(EFL_CANVAS_TEXT_CLASS, obj);
   sd->text_table = efl_add(EFL_UI_TABLE_CLASS, obj);
   efl_composite_attach(obj, text_obj);

   sd->entry_edje = wd->resize_obj;
   sd->cnp_mode = EFL_SELECTION_FORMAT_TEXT;
   sd->line_wrap = ELM_WRAP_WORD;
   sd->context_menu = EINA_TRUE;
   sd->auto_save = EINA_TRUE;
   sd->editable = EINA_TRUE;
   sd->sel_allow = EINA_TRUE;
   sd->drop_format = EFL_SELECTION_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE;
   sd->last.scroll = EINA_SIZE2D(0, 0);
   sd->sel_handler_disabled = EINA_TRUE;

   return obj;
}

EOLIAN static Eo *
_efl_ui_text_efl_object_finalize(Eo *obj,
                                 Efl_Ui_Text_Data *sd)
{
   obj = efl_finalize(efl_super(obj, MY_CLASS));

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   elm_drop_target_add(obj, sd->drop_format,
                       _dnd_enter_cb, NULL,
                       _dnd_leave_cb, NULL,
                       _dnd_pos_cb, NULL,
                       _dnd_drop_cb, NULL);

   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)))
     CRI("Failed tp set layout!");

   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_ENTRY);
   efl_event_callback_add(obj, EFL_EVENT_CALLBACK_ADD, _cb_added, NULL);
   efl_event_callback_add(obj, EFL_EVENT_CALLBACK_DEL, _cb_deleted, NULL);

   //TODO: complete the usage of the text theme
   _update_text_theme(obj, sd);
   //efl_text_font_set(sd->text_obj, "Sans", 12);
   sd->single_line = !efl_text_multiline_get(sd->text_obj);

   efl_text_set(sd->text_obj, "");
   efl_pack_table(sd->text_table, sd->text_obj, 0, 0, 1, 1);
   efl_pack_table(sd->text_table, sd->text_guide_obj, 0, 0, 1, 1);

   //edje_object_part_swallow(sd->entry_edje, "efl.text", sd->text_obj);
   //edje_object_part_swallow(sd->entry_edje, "efl.text_guide", sd->text_guide_obj);
   edje_object_part_swallow(sd->entry_edje, "efl.text", sd->text_table);

   _update_guide_text(obj, sd);

   sd->item_fallback_factory = efl_add(EFL_UI_TEXT_FACTORY_FALLBACK_CLASS, obj);

   evas_object_size_hint_weight_set
      (sd->entry_edje, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
      (sd->entry_edje, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_event_callback_add(sd->text_obj, EFL_UI_TEXT_EVENT_CHANGED_USER,
         _efl_ui_text_changed_user_cb, obj);
   efl_event_callback_add(sd->text_obj, EFL_CANVAS_TEXT_EVENT_CHANGED,
         _efl_ui_text_changed_cb, obj);
   efl_event_callback_add(sd->text_obj, EFL_TEXT_INTERACTIVE_EVENT_SELECTION_CHANGED,
         _efl_ui_text_selection_changed_cb, obj);
   efl_event_callback_add(sd->text_obj, EFL_CANVAS_TEXT_EVENT_CURSOR_CHANGED,
         _efl_ui_text_cursor_changed_cb, obj);
   efl_event_callback_add(sd->text_obj, EFL_GFX_ENTITY_EVENT_MOVE,
         _text_position_changed_cb, obj);
   evas_object_event_callback_add(sd->entry_edje, EVAS_CALLBACK_MOVE,
         _efl_ui_text_move_cb, obj);

   evas_object_event_callback_add
     (sd->entry_edje, EVAS_CALLBACK_KEY_DOWN, _key_down_cb, obj);
   evas_object_event_callback_add
     (sd->entry_edje, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, obj);
   evas_object_event_callback_add
     (sd->entry_edje, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, obj);
   evas_object_event_callback_add
     (sd->entry_edje, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, obj);

   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_RESIZE,
         _text_size_changed_cb, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   efl_ui_text_input_panel_layout_set(obj, ELM_INPUT_PANEL_LAYOUT_NORMAL);
   efl_ui_text_input_panel_enabled_set(obj, EINA_TRUE);
   efl_ui_text_prediction_allow_set(obj, EINA_TRUE);
   efl_ui_text_input_hint_set(obj, ELM_INPUT_HINT_AUTO_COMPLETE);

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   _create_text_cursors(obj, sd);

   sd->calc_force = EINA_TRUE;
   elm_layout_sizing_eval(obj);

   return obj;

}

EOLIAN static void
_efl_ui_text_efl_object_destructor(Eo *obj, Efl_Ui_Text_Data *sd)
{
   Elm_Entry_Context_Menu_Item *it;
   Elm_Entry_Markup_Filter *tf;

   if (sd->delay_write)
     {
        ELM_SAFE_FREE(sd->delay_write, ecore_timer_del);
        if (sd->auto_save) _save_do(obj);
     }

   efl_event_freeze(obj);

   eina_stringshare_del(sd->file);

   ecore_job_del(sd->hov_deljob);
   if ((sd->api) && (sd->api->obj_unhook))
     sd->api->obj_unhook(obj);  // module - unhook

   evas_object_del(sd->mgf_proxy);
   evas_object_del(sd->mgf_bg);
   evas_object_del(sd->mgf_clip);

   entries = eina_list_remove(entries, obj);
   eina_stringshare_del(sd->cut_sel);
   eina_stringshare_del(sd->text);
   ecore_job_del(sd->deferred_decoration_job);
   if (sd->append_text_idler)
     {
        ecore_idler_del(sd->append_text_idler);
        ELM_SAFE_FREE(sd->append_text_left, free);
        sd->append_text_idler = NULL;
     }
   ecore_timer_del(sd->longpress_timer);
   EINA_LIST_FREE(sd->items, it)
     {
        eina_stringshare_del(it->label);
        eina_stringshare_del(it->icon_file);
        eina_stringshare_del(it->icon_group);
        free(it);
     }
   EINA_LIST_FREE(sd->markup_filters, tf)
     {
        _filter_free(tf);
     }
   ELM_SAFE_FREE(sd->delay_write, ecore_timer_del);
   free(sd->input_panel_imdata);
   eina_stringshare_del(sd->anchor_hover.hover_style);

   efl_event_thaw(obj);

   if (sd->start_handler)
     {
        evas_object_del(sd->start_handler);
        evas_object_del(sd->end_handler);
     }

   _anchors_free(sd);
   _clear_text_selection(sd);

   ecore_job_del(sd->deferred_decoration_job);
   sd->deferred_decoration_job = NULL;

   if (sd->item_factory) efl_unref(sd->item_factory);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_text_password_mode_set(Eo *obj, Efl_Ui_Text_Data *sd, Eina_Bool password)
{
   password = !!password;

   if (sd->password == password) return;
   sd->password = password;

   elm_drop_target_del(obj, sd->drop_format,
                       _dnd_enter_cb, NULL,
                       _dnd_leave_cb, NULL,
                       _dnd_pos_cb, NULL,
                       _dnd_drop_cb, NULL);
   if (password)
     {
        sd->single_line = EINA_TRUE;
        sd->line_wrap = ELM_WRAP_NONE;
        efl_ui_text_input_hint_set(obj, ((sd->input_hints & ~ELM_INPUT_HINT_AUTO_COMPLETE) | ELM_INPUT_HINT_SENSITIVE_DATA));
        efl_access_object_role_set(obj, EFL_ACCESS_ROLE_PASSWORD_TEXT);
     }
   else
     {
        sd->drop_format = _get_drop_format(obj);
        elm_drop_target_add(obj, sd->drop_format,
                            _dnd_enter_cb, NULL,
                            _dnd_leave_cb, NULL,
                            _dnd_pos_cb, NULL,
                            _dnd_drop_cb, NULL);

        efl_ui_text_input_hint_set(obj, ((sd->input_hints | ELM_INPUT_HINT_AUTO_COMPLETE) & ~ELM_INPUT_HINT_SENSITIVE_DATA));
        efl_access_object_role_set(obj, EFL_ACCESS_ROLE_ENTRY);
     }

   efl_ui_widget_theme_apply(obj);
}

EOLIAN static Eina_Bool
_efl_ui_text_password_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->password;
}

static void
_efl_ui_text_calc_force(Eo *obj, Efl_Ui_Text_Data *sd)
{
   sd->calc_force = EINA_TRUE;
   edje_object_calc_force(sd->entry_edje);
   elm_layout_sizing_eval(obj);
}

static const char*
_efl_ui_text_selection_get(const Eo *obj, Efl_Ui_Text_Data *sd)
{
   Efl_Text_Cursor_Cursor *start_obj, *end_obj;

   if ((sd->password)) return NULL;

   efl_text_interactive_selection_cursors_get(obj, &start_obj, &end_obj);
   return efl_canvas_text_range_text_get(obj, start_obj, end_obj);
}

EOLIAN static void
_efl_ui_text_selection_handler_disabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Eina_Bool disabled)
{
   if (sd->sel_handler_disabled == disabled) return;
   sd->sel_handler_disabled = disabled;
}

EOLIAN static Eina_Bool
_efl_ui_text_selection_handler_disabled_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->sel_handler_disabled;
}

static void
_efl_ui_text_entry_insert(Eo *obj, Efl_Ui_Text_Data *sd, const char *entry)
{
   Efl_Text_Cursor_Cursor *cur_obj = efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN);
   efl_text_cursor_text_insert(obj, cur_obj, entry);
   sd->text_changed = EINA_TRUE;
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_text_efl_text_interactive_editable_set(Eo *obj, Efl_Ui_Text_Data *sd, Eina_Bool editable)
{
   efl_text_interactive_editable_set(efl_super(obj, MY_CLASS), editable);
   if (sd->editable == editable) return;
   sd->editable = editable;
   efl_ui_widget_theme_apply(obj);

   elm_drop_target_del(obj, sd->drop_format,
                       _dnd_enter_cb, NULL,
                       _dnd_leave_cb, NULL,
                       _dnd_pos_cb, NULL,
                       _dnd_drop_cb, NULL);
   if (editable)
     {
        sd->drop_format = _get_drop_format(obj);
        elm_drop_target_add(obj, sd->drop_format,
                            _dnd_enter_cb, NULL,
                            _dnd_leave_cb, NULL,
                            _dnd_pos_cb, NULL,
                            _dnd_drop_cb, NULL);
        if (sd->cursor)
          {
             evas_object_show(sd->cursor);
             evas_object_show(sd->cursor_bidi);
          }
     }
   if (!editable && sd->cursor)
     {
        evas_object_hide(sd->cursor);
        evas_object_hide(sd->cursor_bidi);
     }
}

static void
_efl_ui_text_select_none(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   if ((sd->password)) return;
   if (sd->sel_mode)
     {
        sd->sel_mode = EINA_FALSE;
        if (!_elm_config->desktop_entry)
          edje_object_part_text_select_allow_set
            (sd->entry_edje, "efl.text", EINA_FALSE);
        edje_object_signal_emit(sd->entry_edje, "efl,state,select,off", "efl");
     }
   if (sd->have_selection)
     efl_event_callback_call(obj, EFL_UI_EVENT_SELECTION_CLEARED, NULL);

   sd->have_selection = EINA_FALSE;
   _edje_signal_emit(sd, "selection,cleared", "efl.text");
   efl_text_interactive_select_none(sd->text_obj);

   _hide_selection_handler(obj);
}

static void
_efl_ui_text_select_region_set(Eo *obj, Efl_Ui_Text_Data *sd, int start, int end)
{
   Efl_Text_Cursor_Cursor *sel_start, *sel_end;

   if ((sd->password)) return;

   efl_text_interactive_selection_cursors_get(obj, &sel_start, &sel_end);

   efl_text_cursor_position_set(obj, sel_start, start);
   efl_text_cursor_position_set(obj, sel_end, end);
}

EOLIAN static void
_efl_ui_text_cursor_selection_end(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   edje_object_part_text_select_extend(sd->entry_edje, "efl.text");
}

EOLIAN static void
_efl_ui_text_selection_cut(Eo *obj, Efl_Ui_Text_Data *sd)
{
   if ((sd->password)) return;
   _cut_cb(obj);
}

EOLIAN static void
_efl_ui_text_selection_copy(Eo *obj, Efl_Ui_Text_Data *sd)
{
   if ((sd->password)) return;
   _copy_cb(obj);
}

EOLIAN static void
_efl_ui_text_selection_paste(Eo *obj, Efl_Ui_Text_Data *sd)
{
   if ((sd->password)) return;
   _paste_cb(obj);
}

EOLIAN static void
_efl_ui_text_context_menu_clear(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   Elm_Entry_Context_Menu_Item *it;

   EINA_LIST_FREE(sd->items, it)
     {
        eina_stringshare_del(it->label);
        eina_stringshare_del(it->icon_file);
        eina_stringshare_del(it->icon_group);
        free(it);
     }
}

EOLIAN static void
_efl_ui_text_context_menu_item_add(Eo *obj, Efl_Ui_Text_Data *sd, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data)
{
   Elm_Entry_Context_Menu_Item *it;

   it = calloc(1, sizeof(Elm_Entry_Context_Menu_Item));
   if (!it) return;

   sd->items = eina_list_append(sd->items, it);
   it->obj = obj;
   it->label = eina_stringshare_add(label);
   it->icon_file = eina_stringshare_add(icon_file);
   it->icon_type = icon_type;
   it->func = func;
   it->data = (void *)data;
}

EOLIAN static void
_efl_ui_text_context_menu_disabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Eina_Bool disabled)
{
   if (sd->context_menu == !disabled) return;
   sd->context_menu = !disabled;
}

EOLIAN static Eina_Bool
_efl_ui_text_context_menu_disabled_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return !sd->context_menu;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_file_file_set(Eo *obj, Efl_Ui_Text_Data *sd, const char *file, const char *group EINA_UNUSED)
{
   ELM_SAFE_FREE(sd->delay_write, ecore_timer_del);
   if (sd->auto_save) _save_do(obj);
   eina_stringshare_replace(&sd->file, file);
   Eina_Bool int_ret = _load_do(obj);
   return int_ret;
}

EOLIAN static void
_efl_ui_text_efl_file_file_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, const char **file, const char **group)
{
   if (file) *file = sd->file;
   if (group) *group = NULL;
}

EOLIAN static void
_efl_ui_text_cnp_mode_set(Eo *obj, Efl_Ui_Text_Data *sd, Efl_Selection_Format cnp_mode)
{
   Elm_Sel_Format dnd_format = EFL_SELECTION_FORMAT_MARKUP;

   if (cnp_mode != EFL_SELECTION_FORMAT_TARGETS)
     {
        if (cnp_mode & EFL_SELECTION_FORMAT_VCARD)
          ERR("VCARD format not supported for copy & paste!");
        else if (cnp_mode & EFL_SELECTION_FORMAT_HTML)
          ERR("HTML format not supported for copy & paste!");
        cnp_mode &= ~EFL_SELECTION_FORMAT_VCARD;
        cnp_mode &= ~EFL_SELECTION_FORMAT_HTML;
     }

   if (sd->cnp_mode == cnp_mode) return;
   sd->cnp_mode = cnp_mode;
   if (sd->cnp_mode == EFL_SELECTION_FORMAT_TEXT)
     dnd_format = EFL_SELECTION_FORMAT_TEXT;
   else if (cnp_mode == EFL_SELECTION_FORMAT_IMAGE)
     dnd_format |= ELM_SEL_FORMAT_IMAGE;

   elm_drop_target_del(obj, sd->drop_format,
                       _dnd_enter_cb, NULL,
                       _dnd_leave_cb, NULL,
                       _dnd_pos_cb, NULL,
                       _dnd_drop_cb, NULL);
   sd->drop_format = dnd_format;
   elm_drop_target_add(obj, sd->drop_format,
                       _dnd_enter_cb, NULL,
                       _dnd_leave_cb, NULL,
                       _dnd_pos_cb, NULL,
                       _dnd_drop_cb, NULL);
}

EOLIAN static Efl_Selection_Format
_efl_ui_text_cnp_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->cnp_mode;
}

EOLIAN static void
_efl_ui_text_scrollable_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Eina_Bool scroll)
{
   if (sd->scroll == scroll) return;
   sd->scroll = scroll;

   if (scroll)
     {
        edje_object_part_swallow(sd->entry_edje, "efl.text", NULL);
        sd->scroller = efl_add(EFL_UI_INTERNAL_TEXT_SCROLLER_CLASS, obj,
              efl_ui_internal_text_scroller_initialize(efl_added,
                 sd->text_obj, sd->text_table));
        efl_ui_scrollbar_bar_mode_set(sd->scroller, EFL_UI_SCROLLBAR_MODE_AUTO, EFL_UI_SCROLLBAR_MODE_AUTO);
        edje_object_part_swallow(sd->entry_edje, "efl.text", sd->scroller);
        evas_object_clip_set(sd->cursor,
              efl_ui_internal_text_scroller_viewport_clip_get(sd->scroller));
        efl_event_callback_add(sd->scroller, EFL_GFX_ENTITY_EVENT_RESIZE,
              _scroller_size_changed_cb, obj);
     }
   else
     {
        efl_content_set(sd->scroller, NULL);
        edje_object_part_swallow(sd->entry_edje, "efl.text", sd->text_table);
        efl_del(sd->scroller);
        sd->scroller = NULL;
     }
   elm_layout_sizing_eval(obj);
}

EOLIAN static Eina_Bool
_efl_ui_text_scrollable_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->scroll;
}

EOLIAN static void
_efl_ui_text_input_panel_layout_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Input_Panel_Layout layout)
{
   sd->input_panel_layout = layout;

   edje_object_part_text_input_panel_layout_set
     (sd->entry_edje, "efl.text", (Edje_Input_Panel_Layout)layout);

   if (layout == ELM_INPUT_PANEL_LAYOUT_PASSWORD)
     efl_ui_text_input_hint_set(obj, ((sd->input_hints & ~ELM_INPUT_HINT_AUTO_COMPLETE) | ELM_INPUT_HINT_SENSITIVE_DATA));
   else if (layout == ELM_INPUT_PANEL_LAYOUT_TERMINAL)
     efl_ui_text_input_hint_set(obj, (sd->input_hints & ~ELM_INPUT_HINT_AUTO_COMPLETE));
}

EOLIAN static Elm_Input_Panel_Layout
_efl_ui_text_input_panel_layout_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_panel_layout;
}

EOLIAN static void
_efl_ui_text_input_panel_layout_variation_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, int variation)
{
   sd->input_panel_layout_variation = variation;

   edje_object_part_text_input_panel_layout_variation_set
     (sd->entry_edje, "efl.text", variation);
}

EOLIAN static int
_efl_ui_text_input_panel_layout_variation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_panel_layout_variation;
}

EOLIAN static void
_efl_ui_text_autocapital_type_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Autocapital_Type autocapital_type)
{
   sd->autocapital_type = autocapital_type;
   edje_object_part_text_autocapital_type_set
     (sd->entry_edje, "efl.text", (Edje_Text_Autocapital_Type)autocapital_type);
}

EOLIAN static Elm_Autocapital_Type
_efl_ui_text_autocapital_type_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->autocapital_type;
}

EOLIAN static void
_efl_ui_text_prediction_allow_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Eina_Bool prediction)
{
   sd->prediction_allow = prediction;
   edje_object_part_text_prediction_allow_set
     (sd->entry_edje, "efl.text", prediction);
}

EOLIAN static Eina_Bool
_efl_ui_text_prediction_allow_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->prediction_allow;
}

EOLIAN static void
_efl_ui_text_input_hint_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Input_Hints hints)
{
   sd->input_hints = hints;

   edje_object_part_text_input_hint_set
     (sd->entry_edje, "efl.text", (Edje_Input_Hints)hints);
}

EOLIAN static Elm_Input_Hints
_efl_ui_text_input_hint_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_hints;
}

EOLIAN static void
_efl_ui_text_input_panel_enabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Eina_Bool enabled)
{
   sd->input_panel_enable = enabled;
   edje_object_part_text_input_panel_enabled_set
     (sd->entry_edje, "efl.text", enabled);
}

EOLIAN static Eina_Bool
_efl_ui_text_input_panel_enabled_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_panel_enable;
}

EOLIAN static void
_efl_ui_text_input_panel_show(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   edje_object_part_text_input_panel_show(sd->entry_edje, "efl.text");
}

EOLIAN static void
_efl_ui_text_input_panel_hide(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{

   edje_object_part_text_input_panel_hide(sd->entry_edje, "efl.text");
}

EOLIAN static void
_efl_ui_text_input_panel_language_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Input_Panel_Lang lang)
{
   sd->input_panel_lang = lang;
   edje_object_part_text_input_panel_language_set
     (sd->entry_edje, "efl.text", (Edje_Input_Panel_Lang)lang);
}

EOLIAN static Elm_Input_Panel_Lang
_efl_ui_text_input_panel_language_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_panel_lang;
}

EOLIAN static void
_efl_ui_text_input_panel_imdata_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, const void *data, int len)
{
   free(sd->input_panel_imdata);

   sd->input_panel_imdata = calloc(1, len);
   sd->input_panel_imdata_len = len;
   memcpy(sd->input_panel_imdata, data, len);

   edje_object_part_text_input_panel_imdata_set
     (sd->entry_edje, "efl.text", sd->input_panel_imdata,
     sd->input_panel_imdata_len);
}

EOLIAN static void
_efl_ui_text_input_panel_imdata_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, void *data, int *len)
{
   edje_object_part_text_input_panel_imdata_get
     (sd->entry_edje, "efl.text", data, len);
}

EOLIAN static void
_efl_ui_text_input_panel_return_key_type_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Input_Panel_Return_Key_Type return_key_type)
{
   sd->input_panel_return_key_type = return_key_type;

   edje_object_part_text_input_panel_return_key_type_set
     (sd->entry_edje, "efl.text", (Edje_Input_Panel_Return_Key_Type)return_key_type);
}

EOLIAN static Elm_Input_Panel_Return_Key_Type
_efl_ui_text_input_panel_return_key_type_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_panel_return_key_type;
}

EOLIAN static void
_efl_ui_text_input_panel_return_key_disabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Eina_Bool disabled)
{
   sd->input_panel_return_key_disabled = disabled;

   edje_object_part_text_input_panel_return_key_disabled_set
     (sd->entry_edje, "efl.text", disabled);
}

EOLIAN static Eina_Bool
_efl_ui_text_input_panel_return_key_disabled_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_panel_return_key_disabled;
}

EOLIAN static void
_efl_ui_text_input_panel_return_key_autoenabled_set(Eo *obj, Efl_Ui_Text_Data *sd, Eina_Bool enabled)
{
   sd->auto_return_key = enabled;
   _return_key_enabled_check(obj);
}

EOLIAN static void
_efl_ui_text_input_panel_show_on_demand_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Eina_Bool ondemand)
{
   sd->input_panel_show_on_demand = ondemand;

   edje_object_part_text_input_panel_show_on_demand_set
     (sd->entry_edje, "efl.text", ondemand);
}

EOLIAN static Eina_Bool
_efl_ui_text_input_panel_show_on_demand_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_panel_show_on_demand;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_ui_widget_on_access_activate(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, Efl_Ui_Activate act)
{
   if (act != EFL_UI_ACTIVATE_DEFAULT) return EINA_FALSE;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (!elm_widget_disabled_get(obj) &&
       !evas_object_freeze_events_get(obj))
     {
        efl_event_callback_call(obj, EFL_UI_EVENT_CLICKED, NULL);
        if (sd->editable && sd->input_panel_enable)
          edje_object_part_text_input_panel_show(sd->entry_edje, "efl.text");
     }
   return EINA_TRUE;
}

// ATSPI Accessibility

EOLIAN static Eina_Unicode
_efl_ui_text_efl_access_text_character_get(const Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, int offset)
{
   const char *txt;
   int idx = 0;
   Eina_Unicode ret = 0;
   if (offset < 0) return ret;

   if (_pd->password) return ENTRY_PASSWORD_MASK_CHARACTER;

   txt = efl_text_get(obj);
   if (!txt) return ret;

   ret = eina_unicode_utf8_next_get(txt, &idx);
   while (offset--) ret = eina_unicode_utf8_next_get(txt, &idx);

   return ret;
}

EOLIAN static int
_efl_ui_text_efl_access_text_character_count_get(const Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   const char *txt;

   txt = efl_text_get(obj);
   if (!txt) return -1;
   return eina_unicode_utf8_get_len(txt);
}

EOLIAN static char*
_efl_ui_text_efl_access_text_string_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *pd, Efl_Access_Text_Granularity granularity, int *start_offset, int *end_offset)
{
   Evas_Textblock_Cursor *cur = NULL, *cur2 = NULL;
   char *ret = NULL;

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

   ret = evas_textblock_cursor_range_text_get(cur, cur2, EVAS_TEXTBLOCK_TEXT_PLAIN);

   evas_textblock_cursor_free(cur);
   evas_textblock_cursor_free(cur2);

   if (ret && pd->password)
     {
        int i = 0;
        while (ret[i] != '\0')
         ret[i++] = ENTRY_PASSWORD_MASK_CHARACTER;
     }

   return ret;

fail:
   if (start_offset) *start_offset = -1;
   if (end_offset) *end_offset = -1;
   if (cur) evas_textblock_cursor_free(cur);
   if (cur2) evas_textblock_cursor_free(cur2);
   return NULL;
}

EOLIAN static char*
_efl_ui_text_efl_access_text_access_text_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *pd EINA_UNUSED, int start_offset, int end_offset)
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

   if (ret && pd->password)
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
_efl_ui_text_efl_access_text_caret_offset_get(const Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   return efl_text_cursor_position_get(obj, efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN));
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_access_text_caret_offset_set(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, int offset)
{
   efl_text_cursor_position_set(obj, efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN), offset);
   return EINA_TRUE;
}

EOLIAN static int
_efl_ui_text_efl_access_text_selections_count_get(const Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   return _efl_ui_text_selection_get(obj, _pd) ? 1 : 0;
}

EOLIAN static void
_efl_ui_text_efl_access_text_access_selection_get(const Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, int selection_number, int *start_offset, int *end_offset)
{
   if (selection_number != 0) return;

   elm_obj_entry_select_region_get(obj, start_offset, end_offset);
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_access_text_access_selection_set(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, int selection_number, int start_offset, int end_offset)
{
   if (selection_number != 0) return EINA_FALSE;

   _efl_ui_text_select_region_set(obj, _pd, start_offset, end_offset);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_access_text_selection_remove(Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, int selection_number)
{
   if (selection_number != 0) return EINA_FALSE;
   _efl_ui_text_select_none(obj, pd);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_access_text_selection_add(Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, int start_offset, int end_offset)
{
   _efl_ui_text_select_region_set(obj, pd, start_offset, end_offset);

   return EINA_TRUE;
}

EOLIAN static Eina_List*
_efl_ui_text_efl_access_text_bounded_ranges_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *_pd EINA_UNUSED, Eina_Bool screen_coods EINA_UNUSED, Eina_Rect rect EINA_UNUSED, Efl_Access_Text_Clip_Type xclip EINA_UNUSED, Efl_Access_Text_Clip_Type yclip EINA_UNUSED)
{
   return NULL;
}

EOLIAN static int
_efl_ui_text_efl_access_text_offset_at_point_get(const Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, Eina_Bool screen_coods, int x, int y)
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
_efl_ui_text_efl_access_text_character_extents_get(const Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, int offset, Eina_Bool screen_coods, Eina_Rect *rect)
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
_efl_ui_text_efl_access_text_range_extents_get(const Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, Eina_Bool screen_coods, int start_offset, int end_offset, Eina_Rect *rect)
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
_textblock_node_format_to_atspi_text_attr(const Eo *obj,
      Efl_Text_Annotate_Annotation *annotation)
{
   Efl_Access_Text_Attribute *ret;
   const char *txt;

   txt = efl_text_annotation_get(obj, annotation);
   if (!txt) return NULL;

   ret = calloc(1, sizeof(Efl_Access_Text_Attribute));
   if (!ret) return NULL;

   ret->value = eina_stringshare_add(txt);
   int size = strlen(txt);
   ret->name = eina_stringshare_add_length(txt, size);

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_access_text_attribute_get(const Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, const char *attr_name EINA_UNUSED, int *start_offset, int *end_offset, char **value)
{
   Evas_Textblock_Cursor *cur1, *cur2;
   Efl_Access_Text_Attribute *attr;
   Eina_Iterator *annotations;
   Efl_Text_Annotate_Annotation *an;

   cur1 = evas_object_textblock_cursor_new(obj);
   if (!cur1) return EINA_FALSE;

   cur2 = evas_object_textblock_cursor_new(obj);
   if (!cur2)
     {
        evas_textblock_cursor_free(cur1);
        return EINA_FALSE;
     }

   evas_textblock_cursor_pos_set(cur1, *start_offset);
   evas_textblock_cursor_pos_set(cur2, *end_offset);

   annotations = efl_text_range_annotations_get(obj, cur1, cur2);

   evas_textblock_cursor_free(cur1);
   evas_textblock_cursor_free(cur2);

   if (!annotations) return EINA_FALSE;

   EINA_ITERATOR_FOREACH(annotations, an)
     {
        attr = _textblock_node_format_to_atspi_text_attr(obj, an);
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

EOLIAN static Eina_List*
_efl_ui_text_efl_access_text_text_attributes_get(const Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, int *start_offset, int *end_offset)
{
   Evas_Textblock_Cursor *cur1, *cur2;
   Eina_List *ret = NULL;
   Efl_Access_Text_Attribute *attr;
   Eina_Iterator *annotations;
   Efl_Text_Annotate_Annotation *an;

   cur1 = evas_object_textblock_cursor_new(obj);
   if (!cur1) return NULL;

   cur2 = evas_object_textblock_cursor_new(obj);
   if (!cur2)
     {
        evas_textblock_cursor_free(cur1);
        return NULL;
     }

   evas_textblock_cursor_pos_set(cur1, *start_offset);
   evas_textblock_cursor_pos_set(cur2, *end_offset);

   annotations = efl_text_range_annotations_get(obj, cur1, cur2);

   evas_textblock_cursor_free(cur1);
   evas_textblock_cursor_free(cur2);

   if (!annotations) return NULL;

   EINA_ITERATOR_FOREACH(annotations, an)
     {
        attr = _textblock_node_format_to_atspi_text_attr(obj, an);
        if (!attr) continue;
        ret = eina_list_append(ret, attr);
     }
   eina_iterator_free(annotations);

   return ret;
}

EOLIAN static Eina_List*
_efl_ui_text_efl_access_text_default_attributes_get(const Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   Eina_List *ret = NULL;
   Efl_Access_Text_Attribute *attr;
   Efl_Text_Cursor_Cursor *start, *end;
   Eina_Iterator *annotations;
   Efl_Text_Annotate_Annotation *an;

   /* Retrieve all annotations in the text. */
   Eo *mobj = (Eo *)obj; /* XXX const */
   start = efl_text_cursor_new(mobj);
   end = efl_text_cursor_new(mobj);

   efl_text_cursor_paragraph_first(mobj, start);
   efl_text_cursor_paragraph_last(mobj, end);

   annotations = efl_text_range_annotations_get(obj, start, end);

   EINA_ITERATOR_FOREACH(annotations, an)
     {
        attr = _textblock_node_format_to_atspi_text_attr(obj, an);
        if (!attr) continue;
        ret = eina_list_append(ret, attr);
     }
   eina_iterator_free(annotations);

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_access_editable_text_text_content_set(Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, const char *content)
{
   efl_text_set(obj, content);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_access_editable_text_insert(Eo *obj, Efl_Ui_Text_Data *pd, const char *string, int position)
{
   Efl_Text_Cursor_Cursor *cur_obj = efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN);
   efl_text_cursor_position_set(obj, cur_obj, position);
   _efl_ui_text_entry_insert(obj, pd, string);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_access_editable_text_copy(Eo *obj, Efl_Ui_Text_Data *pd, int start, int end)
{
   _efl_ui_text_select_region_set(obj, pd, start, end);
   efl_ui_text_selection_copy(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_access_editable_text_delete(Eo *obj, Efl_Ui_Text_Data *pd, int start_offset, int end_offset)
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

   _efl_ui_text_calc_force(obj, pd);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_access_editable_text_paste(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, int position)
{
   Efl_Text_Cursor_Cursor *cur_obj = efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN);
   efl_text_cursor_position_set(obj, cur_obj, position);
   efl_ui_text_selection_paste(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_access_editable_text_cut(Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, int start, int end)
{
   _efl_ui_text_select_region_set(obj, pd, start, end);
   efl_ui_text_selection_cut(obj);
   return EINA_TRUE;
}

EOLIAN static Efl_Access_State_Set
_efl_ui_text_efl_access_object_state_set_get(const Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   Efl_Access_State_Set ret;
   ret = efl_access_object_state_set_get(efl_super(obj, EFL_UI_TEXT_CLASS));

   if (efl_text_interactive_editable_get(obj))
     STATE_TYPE_SET(ret, EFL_ACCESS_STATE_EDITABLE);

   return ret;
}

EOLIAN static const char*
_efl_ui_text_efl_access_object_i18n_name_get(const Eo *obj, Efl_Ui_Text_Data *pd)
{
   const char *name;
   name = efl_access_object_i18n_name_get(efl_super(obj, EFL_UI_TEXT_CLASS));
   if (name && strncmp("", name, 1)) return name;
   const char *ret = edje_object_part_text_get(pd->entry_edje, "efl.guide");
   return ret;
}

EOLIAN static Efl_Text_Cursor_Cursor *
_efl_ui_text_cursor_new(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd EINA_UNUSED)
{
   Eo *text_obj = sd->text_obj;
   return efl_text_cursor_new(text_obj);
}

static void
_edje_signal_emit(Efl_Ui_Text_Data *sd, const char *sig, const char *src)
{
   edje_object_signal_emit(sd->entry_edje, sig, src);
   edje_object_signal_emit(sd->cursor, sig, src);
   edje_object_signal_emit(sd->cursor_bidi, sig, src);
}

static inline Eo *
_decoration_create(Eo *obj, Efl_Ui_Text_Data *sd,
      const char *group_name, Eina_Bool above)
{
   Eo *ret = NULL;
   Eo *clip = efl_ui_internal_text_scroller_viewport_clip_get(sd->scroller);

   ret = efl_add(EFL_CANVAS_LAYOUT_CLASS, obj);
   elm_widget_element_update(obj, ret, group_name);
   evas_object_smart_member_add(ret, sd->entry_edje);
   if (above)
     {
        evas_object_stack_above(ret, sd->text_table);
     }
   else
     {
        evas_object_stack_below(ret, NULL);
     }
   evas_object_clip_set(ret, clip);
   evas_object_pass_events_set(ret, EINA_TRUE);
   return ret;
}

static void
_create_text_cursors(Eo *obj, Efl_Ui_Text_Data *sd)
{
   sd->cursor = _decoration_create(obj, sd, PART_NAME_CURSOR, EINA_TRUE);
   sd->cursor_bidi = _decoration_create(obj, sd, PART_NAME_CURSOR, EINA_TRUE);

   if (!efl_text_interactive_editable_get(obj))
     {
        evas_object_hide(sd->cursor);
        evas_object_hide(sd->cursor_bidi);
     }
}

static Eina_Position2D
_decoration_calc_offset(Efl_Ui_Text_Data *sd)
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
   Evas_Coord xx2, yy2;
   Eina_Position2D off;
   Eina_Bool bidi_cursor;
   Eo *text_obj;


   EFL_UI_TEXT_DATA_GET(obj, sd);
   if (!sd->deferred_decoration_cursor) return;
   sd->deferred_decoration_cursor = EINA_FALSE;

   text_obj = sd->text_obj;

   xx = yy = ww = hh = -1;
   off =_decoration_calc_offset(sd);
   bidi_cursor = efl_text_cursor_geometry_get(obj,
         efl_text_cursor_get(text_obj, EFL_TEXT_CURSOR_GET_MAIN),
         EFL_TEXT_CURSOR_TYPE_BEFORE, &xx, &yy, &ww, &hh, &xx2, &yy2,
         NULL, NULL);
   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;
   if (sd->cursor)
     {
        evas_object_geometry_set(sd->cursor, off.x + xx, off.y + yy, ww, hh);
     }
   if (sd->cursor_bidi)
     {
        if (bidi_cursor)
          {
             evas_object_geometry_set(sd->cursor_bidi,
                                      off.x + xx2, off.y + yy2 + (hh / 2),
                                      ww, hh / 2);
             evas_object_resize(sd->cursor, ww, hh / 2);
             evas_object_show(sd->cursor_bidi);
          }
        else
          {
             evas_object_hide(sd->cursor_bidi);
          }
     }
   if (sd->cursor_update)
     {
        sd->cursor_update = EINA_FALSE;
        _cursor_geometry_recalc(obj);
     }
}

static void
_clear_text_selection(Efl_Ui_Text_Data *sd)
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
   Efl_Text_Cursor_Cursor *sel_start, *sel_end;

   Eina_List *l;
   Eina_Iterator *range;
   Efl_Ui_Text_Rectangle *rect;
   Eina_Rectangle *r;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (!sd->deferred_decoration_selection) return;
   sd->deferred_decoration_selection = EINA_FALSE;

   off = _decoration_calc_offset(sd);

   efl_text_interactive_selection_cursors_get(text_obj, &sel_start, &sel_end);

   range = efl_canvas_text_range_simple_geometry_get(text_obj,
         sel_start, sel_end);

   l = sd->sel;
   EINA_ITERATOR_FOREACH(range, r)
     {
        /* Create if there isn't a rectangle to populate. */
        if (!l)
          {
             rect = calloc(1, sizeof(Efl_Ui_Text_Rectangle));
             sd->sel = eina_list_append(sd->sel, rect);

             rect->obj_bg = _decoration_create(obj, sd, PART_NAME_SELECTION, EINA_FALSE);
             evas_object_show(rect->obj_bg);
          }
        else
          {
             rect = eina_list_data_get(l);
             l = l->next;
          }

        if (rect->obj_bg)
          {
             evas_object_geometry_set(rect->obj_bg, off.x + r->x, off.y + r->y,
                                      r->w, r->h);
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
_anchors_free(Efl_Ui_Text_Data *sd)
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
_anchor_get(Eo *obj, Efl_Ui_Text_Data *sd, Efl_Text_Annotate_Annotation *an)
{
   Anchor *anc; 
   Eina_List *i;
   const char *str;

   str = efl_text_annotation_get(obj, an);

   EINA_LIST_FOREACH(sd->anchors, i, anc)
     {
        if (anc->annotation == an) break;
     }

   if (!anc && (efl_text_annotation_is_item(obj, an) || !strncmp(str, "a ", 2)))
     {
        const char *p;

        anc = calloc(1, sizeof(Anchor));
        if (anc)
          {
             anc->obj = obj;
             anc->annotation = an;
             anc->item = efl_text_annotation_is_item(obj, an);
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
_anchors_update(Eo *obj, Efl_Ui_Text_Data *sd)
{
   Evas_Object *smart, *clip;
   Eina_Iterator *it;
   Eina_Position2D off;
   Efl_Text_Cursor_Cursor *start, *end;
   Efl_Text_Annotate_Annotation *an;
   Eina_List *i, *ii;
   Anchor *anc;

   if (!sd->deferred_decoration_anchor) return;
   sd->deferred_decoration_anchor = EINA_FALSE;

   if (sd->anchors_updated) return;

   sd->gen++;

   start = efl_text_cursor_new(sd->text_obj);
   end = efl_text_cursor_new(sd->text_obj);

   /* Retrieve all annotations in the text. */
   efl_text_cursor_paragraph_first(obj, start);
   efl_text_cursor_paragraph_last(obj, end);

   it = efl_text_range_annotations_get(obj, start, end);
   efl_text_cursor_free(sd->text_obj, start);
   efl_text_cursor_free(sd->text_obj, end);

   smart = evas_object_smart_parent_get(obj);
   clip = evas_object_clip_get(sd->text_obj);
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
                            efl_canvas_object_clip_set(ob, clip);
                            efl_canvas_object_pass_events_set(ob, EINA_TRUE);
                            rect->obj = ob;
                         }
                    }

                  rect = eina_list_data_get(anc->rects);
                  efl_text_item_geometry_get(sd->text_obj,
                        anc->annotation, &cx, &cy, &cw, &ch);
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
                  start = efl_text_cursor_new(obj);
                  end = efl_text_cursor_new(obj);
                  efl_text_annotation_positions_get(obj, anc->annotation,
                        start, end);

                  range = efl_canvas_text_range_geometry_get(obj, start, end);
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
                       ob = evas_object_rectangle_add(obj);
                       evas_object_color_set(ob, 0, 0, 0, 0);
                       evas_object_smart_member_add(ob, smart);
                       evas_object_stack_above(ob, obj);
                       evas_object_clip_set(ob, clip);
                       evas_object_repeat_events_set(ob, EINA_TRUE);
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
                            evas_object_geometry_set(rect->obj_bg,
                                                     off.x + r->x, off.y + r->y,
                                                     r->w, r->h);
                            evas_object_show(rect->obj_bg);
                         }
                       if (rect->obj_fg)
                         {
                            evas_object_geometry_set(rect->obj_fg,
                                                     off.x + r->x, off.y + r->y,
                                                     r->w, r->h);
                            evas_object_show(rect->obj_fg);
                         }
                       if (rect->obj)
                         {
                            evas_object_geometry_set(rect->obj,
                                                     off.x + r->x, off.y + r->y,
                                                     r->w, r->h);
                            evas_object_show(rect->obj);
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

static void
_deferred_decoration_job(void *data)
{
   EFL_UI_TEXT_DATA_GET(data, sd);
   _update_decorations(data);
   sd->deferred_decoration_job = NULL;
}

static void
_decoration_defer(Eo *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);
   if (!sd->deferred_decoration_job)
     {
        sd->deferred_decoration_job =
           ecore_job_add(_deferred_decoration_job, obj);
     }
}

static void
_selection_defer(Eo *obj, Efl_Ui_Text_Data *sd)
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
_efl_ui_text_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   if (efl_invalidated_get(event->object)) return;
   EFL_UI_TEXT_DATA_GET(data, sd);
   sd->text_changed = EINA_TRUE;
   sd->cursor_update = EINA_TRUE;
   _update_guide_text(data, sd);
   elm_layout_sizing_eval(data);
   _decoration_defer(data);
}

static void
_efl_ui_text_changed_user_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;

   if (efl_invalidated_get(event->object)) return;
   EFL_UI_TEXT_DATA_GET(obj, sd);
   sd->text_changed = EINA_TRUE;
   _update_guide_text(data, sd);
   elm_layout_sizing_eval(obj);
   _decoration_defer_all(obj);
}

static void
_efl_ui_text_cursor_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   if (efl_invalidated_get(event->object)) return;
   EFL_UI_TEXT_DATA_GET(data, sd);
   sd->cur_changed = EINA_TRUE;
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
_efl_ui_text_selection_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   if (efl_invalidated_get(event->object)) return;
   Eo *obj = data;
   Efl_Text_Cursor_Cursor *start, *end;
   char *text;
   EFL_UI_TEXT_DATA_GET(obj, sd);

   efl_text_interactive_selection_cursors_get(obj, &start, &end);

   text = efl_canvas_text_range_text_get(obj, start, end);
   if (!text || (text[0] == '\0'))
     {
        _edje_signal_emit(sd, "selection,cleared", "efl.text");
        sd->have_selection = EINA_FALSE;
        _selection_clear(data, 0);
     }
   else
     {
        if (!sd->have_selection)
          {
             _edje_signal_emit(sd, "selection,start", "efl.text");
          }
        _edje_signal_emit(sd, "selection,changed", "efl.text");
        sd->have_selection = EINA_TRUE;
        _selection_store(EFL_SELECTION_TYPE_PRIMARY, obj);
     }
   if (text) free(text);
   _selection_defer(obj, sd);
}

static void
_efl_ui_text_move_cb(void *data, Evas *e EINA_UNUSED,
      Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _decoration_defer_all(data);
}

static void
_efl_ui_text_item_factory_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *pd,
      Efl_Canvas_Text_Factory *item_factory)
{
   if (pd->item_factory) efl_unref(pd->item_factory);
   pd->item_factory = efl_ref(item_factory);
}

static Eo *
_efl_ui_text_item_factory_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *pd)
{
   return pd->item_factory;
}

/* Efl.Part begin */

#define STRCMP(X, Y) strncmp((X), (Y), strlen(X))

EOLIAN static Eina_Bool
_efl_ui_text_text_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *pd,
      const char *part, const char *text)
{
   if (!STRCMP("efl.text_guide", part))
     {
        efl_text_set(pd->text_guide_obj, text);
        return EINA_TRUE;
     }
   else if (!STRCMP("efl.text", part))
     {
        efl_text_set(pd->text_obj, text);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static const char *
_efl_ui_text_text_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *pd,
      const char *part)
{
   if (!STRCMP("efl.text_guide", part))
     {
        return efl_text_get(pd->text_guide_obj);
     }
   else if (!STRCMP("efl.text", part))
     {
        return efl_text_get(pd->text_obj);
     }

   return NULL;
}

#undef STRCMP

ELM_PART_OVERRIDE(efl_ui_text, EFL_UI_TEXT, Efl_Ui_Text_Data)
ELM_PART_OVERRIDE_TEXT_SET(efl_ui_text, EFL_UI_TEXT, Efl_Ui_Text_Data)
ELM_PART_OVERRIDE_TEXT_GET(efl_ui_text, EFL_UI_TEXT, Efl_Ui_Text_Data)
#include "efl_ui_text_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

//ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT(MY_CLASS_PFX)

#define EFL_UI_TEXT_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_text)

#include "efl_ui_text.eo.c"

EOLIAN static Eo *
_efl_ui_text_async_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   sd->async.enabled = EINA_TRUE;

   // FIXME: should we have to keep this efl_ui_text_xxx classes?
   // Then, going to make new theme for these classes? ex) efl/text_async?
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "text");
   obj = efl_constructor(efl_super(obj, EFL_UI_TEXT_ASYNC_CLASS));

   _update_text_theme(obj, sd);
   return obj;
}

#include "efl_ui_text_async.eo.c"

#undef MY_CLASS
#define MY_CLASS EFL_UI_TEXT_EDITABLE_CLASS


EOLIAN static Eo *
_efl_ui_text_editable_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   // FIXME: should we have to keep this efl_ui_text_xxx classes?
   // Then, going to make new theme for these classes? ex) efl/text_editable?
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "text");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_text_interactive_editable_set(obj, EINA_TRUE);

   return obj;
}

#include "efl_ui_text_editable.eo.c"
