#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_TEXT_PROTECTED
#define ELM_INTERFACE_ATSPI_TEXT_EDITABLE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

//#include "elm_entry_internal_part.eo.h"
//#include "elm_part_helper.h"
//
#include "elm_interface_scrollable.h"
#include "elm_widget_layout.h"
#include "elm_entry_common.h"
#include "elm_widget_entry.h"
#include "efl_ui_text.eo.h"
#include "elm_hoversel.eo.h"

typedef struct _Efl_Ui_Text_Data        Efl_Ui_Text_Data;
typedef struct _Efl_Ui_Text_Rectangle   Efl_Ui_Text_Rectangle;
typedef struct _Anchor                  Anchor;
typedef struct _Item_Obj                Item_Obj;

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
   Eo                                   *cursor;
   Eo                                   *cursor_bidi;
   Evas_Object                          *start_handler;
   Evas_Object                          *end_handler;
   Ecore_Job                            *deferred_recalc_job;
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
   Eina_List                            *item_anchors;
   Eina_List                            *sel;
   Eina_List                            *items; /** context menu item list */
   Item_Obj                             *item_objs;
   Eina_List                            *item_providers;
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

   Elm_Cnp_Mode                          cnp_mode;
   Elm_Sel_Format                        drop_format;

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
   Eina_Bool                             deferred_cur : 1;
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
   Eina_Bool                             test_bit : 1;
};

struct _Anchor
{
   Eo                    *obj;
   char                  *name;
   Efl_Text_Annotate_Annotation *annotation;
   Eina_List             *sel;
   Eina_Bool              item : 1;
};

struct _Item_Obj
{
   EINA_INLIST;
   Anchor                *an;
   char                  *name;
   Evas_Object           *obj;
};

#define EFL_UI_TEXT_DATA_GET(o, sd) \
  Efl_Ui_Text_Data * sd = efl_data_scope_get(o, EFL_UI_TEXT_CLASS)

#define EFL_UI_TEXT_DATA_GET_OR_RETURN(o, ptr)         \
  EFL_UI_TEXT_DATA_GET(o, ptr);                        \
  if (EINA_UNLIKELY(!ptr))                           \
    {                                                \
       CRI("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return;                                       \
    }

#define EFL_UI_TEXT_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EFL_UI_TEXT_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
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

#define MY_CLASS EFL_UI_TEXT_CLASS
#define MY_CLASS_PFX efl_ui_text
#define MY_CLASS_NAME "Efl.Ui.Text"
#define MY_CLASS_NAME_LEGACY "elm_entry"

#include "efl_ui_internal_text_interactive.h"

/* Maximum chunk size to be inserted to the entry at once
 * FIXME: This size is arbitrary, should probably choose a better size.
 * Possibly also find a way to set it to a low value for weak computers,
 * and to a big value for better computers. */
#define EFL_UI_TEXT_CHUNK_SIZE 10000
#define EFL_UI_TEXT_DELAY_WRITE_TIME 2.0

#define ELM_PRIV_ENTRY_SIGNALS(cmd) \
   cmd(SIG_ABORTED, "aborted", "") \
   cmd(SIG_ACTIVATED, "activated", "") \
   cmd(SIG_ANCHOR_CLICKED, "anchor,clicked", "") \
   cmd(SIG_ANCHOR_DOWN, "anchor,down", "") \
   cmd(SIG_ANCHOR_HOVER_OPENED, "anchor,hover,opened", "") \
   cmd(SIG_ANCHOR_IN, "anchor,in", "") \
   cmd(SIG_ANCHOR_OUT, "anchor,out", "") \
   cmd(SIG_ANCHOR_UP, "anchor,up", "") \
   cmd(SIG_CHANGED, "changed", "") \
   cmd(SIG_CHANGED_USER, "changed,user", "") \
   cmd(SIG_CLICKED, "clicked", "") \
   cmd(SIG_CLICKED_DOUBLE, "clicked,double", "") \
   cmd(SIG_CLICKED_TRIPLE, "clicked,triple", "") \
   cmd(SIG_CURSOR_CHANGED, "cursor,changed", "") \
   cmd(SIG_CURSOR_CHANGED_MANUAL, "cursor,changed,manual", "") \
   cmd(SIG_FOCUSED, "focused", "") \
   cmd(SIG_UNFOCUSED, "unfocused", "") \
   cmd(SIG_LONGPRESSED, "longpressed", "") \
   cmd(SIG_MAX_LENGTH, "maxlength,reached", "") \
   cmd(SIG_PREEDIT_CHANGED, "preedit,changed", "") \
   cmd(SIG_PRESS, "press", "") \
   cmd(SIG_REDO_REQUEST, "redo,request", "") \
   cmd(SIG_SELECTION_CHANGED, "selection,changed", "") \
   cmd(SIG_SELECTION_CLEARED, "selection,cleared", "") \
   cmd(SIG_SELECTION_COPY, "selection,copy", "") \
   cmd(SIG_SELECTION_CUT, "selection,cut", "") \
   cmd(SIG_SELECTION_PASTE, "selection,paste", "") \
   cmd(SIG_SELECTION_START, "selection,start", "") \
   cmd(SIG_TEXT_SET_DONE, "text,set,done", "") \
   cmd(SIG_THEME_CHANGED, "theme,changed", "") \
   cmd(SIG_UNDO_REQUEST, "undo,request", "") \
   cmd(SIG_REJECTED, "rejected", "")

ELM_PRIV_ENTRY_SIGNALS(ELM_PRIV_STATIC_VARIABLE_DECLARE);

#define ENTRY_PASSWORD_MASK_CHARACTER 0x002A

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   ELM_PRIV_ENTRY_SIGNALS(ELM_PRIV_SMART_CALLBACKS_DESC)
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};
#undef ELM_PRIV_ENTRY_SIGNALS

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"icon", "elm.swallow.icon"},
   {"end", "elm.swallow.end"},
   {NULL, NULL}
};

static Eina_List *entries = NULL;

struct _Mod_Api
{
   void (*obj_hook)(Evas_Object *obj);
   void (*obj_unhook)(Evas_Object *obj);
   void (*obj_longpress)(Evas_Object *obj);
};

static void _create_selection_handlers(Evas_Object *obj, Efl_Ui_Text_Data *sd, const char *file);
static void _magnifier_move(void *data);
static void _update_decorations(Eo *obj);
static void _create_text_cursors(Eo *obj, Efl_Ui_Text_Data *sd);
static void _efl_ui_text_changed_cb(void *data EINA_UNUSED, const Efl_Event *event);
static void _efl_ui_text_selection_changed_cb(void *data EINA_UNUSED, const Efl_Event *event);
static void _efl_ui_text_cursor_changed_cb(void *data EINA_UNUSED, const Efl_Event *event);
static void _efl_ui_text_move_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED);
static void _efl_ui_text_select_none(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd);
static void _efl_ui_text_anchor_hover_end(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd);
static void _efl_ui_text_anchor_hover_parent_set(Eo *obj, Efl_Ui_Text_Data *sd, Evas_Object *parent);
static const char* _efl_ui_text_selection_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd);
static void _edje_signal_emit(Efl_Ui_Text_Data *obj, const char *sig, const char *src);
static void _decoration_defer_all(Eo *obj);
static inline Eo * _decoration_create(Efl_Ui_Text_Data *sd, const char *file, const char *source, Eina_Bool above);
static void _decoration_defer(Eo *obj);
static void _anchors_clear_all(Evas_Object *o EINA_UNUSED, Efl_Ui_Text_Data *sd);
static void _unused_item_objs_free(Efl_Ui_Text_Data *sd);
static void _clear_text_selection(Efl_Ui_Text_Data *sd);

static Mod_Api *
_module_find(Evas_Object *obj EINA_UNUSED)
{
   static Elm_Module *m = NULL;

   if (m) goto ok;  // already found - just use
   if (!(m = _elm_module_find_as("entry/api"))) return NULL;
   // get module api
   m->api = malloc(sizeof(Mod_Api));
   if (!m->api) return NULL;

   ((Mod_Api *)(m->api))->obj_hook = // called on creation
     _elm_module_symbol_get(m, "obj_hook");
   ((Mod_Api *)(m->api))->obj_unhook = // called on deletion
     _elm_module_symbol_get(m, "obj_unhook");
   ((Mod_Api *)(m->api))->obj_longpress = // called on long press menu
     _elm_module_symbol_get(m, "obj_longpress");
ok: // ok - return api
   return m->api;
}

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

static Eina_Bool
_delay_write(void *data)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   _save_do(data);
   sd->delay_write = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_efl_ui_text_guide_update(Evas_Object *obj,
                        Eina_Bool has_text)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   if ((has_text) && (!sd->has_text))
     edje_object_signal_emit(sd->entry_edje, "elm,guide,disabled", "elm");
   else if ((!has_text) && (sd->has_text))
     edje_object_signal_emit(sd->entry_edje, "elm,guide,enabled", "elm");

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

   vc.text = edje_object_part_text_get(sd->entry_edje, "elm.text");
   res = efl_event_callback_legacy_call(obj, EFL_UI_TEXT_EVENT_VALIDATE, (void *)&vc);
   buf = eina_strbuf_new();
   eina_strbuf_append_printf(buf, "validation,%s,%s", vc.signal, res == EINA_FALSE ? "fail" : "pass");
   edje_object_signal_emit(sd->scr_edje, eina_strbuf_string_get(buf), "elm");
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
        edje_object_signal_emit(sd->start_handler, "elm,handler,hide", "elm");
        sd->start_handler_shown = EINA_FALSE;
     }
   if (sd->end_handler_shown)
     {
        edje_object_signal_emit(sd->end_handler, "elm,handler,hide", "elm");
        sd->end_handler_shown = EINA_FALSE;
     }
}

static Eina_Rectangle *
_viewport_region_get(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);
   Eina_Rectangle *rect = eina_rectangle_new(0, 0, 0, 0);
   Evas_Object *parent;

   if (!rect) return NULL;
   if (sd->scroll)
     elm_interface_scrollable_content_viewport_geometry_get
           (obj, &rect->x, &rect->y, &rect->w, &rect->h);
   else
     evas_object_geometry_get(sd->entry_edje, &rect->x, &rect->y, &rect->w, &rect->h);

   parent = elm_widget_parent_get(obj);
   while (parent)
     {
        if (efl_isa(parent, ELM_INTERFACE_SCROLLABLE_MIXIN))
          {
             Eina_Rectangle r;
             EINA_RECTANGLE_SET(&r, 0, 0, 0, 0);
             evas_object_geometry_get(parent, &r.x, &r.y, &r.w, &r.h);
             if (!eina_rectangle_intersection(rect, &r))
               {
                  rect->x = rect->y = rect->w = rect->h = 0;
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
   Evas_Coord ent_x, ent_y;

   if (!sd->have_selection)
     {
        _hide_selection_handler(obj);
        return;
     }

   if (!sd->sel_handler_disabled)
     {
        Eina_Rectangle *rect;
        Evas_Coord hx, hy;
        Eina_Bool hidden = EINA_FALSE;
        Efl_Text_Cursor_Cursor *sel_start, *sel_end;

        efl_ui_text_interactive_selection_cursors_get(obj, &sel_start, &sel_end);

        if (!sd->start_handler)
          {
             const char *file;
             efl_file_get(sd->entry_edje, &file, NULL);
             _create_selection_handlers(obj, sd, file);
          }

        rect = _viewport_region_get(obj);

        evas_object_geometry_get(sd->entry_edje, &ent_x, &ent_y, NULL, NULL);

        efl_text_cursor_geometry_get(obj, sel_start,
              EFL_TEXT_CURSOR_TYPE_BEFORE,
              &sx, &sy, NULL, &sh,
              NULL, NULL, NULL, NULL);
        hx = ent_x + sx;
        hy = ent_y + sy + sh;
        evas_object_move(sd->start_handler, hx, hy);

        if (!eina_rectangle_xcoord_inside(rect, hx) ||
            !eina_rectangle_ycoord_inside(rect, hy))
          {
             hidden = EINA_TRUE;
          }
        if (!sd->start_handler_shown && !hidden)
          {
             edje_object_signal_emit(sd->start_handler,
                                     "elm,handler,show", "elm");
             sd->start_handler_shown = EINA_TRUE;
          }
        else if (sd->start_handler_shown && hidden)
          {
             edje_object_signal_emit(sd->start_handler,
                                     "elm,handler,hide", "elm");
             sd->start_handler_shown = EINA_FALSE;
          }

        efl_text_cursor_geometry_get(obj, sel_end,
              EFL_TEXT_CURSOR_TYPE_BEFORE,
              &ex, &ey, NULL, &eh,
              NULL, NULL, NULL, NULL);
        hx = ent_x + ex;
        hy = ent_y + ey + eh;
        evas_object_move(sd->end_handler, hx, hy);

        if (!eina_rectangle_xcoord_inside(rect, hx) ||
            !eina_rectangle_ycoord_inside(rect, hy))
          {
             hidden = EINA_TRUE;
          }
        if (!sd->end_handler_shown && !hidden)
          {
             edje_object_signal_emit(sd->end_handler,
                                     "elm,handler,show", "elm");
             sd->end_handler_shown = EINA_TRUE;
          }
        else if (sd->end_handler_shown && hidden)
          {
             edje_object_signal_emit(sd->end_handler,
                                     "elm,handler,hide", "elm");
             sd->end_handler_shown = EINA_FALSE;
          }
        eina_rectangle_free(rect);
     }
   else
     {
        if (sd->start_handler_shown)
          {
             edje_object_signal_emit(sd->start_handler,
                                     "elm,handler,hide", "elm");
             sd->start_handler_shown = EINA_FALSE;
          }
        if (sd->end_handler_shown)
          {
             edje_object_signal_emit(sd->end_handler,
                                     "elm,handler,hide", "elm");
             sd->end_handler_shown = EINA_FALSE;
          }
     }
}

static const char *
_efl_ui_text_theme_group_get(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (sd->editable)
     {
        if (sd->password) return "base-password";
        else
          {
             if (sd->single_line) return "base-single";
             else
               {
                  switch (sd->line_wrap)
                    {
                     case ELM_WRAP_CHAR:
                       return "base-charwrap";

                     case ELM_WRAP_WORD:
                       return "base";

                     case ELM_WRAP_MIXED:
                       return "base-mixedwrap";

                     case ELM_WRAP_NONE:
                     default:
                       return "base-nowrap";
                    }
               }
          }
     }
   else
     {
        if (sd->password) return "base-password";
        else
          {
             if (sd->single_line) return "base-single-noedit";
             else
               {
                  switch (sd->line_wrap)
                    {
                     case ELM_WRAP_CHAR:
                       return "base-noedit-charwrap";

                     case ELM_WRAP_WORD:
                       return "base-noedit";

                     case ELM_WRAP_MIXED:
                       return "base-noedit-mixedwrap";

                     case ELM_WRAP_NONE:
                     default:
                       return "base-nowrap-noedit";
                    }
               }
          }
     }
}

static void
_edje_entry_user_insert(Evas_Object *obj, const char *data)
{
   if (!data) return;
   EFL_UI_TEXT_DATA_GET(obj, sd);

   sd->changed = EINA_TRUE;
   edje_object_part_text_user_insert(sd->entry_edje, "elm.text", data);
   elm_layout_sizing_eval(obj);
}

static Eina_Bool
_selection_data_cb(void *data EINA_UNUSED,
                   Evas_Object *obj,
                   Elm_Selection_Data *sel_data)
{
   char *buf;

   if (!sel_data->data) return EINA_FALSE;
   EFL_UI_TEXT_DATA_GET(obj, sd);

   buf = malloc(sel_data->len + 1);
   if (!buf)
     {
        ERR("Failed to allocate memory, obj: %p", obj);
        return EINA_FALSE;
     }
   memcpy(buf, sel_data->data, sel_data->len);
   buf[sel_data->len] = '\0';

   if ((sel_data->format & ELM_SEL_FORMAT_IMAGE) &&
       (sd->cnp_mode != ELM_CNP_MODE_NO_IMAGE))
     {
        char *entry_tag;
        int len;
        static const char *tag_string =
           "<item absize=240x180 href=file://%s></item>";

        len = strlen(tag_string) + strlen(buf);
        entry_tag = alloca(len + 1);
        snprintf(entry_tag, len + 1, tag_string, buf);
        _edje_entry_user_insert(obj, entry_tag);
     }
   else if (sel_data->format & ELM_SEL_FORMAT_MARKUP)
     {
        _edje_entry_user_insert(obj, buf);
     }
   else
     {
        Efl_Text_Cursor_Cursor *cur, *start, *end;
        efl_ui_text_interactive_selection_cursors_get(obj, &start, &end);
        if (!efl_text_cursor_equal(obj, start, end))
          {
             efl_canvas_text_range_delete(obj, start, end);
          }
        cur = efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN);
        efl_text_cursor_text_insert(obj, cur, buf);
     }
   free(buf);

   return EINA_TRUE;
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
      (sd->entry_edje, "elm.text", EDJE_CURSOR_USER, x, y);
   pos = edje_object_part_text_cursor_pos_get
      (sd->entry_edje, "elm.text", EDJE_CURSOR_USER);
   edje_object_part_text_cursor_pos_set(sd->entry_edje, "elm.text",
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
       (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN, drop->x, drop->y);

   if (!rv) WRN("Warning: Failed to position cursor: paste anyway");

   rv = _selection_data_cb(NULL, obj, drop);

   return rv;
}

static Elm_Sel_Format
_get_drop_format(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   if ((sd->editable) && (!sd->single_line) && (!sd->password) && (!sd->disabled))
     return ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE;
   return ELM_SEL_FORMAT_MARKUP;
}

/* we can't reuse layout's here, because it's on entry_edje only */
EOLIAN static Eina_Bool
_efl_ui_text_elm_widget_disable(Eo *obj, Efl_Ui_Text_Data *sd)
{
   elm_drop_target_del(obj, sd->drop_format,
                       _dnd_enter_cb, NULL,
                       _dnd_leave_cb, NULL,
                       _dnd_pos_cb, NULL,
                       _dnd_drop_cb, NULL);
   if (elm_object_disabled_get(obj))
     {
        edje_object_signal_emit(sd->entry_edje, "elm,state,disabled", "elm");
        if (sd->scroll)
          {
             edje_object_signal_emit(sd->scr_edje, "elm,state,disabled", "elm");
             elm_interface_scrollable_freeze_set(obj, EINA_TRUE);
          }
        sd->disabled = EINA_TRUE;
     }
   else
     {
        edje_object_signal_emit(sd->entry_edje, "elm,state,enabled", "elm");
        if (sd->scroll)
          {
             edje_object_signal_emit(sd->scr_edje, "elm,state,enabled", "elm");
             elm_interface_scrollable_freeze_set(obj, EINA_FALSE);
          }
        sd->disabled = EINA_FALSE;
        sd->drop_format = _get_drop_format(obj);
        elm_drop_target_add(obj, sd->drop_format,
                            _dnd_enter_cb, NULL,
                            _dnd_leave_cb, NULL,
                            _dnd_pos_cb, NULL,
                            _dnd_drop_cb, NULL);
     }

   return EINA_TRUE;
}

/* It gets the background object from from_edje object and
 * sets the background object to to_edje object.
 * The background object has to be moved to proper Edje object
 * when scrollable status is changed. */
static void
_efl_ui_text_background_switch(Evas_Object *from_edje, Evas_Object *to_edje)
{
   Evas_Object *bg_obj;

   if (!from_edje || !to_edje) return;

   if (edje_object_part_exists(from_edje, "elm.swallow.background") &&
       edje_object_part_exists(to_edje, "elm.swallow.background") &&
       !edje_object_part_swallow_get(to_edje, "elm.swallow.background"))
     {
        bg_obj = edje_object_part_swallow_get(from_edje, "elm.swallow.background");

        if (bg_obj)
          {
             edje_object_part_unswallow(from_edje, bg_obj);
             edje_object_part_swallow(to_edje, "elm.swallow.background", bg_obj);
          }
     }
}

/* we can't issue the layout's theming code here, cause it assumes an
 * unique edje object, always */
EOLIAN static Elm_Theme_Apply
_efl_ui_text_elm_widget_theme_apply(Eo *obj, Efl_Ui_Text_Data *sd)
{
   const char *str;
   const char *style = elm_widget_style_get(obj);
   Elm_Theme_Apply theme_apply;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   // Note: We are skipping elm_layout here! This is by design.
   // This assumes the following inheritance: my_class -> layout -> widget ...
   theme_apply = elm_obj_widget_theme_apply(efl_cast(obj, ELM_WIDGET_CLASS));
   if (!theme_apply) return ELM_THEME_APPLY_FAILED;

   evas_event_freeze(evas_object_evas_get(obj));

   edje_object_mirrored_set
     (wd->resize_obj, efl_ui_mirrored_get(obj));

   edje_object_scale_set
     (wd->resize_obj,
     efl_ui_scale_get(obj) * elm_config_scale_get());

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   elm_widget_theme_object_set
     (obj, sd->entry_edje, "efl_ui_text", _efl_ui_text_theme_group_get(obj), style);

   if (sd->sel_allow && _elm_config->desktop_entry)
     edje_obj_part_text_select_allow_set
        (sd->entry_edje, "elm.text", EINA_TRUE);
   else
     edje_obj_part_text_select_allow_set
        (sd->entry_edje, "elm.text", EINA_FALSE);

   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(sd->entry_edje, "elm,state,disabled", "elm");

   edje_object_part_text_input_panel_layout_set
     (sd->entry_edje, "elm.text", (Edje_Input_Panel_Layout)sd->input_panel_layout);
   edje_object_part_text_input_panel_layout_variation_set
     (sd->entry_edje, "elm.text", sd->input_panel_layout_variation);
   edje_object_part_text_autocapital_type_set
     (sd->entry_edje, "elm.text", (Edje_Text_Autocapital_Type)sd->autocapital_type);
   edje_object_part_text_prediction_allow_set
     (sd->entry_edje, "elm.text", sd->prediction_allow);
   edje_object_part_text_input_hint_set
     (sd->entry_edje, "elm.text", (Edje_Input_Hints)sd->input_hints);
   edje_object_part_text_input_panel_enabled_set
     (sd->entry_edje, "elm.text", sd->input_panel_enable);
   edje_object_part_text_input_panel_imdata_set
     (sd->entry_edje, "elm.text", sd->input_panel_imdata,
     sd->input_panel_imdata_len);
   edje_object_part_text_input_panel_return_key_type_set
     (sd->entry_edje, "elm.text", (Edje_Input_Panel_Return_Key_Type)sd->input_panel_return_key_type);
   edje_object_part_text_input_panel_return_key_disabled_set
     (sd->entry_edje, "elm.text", sd->input_panel_return_key_disabled);
   edje_object_part_text_input_panel_show_on_demand_set
     (sd->entry_edje, "elm.text", sd->input_panel_show_on_demand);

   // elm_entry_cursor_pos_set -> cursor,changed -> widget_show_region_set
   // -> smart_objects_calculate will call all smart calculate functions,
   // and one of them can delete elm_entry.
   evas_object_ref(obj);

   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(sd->entry_edje, "elm,action,focus", "elm");
        if (sd->scroll)
          edje_object_signal_emit(sd->scr_edje, "elm,action,focus", "elm");
     }

   edje_object_message_signal_process(sd->entry_edje);

   Evas_Object* clip = evas_object_clip_get(sd->entry_edje);
   evas_object_clip_set(sd->hit_rect, clip);

   if (sd->scroll)
     {
        Elm_Theme_Apply ok = ELM_THEME_APPLY_FAILED;

        elm_interface_scrollable_mirrored_set(obj, efl_ui_mirrored_get(obj));

        if (sd->single_line)
          ok = elm_widget_theme_object_set
          (obj, sd->scr_edje, "scroller", "entry_single", style);
        if (!ok)
          elm_widget_theme_object_set
          (obj, sd->scr_edje, "scroller", "entry", style);

        _efl_ui_text_background_switch(sd->entry_edje, sd->scr_edje);

        str = edje_object_data_get(sd->scr_edje, "focus_highlight");
     }
   else
     {
        _efl_ui_text_background_switch(sd->scr_edje, sd->entry_edje);

        str = edje_object_data_get(sd->entry_edje, "focus_highlight");
     }

   if ((str) && (!strcmp(str, "on")))
     elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_highlight_in_theme_set(obj, EINA_FALSE);

   if (sd->start_handler)
     {
        elm_widget_theme_object_set(obj, sd->start_handler,
                                    "entry", "handler/start", style);
        elm_widget_theme_object_set(obj, sd->end_handler,
                                    "entry", "handler/end", style);
     }

   sd->changed = EINA_TRUE;
   elm_layout_sizing_eval(obj);

   sd->has_text = !sd->has_text;
   _efl_ui_text_guide_update(obj, !sd->has_text);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   efl_event_callback_legacy_call(obj, ELM_LAYOUT_EVENT_THEME_CHANGED, NULL);

   evas_object_unref(obj);

   return theme_apply;
}

static void
_cursor_geometry_recalc(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   efl_event_callback_legacy_call(obj, EFL_UI_TEXT_EVENT_CURSOR_CHANGED, NULL);

   Evas_Coord x, y, w, h;
   Evas_Coord x2, y2, w2, h2;
   Evas_Coord cx, cy, cw, ch;

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
         edje_object_part_swallow_get(sd->entry_edje, "elm.text"),
         &x2, &y2, &w2, &h2);
   cx = cx + x - x2;
   cy = cy + y - y2;
   elm_widget_show_region_set(obj, cx, cy, cw, ch, EINA_FALSE);
}

EOLIAN static void
_efl_ui_text_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Text_Data *sd)
{
   Evas_Coord minw, minh, resw, resh;
   Evas_Coord fw, fh;
   Eo *sw;
   Eina_Bool wrap;

   evas_object_geometry_get(obj, NULL, NULL, &resw, &resh);

   sw = edje_object_part_swallow_get(sd->entry_edje, "elm.text");
   if (!sw) return;

   wrap = efl_text_wrap_get(sw);

   if (!sd->changed && (resw == sd->ent_w) && (resh == sd->ent_h)) return;

   sd->changed = EINA_FALSE;
   sd->ent_w = resw;
   sd->ent_h = resh;


   evas_event_freeze(evas_object_evas_get(obj));
   if (sd->scroll)
     {
        Evas_Coord vw, vh;
        Evas_Coord tw, th;
        elm_interface_scrollable_content_viewport_geometry_get(obj, NULL, NULL, &vw, &vh);
        efl_gfx_size_set(sd->entry_edje, vw, vh);
        efl_gfx_size_get(sw, &tw, &th);
        efl_canvas_text_size_formatted_get(sw, &fw, &fh);
        evas_object_size_hint_min_set(sw, fw, fh);
        edje_object_size_min_calc(sd->entry_edje, &minw, &minh);
        evas_object_size_hint_min_set(sw, -1, -1);

        if (vw > minw) minw = vw;
        efl_gfx_size_set(sd->entry_edje, minw, minh);

        if (!efl_text_multiline_get(sw))
          {
             evas_object_size_hint_min_set(obj, -1, minh);
          }
     }
   else
     {
        efl_canvas_text_size_formatted_get(sw, &fw, &fh);
        evas_object_size_hint_min_set(sw, fw, fh);
        edje_object_size_min_calc(sd->entry_edje, &minw, &minh);
        evas_object_size_hint_min_set(sw, -1, -1);
        if (wrap == EFL_TEXT_FORMAT_WRAP_NONE)
          {
             evas_object_size_hint_min_set(obj, minw, minh);
          }
     }
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   _cursor_geometry_recalc(obj);
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
_efl_ui_text_elm_widget_on_focus(Eo *obj, Efl_Ui_Text_Data *sd, Elm_Object_Item *item EINA_UNUSED)
{
   Evas_Object *top;
   Eina_Bool top_is_win = EINA_FALSE;

   if (!sd->editable) return EINA_FALSE;

   top = elm_widget_top_get(obj);
   if (top && efl_isa(top, EFL_UI_WIN_CLASS))
     top_is_win = EINA_TRUE;

   if (elm_widget_focus_get(obj))
     {
        Eo *sw = edje_object_part_swallow_get(sd->entry_edje, "elm.text");
        evas_object_focus_set(sw, EINA_TRUE);

        _edje_signal_emit(sd, "elm,action,focus", "elm");
        if (sd->scroll)
          edje_object_signal_emit(sd->scr_edje, "elm,action,focus", "elm");

        if (top && top_is_win && sd->input_panel_enable && !sd->input_panel_show_on_demand)
          elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_ON);
        efl_event_callback_legacy_call(obj, ELM_WIDGET_EVENT_FOCUSED, NULL);
        if (_elm_config->atspi_mode)
          elm_interface_atspi_accessible_state_changed_signal_emit(obj, ELM_ATSPI_STATE_FOCUSED, EINA_TRUE);
        _return_key_enabled_check(obj);
        _validate(obj);
     }
   else
     {
        Eo *sw = edje_object_part_swallow_get(sd->entry_edje, "elm.text");

        _edje_signal_emit(sd, "elm,action,unfocus", "elm");
        if (sd->scroll)
          edje_object_signal_emit(sd->scr_edje, "elm,action,unfocus", "elm");
        evas_object_focus_set(sw, EINA_FALSE);

        if (top && top_is_win && sd->input_panel_enable)
          elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_OFF);
        efl_event_callback_legacy_call(obj, ELM_WIDGET_EVENT_UNFOCUSED, NULL);
        if (_elm_config->atspi_mode)
          elm_interface_atspi_accessible_state_changed_signal_emit(obj, ELM_ATSPI_STATE_FOCUSED, EINA_FALSE);

        if (_elm_config->selection_clear_enable)
          {
             if ((sd->have_selection) && (!sd->hoversel))
               {
                  sd->sel_mode = EINA_FALSE;
                  elm_widget_scroll_hold_pop(obj);
                  edje_object_part_text_select_allow_set(sd->entry_edje, "elm.text", EINA_FALSE);
                  edje_object_signal_emit(sd->entry_edje, "elm,state,select,off", "elm");
                  edje_object_part_text_select_none(sd->entry_edje, "elm.text");
               }
          }
        edje_object_signal_emit(sd->scr_edje, "validation,default", "elm");
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_widget_on_focus_region(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Coord edje_x, edje_y, elm_x, elm_y;

   efl_text_cursor_geometry_get(obj,
         efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN),
         EFL_TEXT_CURSOR_TYPE_BEFORE,
         x, y, w, h,
         NULL, NULL, NULL, NULL);

   if (sd->single_line)
     {
        evas_object_geometry_get(sd->entry_edje, NULL, NULL, NULL, h);
        if (y) *y = 0;
     }

   evas_object_geometry_get(sd->entry_edje, &edje_x, &edje_y, NULL, NULL);

   evas_object_geometry_get(obj, &elm_x, &elm_y, NULL, NULL);

   if (x) *x += edje_x - elm_x;
   if (y) *y += edje_y - elm_y;

   return EINA_TRUE;
}

static void
_show_region_hook(void *data EINA_UNUSED,
                  Evas_Object *obj)
{
   Evas_Coord x, y, w, h;
   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   elm_interface_scrollable_content_region_show(obj, x, y, w, h);
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_widget_sub_object_del(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool ret = EINA_FALSE;
   /* unfortunately entry doesn't follow the signal pattern
    * elm,state,icon,{visible,hidden}, so we have to replicate this
    * smart function */
   if (sobj == elm_layout_content_get(obj, "elm.swallow.icon"))
     {
        elm_layout_signal_emit(obj, "elm,action,hide,icon", "elm");
     }
   else if (sobj == elm_layout_content_get(obj, "elm.swallow.end"))
     {
        elm_layout_signal_emit(obj, "elm,action,hide,end", "elm");
     }

   ret = elm_obj_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
   if (!ret) return EINA_FALSE;

   return EINA_TRUE;
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
       (sd->entry_edje, "elm.text", &cx, &cy, &cw, &ch);

   evas_object_size_hint_min_get(sd->hoversel, &mw, &mh);
   if (cx + mw > w)
     cx = w - mw;
   if (cy + mh > h)
     cy = h - mh;
   evas_object_move(sd->hoversel, x + cx, y + cy);
   evas_object_resize(sd->hoversel, mw, mh);
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
                 (sd->entry_edje, "elm.text", EINA_TRUE);
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
   edje_object_part_text_select_none(sd->entry_edje, "elm.text");

   if (!_elm_config->desktop_entry)
     {
        if (!sd->password)
          edje_object_part_text_select_allow_set
            (sd->entry_edje, "elm.text", EINA_TRUE);
     }
   edje_object_signal_emit(sd->entry_edje, "elm,state,select,on", "elm");

   if (!_elm_config->desktop_entry)
     elm_widget_scroll_hold_push(data);
}

static char *
_item_tags_remove(const char *str)
{
   char *ret;
   Eina_Strbuf *buf;

   if (!str)
     return NULL;

   buf = eina_strbuf_new();
   if (!buf)
     return NULL;

   if (!eina_strbuf_append(buf, str))
     {
        eina_strbuf_free(buf);
        return NULL;
     }

   while (EINA_TRUE)
     {
        const char *temp = eina_strbuf_string_get(buf);
        char *start_tag = NULL;
        char *end_tag = NULL;
        size_t sindex;
        size_t eindex;

        start_tag = strstr(temp, "<item");
        if (!start_tag)
          start_tag = strstr(temp, "</item");
        if (start_tag)
          end_tag = strstr(start_tag, ">");
        else
          break;
        if (!end_tag || start_tag > end_tag)
          break;

        sindex = start_tag - temp;
        eindex = end_tag - temp + 1;
        if (!eina_strbuf_remove(buf, sindex, eindex))
          break;
     }

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);

   return ret;
}

void
_efl_ui_text_entry_paste(Evas_Object *obj,
                       const char *entry)
{
   char *str = NULL;

   if (!entry) return;

   EFL_UI_TEXT_CHECK(obj);
   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (sd->cnp_mode == ELM_CNP_MODE_NO_IMAGE)
     {
        str = _item_tags_remove(entry);
        if (!str) str = strdup(entry);
     }
   else
     str = strdup(entry);
   if (!str) str = (char *)entry;

   _edje_entry_user_insert(obj, str);

   if (str != entry) free(str);
}

static void
_paste_cb(Eo *obj)
{
   Elm_Sel_Format formats = ELM_SEL_FORMAT_MARKUP;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SELECTION_PASTE, NULL);

   sd->selection_asked = EINA_TRUE;

   if (sd->cnp_mode == ELM_CNP_MODE_PLAINTEXT)
     formats = ELM_SEL_FORMAT_TEXT;
   else if (sd->cnp_mode != ELM_CNP_MODE_NO_IMAGE)
     formats |= ELM_SEL_FORMAT_IMAGE;

   elm_cnp_selection_get
     (obj, ELM_SEL_TYPE_CLIPBOARD, formats, _selection_data_cb, NULL);
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
   if ((selection == ELM_SEL_TYPE_CLIPBOARD) ||
       (selection == ELM_SEL_TYPE_PRIMARY))
     {
        _efl_ui_text_select_none(data, sd);
     }
}

static void
_selection_store(Elm_Sel_Type seltype,
                 Evas_Object *obj)
{
   char *sel;
   Efl_Text_Cursor_Cursor *start, *end;
   Elm_Sel_Format format = ELM_SEL_FORMAT_TEXT;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   efl_ui_text_interactive_selection_cursors_get(obj, &start, &end);
   sel = efl_canvas_text_range_text_get(obj, start, end);

   if ((!sel) || (!sel[0])) return;  /* avoid deleting our own selection */

   elm_cnp_selection_set
     (obj, seltype, format, sel, strlen(sel));
   elm_cnp_selection_loss_callback_set(obj, seltype, _selection_clear, obj);
   if (seltype == ELM_SEL_TYPE_CLIPBOARD)
     eina_stringshare_replace(&sd->cut_sel, sel);

   free(sel);
}

static void
_cut_cb(Eo *obj)
{
   Efl_Text_Cursor_Cursor *start, *end;
   EFL_UI_TEXT_DATA_GET(obj, sd);

   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SELECTION_CUT, NULL);
   /* Store it */
   sd->sel_mode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set
       (sd->entry_edje, "elm.text", EINA_FALSE);
   edje_object_signal_emit(sd->entry_edje, "elm,state,select,off", "elm");

   if (!_elm_config->desktop_entry)
     elm_widget_scroll_hold_pop(obj);

   _selection_store(ELM_SEL_TYPE_CLIPBOARD, obj);
   efl_ui_text_interactive_selection_cursors_get(obj, &start, &end);
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

   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SELECTION_COPY, NULL);
   sd->sel_mode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     {
        edje_object_part_text_select_allow_set
          (sd->entry_edje, "elm.text", EINA_FALSE);
        edje_object_signal_emit(sd->entry_edje, "elm,state,select,off", "elm");
        elm_widget_scroll_hold_pop(obj);
     }
   _selection_store(ELM_SEL_TYPE_CLIPBOARD, obj);
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
       (sd->entry_edje, "elm.text", EINA_FALSE);
   edje_object_signal_emit(sd->entry_edje, "elm,state,select,off", "elm");
   if (!_elm_config->desktop_entry)
     elm_widget_scroll_hold_pop(data);
   edje_object_part_text_select_none(sd->entry_edje, "elm.text");
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

   efl_event_callback_legacy_call(obj, EFL_UI_TEXT_EVENT_CONTEXT_OPEN, NULL);

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
               (sd->entry_edje, "elm.text", EINA_FALSE);
             edje_object_part_text_select_abort(sd->entry_edje, "elm.text");
          }
     }
}

static void
_magnifier_proxy_update(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _magnifier_move(data);
}

static void
_magnifier_create(void *data)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   double scale = _elm_config->magnifier_scale;
   Evas *e;
   Evas_Coord w, h, mw, mh;

   evas_object_del(sd->mgf_proxy);
   evas_object_del(sd->mgf_bg);
   evas_object_del(sd->mgf_clip);

   e = evas_object_evas_get(data);

   //Bg
   sd->mgf_bg = edje_object_add(e);
   _elm_theme_object_set(data, sd->mgf_bg, "entry", "magnifier", "default");
   evas_object_show(sd->mgf_bg);

   //Proxy
   sd->mgf_proxy = evas_object_image_add(e);
   evas_object_event_callback_add(sd->mgf_proxy, EVAS_CALLBACK_RESIZE,
                                  _magnifier_proxy_update, data);
   evas_object_event_callback_add(sd->mgf_proxy, EVAS_CALLBACK_MOVE,
                                  _magnifier_proxy_update, data);
   edje_object_part_swallow(sd->mgf_bg, "elm.swallow.content", sd->mgf_proxy);
   evas_object_image_source_set(sd->mgf_proxy, data);
   evas_object_geometry_get(data, NULL, NULL, &w, &h);

   //Clipper
   sd->mgf_clip = evas_object_rectangle_add(e);
   evas_object_color_set(sd->mgf_clip, 0, 0, 0, 0);
   evas_object_show(sd->mgf_clip);
   evas_object_clip_set(sd->mgf_proxy, sd->mgf_clip);

   mw = (Evas_Coord)(scale * (float) w);
   mh = (Evas_Coord)(scale * (float) h);
   if ((mw <= 0) || (mh <= 0)) return;

   evas_object_layer_set(sd->mgf_bg, EVAS_LAYER_MAX);
   evas_object_layer_set(sd->mgf_proxy, EVAS_LAYER_MAX);
}

static void
_magnifier_move(void *data)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   Evas_Coord x, y, w, h;
   Evas_Coord px, py, pw, ph;
   Evas_Coord cx, cy, ch;
   Evas_Coord ex, ey;
   Evas_Coord mx, my, mw, mh;
   Evas_Coord diffx = 0;
   Evas_Object *top;
   double fx, fy, fw, fh;
   double scale = _elm_config->magnifier_scale;

   edje_object_part_text_cursor_geometry_get(sd->entry_edje, "elm.text",
                                             &cx, &cy, NULL, &ch);
   if (sd->scroll)
     {
        Evas_Coord ox, oy;
        evas_object_geometry_get(sd->scr_edje, &ex, &ey, NULL, NULL);
        elm_interface_scrollable_content_pos_get(data, &ox, &oy);
        ex -= ox;
        ey -= oy;
     }
   else
     {
        evas_object_geometry_get(data, &ex, &ey, NULL, NULL);
     }
   cx += ex;
   cy += ey;

   //Move the Magnifier
   edje_object_parts_extends_calc(sd->mgf_bg, &x, &y, &w, &h);
   evas_object_move(sd->mgf_bg, cx - x - (w / 2), cy - y - h);

   mx = cx - x - (w / 2);
   my = cy - y - h;
   mw = w;
   mh = h;

   // keep magnifier inside window
   top = elm_widget_top_get(data);
   if (top && efl_isa(top, EFL_UI_WIN_CLASS))
     {
        Evas_Coord wh, ww;
        evas_object_geometry_get(top, NULL, NULL, &ww, &wh);
        if (mx < 0)
          {
             diffx = mx;
             mx = 0;
          }
        if (mx + mw > ww)
          {
             diffx = - (ww - (mx + mw));
             mx = ww - mw;
          }
        if (my < 0)
          my = 0;
        if (my + mh > wh)
          my = wh - mh;
        evas_object_move(sd->mgf_bg, mx, my);
     }

   //Set the Proxy Render Area
   evas_object_geometry_get(data, &x, &y, &w, &h);
   evas_object_geometry_get(sd->mgf_proxy, &px, &py, &pw, &ph);

   fx = -((cx - x) * scale) + (pw * 0.5) + diffx;
   fy = -((cy - y) * scale) + (ph * 0.5) - (ch * 0.5 * scale);
   fw = w * scale;
   fh = h * scale;
   evas_object_image_fill_set(sd->mgf_proxy, fx, fy, fw, fh);

   //Update Clipper Area
   int tx = fx;
   int ty = fy;
   int tw = fw;
   int th = fh;
   if (tx > 0) px += tx;
   if (ty > 0) py += ty;
   if (-(tx - pw) > tw) pw -= (-((tx - pw) + tw));
   if (-(ty - ph) > th) ph -= (-((ty - ph) + th));
   evas_object_move(sd->mgf_clip, px, py);
   evas_object_resize(sd->mgf_clip, pw, ph);
}

static void
_magnifier_hide(void *data)
{
   EFL_UI_TEXT_DATA_GET(data, sd);
   edje_object_signal_emit(sd->mgf_bg, "elm,action,hide,magnifier", "elm");
   elm_widget_scroll_freeze_pop(data);
   evas_object_hide(sd->mgf_clip);
}

static void
_magnifier_show(void *data)
{
   EFL_UI_TEXT_DATA_GET(data, sd);
   edje_object_signal_emit(sd->mgf_bg, "elm,action,show,magnifier", "elm");
   elm_widget_scroll_freeze_push(data);
   evas_object_show(sd->mgf_clip);
}

static Eina_Bool
_long_press_cb(void *data)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   if (_elm_config->magnifier_enable)
     {
        _magnifier_create(data);
        _magnifier_show(data);
        _magnifier_move(data);
     }
   /* Context menu will not appear if context menu disabled is set
    * as false on a long press callback */
   else if (!_elm_config->context_menu_disabled &&
            (!_elm_config->desktop_entry))
     _menu_call(data);

   sd->long_pressed = EINA_TRUE;

   sd->longpress_timer = NULL;
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_LONGPRESSED, NULL);

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
        if ((sd->long_pressed) && (_elm_config->magnifier_enable))
          {
             _magnifier_hide(data);
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
        if ((sd->long_pressed) && (_elm_config->magnifier_enable))
          {
             Evas_Coord x, y;
             Eina_Bool rv;

             evas_object_geometry_get(sd->entry_edje, &x, &y, NULL, NULL);
             rv = edje_object_part_text_cursor_coord_set
               (sd->entry_edje, "elm.text", EDJE_CURSOR_USER,
               ev->cur.canvas.x - x, ev->cur.canvas.y - y);
             if (rv)
               {
                  edje_object_part_text_cursor_copy
                    (sd->entry_edje, "elm.text", EDJE_CURSOR_USER, EDJE_CURSOR_MAIN);
               }
             else
               WRN("Warning: Cannot move cursor");

             _magnifier_move(data);
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

static void
_entry_changed_handle(void *data,
                      const Efl_Event_Description* event)
{
   Evas_Coord minh;
   const char *text;
   Eina_Bool single_line;
   Eo *obj = data;

   EFL_UI_TEXT_DATA_GET(data, sd);

   single_line = !efl_text_multiline_get(obj);

   evas_event_freeze(evas_object_evas_get(data));
   sd->changed = EINA_TRUE;
   /* Reset the size hints which are no more relevant. Keep the
    * height, this is a hack, but doesn't really matter cause we'll
    * re-eval in a moment. */
   evas_object_size_hint_min_get(data, NULL, &minh);
   evas_object_size_hint_min_set(data, -1, minh);

   if (sd->single_line != single_line)
     {
        sd->single_line = single_line;
        elm_obj_widget_theme_apply(data);
     }

   elm_layout_sizing_eval(data);
   ELM_SAFE_FREE(sd->text, eina_stringshare_del);
   ELM_SAFE_FREE(sd->delay_write, ecore_timer_del);
   evas_event_thaw(evas_object_evas_get(data));
   evas_event_thaw_eval(evas_object_evas_get(data));
   if ((sd->auto_save) && (sd->file))
     sd->delay_write = ecore_timer_add(EFL_UI_TEXT_DELAY_WRITE_TIME,
                                       _delay_write, data);

   _return_key_enabled_check(data);
   text = efl_text_get(data);
   if (text)
     {
        if (text[0])
          _efl_ui_text_guide_update(data, EINA_TRUE);
        else
          _efl_ui_text_guide_update(data, EINA_FALSE);
     }
   _validate(data);

   /* callback - this could call callbacks that delete the
    * entry... thus... any access to sd after this could be
    * invalid */
   efl_event_callback_legacy_call(data, event, NULL);
}

static void
_entry_changed_user_signal_cb(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              const char *emission EINA_UNUSED,
                              const char *source EINA_UNUSED)
{
   Elm_Entry_Change_Info info;
   Edje_Entry_Change_Info *edje_info = (Edje_Entry_Change_Info *)
     edje_object_signal_callback_extra_data_get();

   if (edje_info)
     {
        memcpy(&info, edje_info, sizeof(info));
        efl_event_callback_legacy_call(data, EFL_UI_TEXT_EVENT_CHANGED_USER, &info);
     }
   else
     {
        efl_event_callback_legacy_call(data, EFL_UI_TEXT_EVENT_CHANGED_USER, NULL);
     }
   if (_elm_config->atspi_mode)
     {
        Elm_Atspi_Text_Change_Info atspi_info;
        if (edje_info && edje_info->insert)
          {
             atspi_info.content = edje_info->change.insert.content;
             atspi_info.pos = edje_info->change.insert.pos;
             atspi_info.len = edje_info->change.insert.plain_length;
             elm_interface_atspi_accessible_event_emit(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, data, ELM_INTERFACE_ATSPI_TEXT_EVENT_ACCESS_TEXT_INSERTED, &atspi_info);
          }
        else if (edje_info && !edje_info->insert)
          {
             atspi_info.content = edje_info->change.del.content;
             atspi_info.pos = MIN(edje_info->change.del.start, edje_info->change.del.end);
             atspi_info.len = MAX(edje_info->change.del.start, edje_info->change.del.end) - atspi_info.pos;
             elm_interface_atspi_accessible_event_emit(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, data, ELM_INTERFACE_ATSPI_TEXT_EVENT_ACCESS_TEXT_REMOVED, &atspi_info);
          }
     }
}

static void
_entry_preedit_changed_signal_cb(void *data,
                                 Evas_Object *obj EINA_UNUSED,
                                 const char *emission EINA_UNUSED,
                                 const char *source EINA_UNUSED)
{
   _entry_changed_handle(data, EFL_UI_TEXT_EVENT_PREEDIT_CHANGED);
}

static void
_entry_undo_request_signal_cb(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              const char *emission EINA_UNUSED,
                              const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, EFL_UI_TEXT_EVENT_UNDO_REQUEST, NULL);
}

static void
_entry_redo_request_signal_cb(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              const char *emission EINA_UNUSED,
                              const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, EFL_UI_TEXT_EVENT_REDO_REQUEST, NULL);
}

static void
_entry_cursor_changed_signal_cb(void *data,
                                Evas_Object *obj EINA_UNUSED,
                                const char *emission EINA_UNUSED,
                                const char *source EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);
   sd->cursor_pos = edje_object_part_text_cursor_pos_get
       (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
   sd->cur_changed = EINA_TRUE;
   if (elm_widget_focus_get(data))
     {
        edje_object_signal_emit(sd->entry_edje, "elm,action,show,cursor", "elm");
     }
   else
     {
        edje_object_signal_emit(sd->entry_edje, "elm,action,hide,cursor", "elm");
     }
   _cursor_geometry_recalc(data);
   if (_elm_config->atspi_mode)
     elm_interface_atspi_accessible_event_emit(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, data, ELM_INTERFACE_ATSPI_TEXT_EVENT_ACCESS_TEXT_CARET_MOVED, NULL);
}

static void
_entry_cursor_changed_manual_signal_cb(void *data,
                                       Evas_Object *obj EINA_UNUSED,
                                       const char *emission EINA_UNUSED,
                                       const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (data, EFL_UI_TEXT_EVENT_CURSOR_CHANGED_MANUAL, NULL);
   if (_elm_config->atspi_mode)
     elm_interface_atspi_accessible_event_emit(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, data, ELM_INTERFACE_ATSPI_TEXT_EVENT_ACCESS_TEXT_CARET_MOVED, NULL);
}

static void
_signal_anchor_geoms_do_things_with_lol(Efl_Ui_Text_Data *sd,
                                        Elm_Entry_Anchor_Info *ei)
{
   Evas_Textblock_Rectangle *r;
   const Eina_List *geoms, *l;
   Evas_Coord px, py, x, y;

   geoms = edje_object_part_text_anchor_geometry_get
       (sd->entry_edje, "elm.text", ei->name);

   if (!geoms) return;

   evas_object_geometry_get(
      edje_object_part_object_get(sd->entry_edje, "elm.text"),
      &x, &y, NULL, NULL);
   evas_pointer_canvas_xy_get
     (evas_object_evas_get(sd->entry_edje), &px, &py);

   EINA_LIST_FOREACH(geoms, l, r)
     {
        if (((r->x + x) <= px) && ((r->y + y) <= py) &&
            ((r->x + x + r->w) > px) && ((r->y + y + r->h) > py))
          {
             ei->x = r->x + x;
             ei->y = r->y + y;
             ei->w = r->w;
             ei->h = r->h;
             break;
          }
     }
}

static void
_entry_anchor_down_signal_cb(void *data,
                             Evas_Object *obj EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
{
   Elm_Entry_Anchor_Info ei;
   const char *p;
   char *p2;

   EFL_UI_TEXT_DATA_GET(data, sd);

   p = emission + sizeof("nchor,mouse,down,");
   ei.button = strtol(p, &p2, 10);
   ei.name = p2 + 1;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with_lol(sd, &ei);

   if (!sd->disabled)
     efl_event_callback_legacy_call(data, EFL_UI_TEXT_EVENT_ANCHOR_DOWN, &ei);
}

static void
_entry_anchor_up_signal_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Elm_Entry_Anchor_Info ei;
   const char *p;
   char *p2;

   EFL_UI_TEXT_DATA_GET(data, sd);

   p = emission + sizeof("nchor,mouse,up,");
   ei.button = strtol(p, &p2, 10);
   ei.name = p2 + 1;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with_lol(sd, &ei);

   if (!sd->disabled)
     efl_event_callback_legacy_call(data, EFL_UI_TEXT_EVENT_ANCHOR_UP, &ei);
}

static void
_anchor_hover_del_cb(void *data,
                     Evas *e EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   ELM_SAFE_FREE(sd->anchor_hover.pop, evas_object_del);
   evas_object_event_callback_del_full
     (sd->anchor_hover.hover, EVAS_CALLBACK_DEL, _anchor_hover_del_cb, obj);
}

static void
_anchor_hover_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);
   _efl_ui_text_anchor_hover_end(data, sd);
}

static void
_entry_hover_anchor_clicked_do(Evas_Object *obj,
      Efl_Ui_Text_Anchor_Info *info)
{
   Evas_Object *hover_parent;
   Evas_Coord x, w, y, h, px, py;
   Efl_Ui_Text_Anchor_Hover_Info ei;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (sd->hoversel) return;

   ei.anchor_info = info;

   sd->anchor_hover.pop = elm_icon_add(obj);
   evas_object_move(sd->anchor_hover.pop, info->x, info->y);
   evas_object_resize(sd->anchor_hover.pop, info->w, info->h);

   sd->anchor_hover.hover = elm_hover_add(obj);
   evas_object_event_callback_add
     (sd->anchor_hover.hover, EVAS_CALLBACK_DEL, _anchor_hover_del_cb, obj);
   efl_ui_mirrored_set
     (sd->anchor_hover.hover, efl_ui_mirrored_get(obj));
   if (sd->anchor_hover.hover_style)
     elm_object_style_set
       (sd->anchor_hover.hover, sd->anchor_hover.hover_style);

   hover_parent = sd->anchor_hover.hover_parent;
   if (!hover_parent) hover_parent = obj;
   elm_hover_parent_set(sd->anchor_hover.hover, hover_parent);
   elm_hover_target_set(sd->anchor_hover.hover, sd->anchor_hover.pop);
   ei.hover = sd->anchor_hover.hover;

   evas_object_geometry_get(hover_parent, &x, &y, &w, &h);
   ei.hover_parent.x = x;
   ei.hover_parent.y = y;
   ei.hover_parent.w = w;
   ei.hover_parent.h = h;
   px = info->x + (info->w / 2);
   py = info->y + (info->h / 2);
   ei.hover_left = 1;
   if (px < (x + (w / 3))) ei.hover_left = 0;
   ei.hover_right = 1;
   if (px > (x + ((w * 2) / 3))) ei.hover_right = 0;
   ei.hover_top = 1;
   if (py < (y + (h / 3))) ei.hover_top = 0;
   ei.hover_bottom = 1;
   if (py > (y + ((h * 2) / 3))) ei.hover_bottom = 0;

   /* Swap right and left because they switch sides in RTL */
   if (efl_ui_mirrored_get(sd->anchor_hover.hover))
     {
        Eina_Bool tmp = ei.hover_left;

        ei.hover_left = ei.hover_right;
        ei.hover_right = tmp;
     }

   efl_event_callback_legacy_call(obj, EFL_UI_TEXT_EVENT_ANCHOR_HOVER_OPENED, &ei);
   efl_event_callback_add
     (sd->anchor_hover.hover, EFL_UI_EVENT_CLICKED, _anchor_hover_clicked_cb, obj);

   /* FIXME: Should just check if there's any callback registered to
    * the smart events instead.  This is used to determine if anyone
    * cares about the hover or not. */
   if (!elm_layout_content_get(sd->anchor_hover.hover, "middle") &&
       !elm_layout_content_get(sd->anchor_hover.hover, "left") &&
       !elm_layout_content_get(sd->anchor_hover.hover, "right") &&
       !elm_layout_content_get(sd->anchor_hover.hover, "top") &&
       !elm_layout_content_get(sd->anchor_hover.hover, "bottom"))
     {
        ELM_SAFE_FREE(sd->anchor_hover.hover, evas_object_del);
     }
   else
     evas_object_show(sd->anchor_hover.hover);
}

static void
_entry_anchor_move_signal_cb(void *data EINA_UNUSED,
                             Evas_Object *obj EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
{
}

static void
_entry_anchor_in_signal_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Elm_Entry_Anchor_Info ei;

   EFL_UI_TEXT_DATA_GET(data, sd);

   ei.name = emission + sizeof("nchor,mouse,in,");
   ei.button = 0;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with_lol(sd, &ei);

   if (!sd->disabled)
     efl_event_callback_legacy_call(data, EFL_UI_TEXT_EVENT_ANCHOR_IN, &ei);
}

static void
_entry_anchor_out_signal_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *emission EINA_UNUSED,
                            const char *source EINA_UNUSED)
{
   Elm_Entry_Anchor_Info ei;

   EFL_UI_TEXT_DATA_GET(data, sd);

   ei.name = emission + sizeof("nchor,mouse,out,");
   ei.button = 0;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with_lol(sd, &ei);

   if (!sd->disabled)
     efl_event_callback_legacy_call(data, EFL_UI_TEXT_EVENT_ANCHOR_OUT, &ei);
}

static void
_entry_key_enter_signal_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, EFL_UI_TEXT_EVENT_ACTIVATED, NULL);
}

static void
_entry_key_escape_signal_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *emission EINA_UNUSED,
                            const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, EFL_UI_TEXT_EVENT_ABORTED, NULL);
}

static void
_entry_mouse_down_signal_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *emission EINA_UNUSED,
                            const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, EFL_UI_TEXT_EVENT_PRESS, NULL);
}

static void
_entry_mouse_clicked_signal_cb(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               const char *emission EINA_UNUSED,
                               const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_CLICKED, NULL);
}

static void
_entry_mouse_double_signal_cb(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              const char *emission EINA_UNUSED,
                              const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_CLICKED_DOUBLE, NULL);
}

static void
_entry_mouse_triple_signal_cb(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              const char *emission EINA_UNUSED,
                              const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_CLICKED_TRIPLE, NULL);
}

static Evas_Object *
_item_get(void *data, const char *item)
{
   Eina_List *l;
   Evas_Object *o;
   Elm_Entry_Item_Provider *ip;
   const char *style = elm_widget_style_get(data);

   EFL_UI_TEXT_DATA_GET(data, sd);

   EINA_LIST_FOREACH(sd->item_providers, l, ip)
     {
        o = ip->func(ip->data, data, item);
        if (o) return o;
     }
   if (item && !strncmp(item, "file://", 7))
     {
        const char *fname = item + 7;

        o = evas_object_image_filled_add(evas_object_evas_get(data));
        evas_object_image_file_set(o, fname, NULL);
        if (evas_object_image_load_error_get(o) == EVAS_LOAD_ERROR_NONE)
          {
             evas_object_show(o);
          }
        else
          {
             evas_object_del(o);
             o = edje_object_add(evas_object_evas_get(data));
             elm_widget_theme_object_set
               (data, o, "entry/emoticon", "wtf", style);
          }
        return o;
     }

   o = edje_object_add(evas_object_evas_get(data));
   if (!elm_widget_theme_object_set
         (data, o, "entry", item, style))
     elm_widget_theme_object_set
       (data, o, "entry/emoticon", "wtf", style);
   return o;
}

static void
_markup_filter_cb(void *data,
                  Evas_Object *edje EINA_UNUSED,
                  const char *part EINA_UNUSED,
                  char **text)
{
   Eina_List *l;
   Elm_Entry_Markup_Filter *tf;

   EFL_UI_TEXT_DATA_GET(data, sd);

   EINA_LIST_FOREACH(sd->markup_filters, l, tf)
     {
        tf->func(tf->data, data, text);
        if (!*text)
          break;
     }
}

EOLIAN static void
_efl_ui_text_efl_canvas_layout_signal_signal_emit(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, const char *emission, const char *source)
{
   /* always pass to both edje objs */
   efl_canvas_layout_signal_emit(sd->entry_edje, emission, source);

   // FIXME: This should not be here!
   efl_canvas_layout_signal_process(sd->entry_edje, EINA_TRUE);

   if (sd->scr_edje)
     {
        efl_canvas_layout_signal_emit(sd->scr_edje, emission, source);
        efl_canvas_layout_signal_process(sd->scr_edje, EINA_TRUE); // FIXME
     }
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_canvas_layout_signal_signal_callback_add(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Eina_Bool ok;

   ok = efl_canvas_layout_signal_callback_add(sd->entry_edje, emission, source, func_cb, data);
   if (sd->scr_edje)
     ok = efl_canvas_layout_signal_callback_add(sd->scr_edje, emission, source, func_cb, data);

   return ok;
}

EOLIAN static Eina_Bool
_efl_ui_text_efl_canvas_layout_signal_signal_callback_del(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Eina_Bool ok;

   ok = efl_canvas_layout_signal_callback_del(sd->entry_edje, emission, source, func_cb, data);
   if (sd->scr_edje)
     ok = efl_canvas_layout_signal_callback_del(sd->scr_edje, emission, source, func_cb, data);

   return ok;
}

#if 0
static Eina_Bool
_efl_ui_text_content_set(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;
   int_ret = efl_content_set(efl_part(efl_super(obj, MY_CLASS), part), content);
   if (!int_ret) return EINA_FALSE;

   /* too bad entry does not follow the pattern
    * "elm,state,{icon,end},visible", we have to repeat ourselves */
   if (!part || !strcmp(part, "icon") || !strcmp(part, "elm.swallow.icon"))
     efl_ui_text_icon_visible_set(obj, EINA_TRUE);

   if (!part || !strcmp(part, "end") || !strcmp(part, "elm.swallow.end"))
     efl_ui_text_end_visible_set(obj, EINA_TRUE);

   return EINA_TRUE;
}


static Evas_Object*
_efl_ui_text_content_unset(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, const char *part)
{
   Evas_Object *ret = NULL;
   ret = efl_content_unset(efl_part(efl_super(obj, MY_CLASS), part));
   if (!ret) return NULL;

   /* too bad entry does not follow the pattern
    * "elm,state,{icon,end},hidden", we have to repeat ourselves */
   if (!part || !strcmp(part, "icon") || !strcmp(part, "elm.swallow.icon"))
     efl_ui_text_icon_visible_set(obj, EINA_FALSE);

   if (!part || !strcmp(part, "end") || !strcmp(part, "elm.swallow.end"))
     efl_ui_text_end_visible_set(obj, EINA_FALSE);

   return ret;
}
#endif

static char *
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
#if 0
   const char *txt;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (sd->password) return NULL;

   txt = elm_widget_access_info_get(obj);

   if (!txt)
     return _elm_util_mkup_to_text(efl_ui_text_entry_get(obj));
   else return strdup(txt);
#else
   (void) data;
   (void) obj;
   return NULL;
#endif
}

static char *
_access_state_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   Eina_Strbuf *buf;
   char *ret;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   ret = NULL;
   buf = eina_strbuf_new();

   if (elm_widget_disabled_get(obj))
     eina_strbuf_append(buf, "State: Disabled");

   if (!sd->editable)
     {
        if (!eina_strbuf_length_get(buf))
          eina_strbuf_append(buf, "State: Not Editable");
        else eina_strbuf_append(buf, ", Not Editable");
     }

   if (sd->password)
     {
        if (!eina_strbuf_length_get(buf))
          eina_strbuf_append(buf, "State: Password");
        else eina_strbuf_append(buf, ", Password");
     }

   if (!eina_strbuf_length_get(buf)) goto buf_free;

   ret = eina_strbuf_string_steal(buf);

buf_free:
   eina_strbuf_free(buf);
   return ret;
}

static void
_efl_ui_text_resize_internal(Evas_Object *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (sd->line_wrap)
     {
        elm_layout_sizing_eval(obj);
     }
   else if (sd->scroll)
     {
        Evas_Coord vw = 0, vh = 0;

        elm_interface_scrollable_content_viewport_geometry_get
              (obj, NULL, NULL, &vw, &vh);
        evas_object_resize(sd->entry_edje, vw, vh);
     }

   if (sd->hoversel) _hoversel_position(obj);
   _decoration_defer_all(obj);
}

static void
_resize_cb(void *data,
           Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   _efl_ui_text_resize_internal(data);
}

static void
_selection_handlers_offset_calc(Evas_Object *obj, Evas_Object *handler, Evas_Coord canvasx, Evas_Coord canvasy)
{
   Evas_Coord ex, ey;
   Evas_Coord cx, cy, cw, ch;
   Evas_Coord hh;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   evas_object_geometry_get(sd->entry_edje, &ex, &ey, NULL, NULL);
   efl_text_cursor_geometry_get(obj, 
         efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN),
         EFL_TEXT_CURSOR_TYPE_BEFORE,
         &cx, &cy, &cw, &ch,
         NULL, NULL, NULL, NULL);
   edje_object_size_min_calc(handler, NULL, &hh);

   sd->ox = canvasx - (ex + cx + (cw / 2));
   if (ch > hh)
     sd->oy = canvasy - (ey + cy + ch);
   else
     sd->oy = canvasy - (ey + cy + (ch / 2));

   ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
   sd->long_pressed = EINA_FALSE;
   if (_elm_config->magnifier_enable)
     {
        _magnifier_create(obj);
        _magnifier_show(obj);
        _magnifier_move(obj);
     }
}

static void
_start_handler_mouse_down_cb(void *data,
                             Evas *e EINA_UNUSED,
                             Evas_Object *obj EINA_UNUSED,
                             void *event_info)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   Evas_Event_Mouse_Down *ev = event_info;
   int start_pos, end_pos, pos;
   Efl_Text_Cursor_Cursor *sel_start, *sel_end;
   Efl_Text_Cursor_Cursor *main_cur;

   Eo *text_obj = edje_object_part_swallow_get(sd->entry_edje, "elm.text");

   sd->start_handler_down = EINA_TRUE;

   /* Get the cursors */
   efl_ui_text_interactive_selection_cursors_get(text_obj, &sel_start, &sel_end);
   main_cur = efl_text_cursor_get(text_obj, EFL_TEXT_CURSOR_GET_MAIN);

   start_pos = efl_text_cursor_position_get(obj, sel_start);
   end_pos = efl_text_cursor_position_get(obj, sel_end);

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
   efl_text_cursor_position_set(obj, main_cur, pos);
   _selection_handlers_offset_calc(data, sd->start_handler, ev->canvas.x, ev->canvas.y);
}

static void
_start_handler_mouse_up_cb(void *data,
                           Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   sd->start_handler_down = EINA_FALSE;
   if (_elm_config->magnifier_enable)
     _magnifier_hide(data);
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

   efl_text_cursor_coord_set(obj, sd->sel_handler_cursor, cx, cy);
   pos = efl_text_cursor_position_get(obj, sd->sel_handler_cursor);

   /* Set the main cursor. */
   efl_text_cursor_position_set(obj, efl_text_cursor_get(data, EFL_TEXT_CURSOR_GET_MAIN), pos);

   ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
   sd->long_pressed = EINA_FALSE;
   if (_elm_config->magnifier_enable)
     _magnifier_move(data);
}

static void
_end_handler_mouse_down_cb(void *data,
                           Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   Evas_Event_Mouse_Down *ev = event_info;
   Efl_Text_Cursor_Cursor *sel_start, *sel_end;
   Efl_Text_Cursor_Cursor *main_cur;
   int pos, start_pos, end_pos;

   sd->end_handler_down = EINA_TRUE;

   Eo *text_obj = edje_object_part_swallow_get(sd->entry_edje, "elm.text");

   efl_ui_text_interactive_selection_cursors_get(text_obj, &sel_start, &sel_end);
   main_cur = efl_text_cursor_get(text_obj, EFL_TEXT_CURSOR_GET_MAIN);

   start_pos = efl_text_cursor_position_get(obj, sel_start);
   end_pos = efl_text_cursor_position_get(obj, sel_end);

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

   efl_text_cursor_position_set(obj, main_cur, pos);
   _selection_handlers_offset_calc(data, sd->end_handler, ev->canvas.x, ev->canvas.y);
}

static void
_end_handler_mouse_up_cb(void *data,
                         Evas *e EINA_UNUSED,
                         Evas_Object *obj EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   sd->end_handler_down = EINA_FALSE;
   if (_elm_config->magnifier_enable)
     _magnifier_hide(data);
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

   efl_text_cursor_coord_set(obj, sd->sel_handler_cursor, cx, cy);
   pos = efl_text_cursor_position_get(obj, sd->sel_handler_cursor);
   /* Set the main cursor. */
   efl_text_cursor_position_set(obj, efl_text_cursor_get(data, EFL_TEXT_CURSOR_GET_MAIN), pos);
   ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
   sd->long_pressed = EINA_FALSE;
   if (_elm_config->magnifier_enable)
     _magnifier_move(data);
}

EOLIAN static void
_efl_ui_text_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Text_Data *priv)
{
   Eo *text_obj;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   /* XXX: needs to be before efl_canvas_group_add, since the latter will
    * trigger a layout_sizing_eval call and requires the canvas text object to
    * be instantiated. */
   text_obj = efl_add(EFL_UI_INTERNAL_TEXT_INTERACTIVE_CLASS, obj);
   efl_composite_attach(obj, text_obj);

   // FIXME: use the theme, when a proper theming option is available
   //  (possibly, text_classes).
   // For now, set this for easier setup
   efl_text_font_set(text_obj, "Sans", 12);
   efl_text_normal_color_set(text_obj, 255, 255, 255, 255);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->test_bit = EINA_TRUE;
   priv->entry_edje = wd->resize_obj;

   priv->cnp_mode = ELM_CNP_MODE_PLAINTEXT;
   priv->line_wrap = ELM_WRAP_WORD;
   priv->context_menu = EINA_TRUE;
   priv->auto_save = EINA_TRUE;
   priv->editable = EINA_TRUE;
   priv->sel_allow = EINA_TRUE;

   priv->single_line = !efl_text_multiline_get(text_obj);

   priv->drop_format = ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE;
   elm_drop_target_add(obj, priv->drop_format,
                       _dnd_enter_cb, NULL,
                       _dnd_leave_cb, NULL,
                       _dnd_pos_cb, NULL,
                       _dnd_drop_cb, NULL);

   if (!elm_layout_theme_set(obj, "efl_ui_text", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   edje_object_part_swallow(priv->entry_edje, "elm.text", text_obj);
   evas_object_size_hint_weight_set
      (priv->entry_edje, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
      (priv->entry_edje, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_event_callback_add(text_obj, EFL_UI_TEXT_INTERACTIVE_EVENT_CHANGED_USER,
         _efl_ui_text_changed_cb, obj);
   efl_event_callback_add(text_obj, EFL_CANVAS_TEXT_EVENT_CHANGED,
         _efl_ui_text_changed_cb, obj);
   efl_event_callback_add(text_obj, EFL_UI_TEXT_INTERACTIVE_EVENT_SELECTION_CHANGED,
         _efl_ui_text_selection_changed_cb, obj);
   efl_event_callback_add(text_obj, EFL_CANVAS_TEXT_EVENT_CURSOR_CHANGED,
         _efl_ui_text_cursor_changed_cb, obj);
   evas_object_event_callback_add(priv->entry_edje, EVAS_CALLBACK_MOVE,
         _efl_ui_text_move_cb, obj);


   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_data_set(priv->hit_rect, "_elm_leaveme", obj);

   Evas_Object* clip = evas_object_clip_get(priv->entry_edje);
   evas_object_clip_set(priv->hit_rect, clip);

   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   elm_interface_scrollable_objects_set(obj, priv->entry_edje, priv->hit_rect);

   edje_object_text_markup_filter_callback_add
     (priv->entry_edje, "elm.text", _markup_filter_cb, obj);

   evas_object_event_callback_add
     (priv->entry_edje, EVAS_CALLBACK_KEY_DOWN, _key_down_cb, obj);
   evas_object_event_callback_add
     (priv->entry_edje, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, obj);
   evas_object_event_callback_add
     (priv->entry_edje, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, obj);
   evas_object_event_callback_add
     (priv->entry_edje, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, obj);

   /* this code can't go in smart_resize. sizing gets wrong */
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize_cb, obj);

   edje_object_signal_callback_add
     (priv->entry_edje, "entry,changed,user", "elm.text",
     _entry_changed_user_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "preedit,changed", "elm.text",
     _entry_preedit_changed_signal_cb, obj);

   edje_object_signal_callback_add
     (priv->entry_edje, "cursor,changed", "elm.text",
     _entry_cursor_changed_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "cursor,changed,manual", "elm.text",
     _entry_cursor_changed_manual_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "anchor,mouse,down,*", "elm.text",
     _entry_anchor_down_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "anchor,mouse,up,*", "elm.text",
     _entry_anchor_up_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "anchor,mouse,move,*", "elm.text",
     _entry_anchor_move_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "anchor,mouse,in,*", "elm.text",
     _entry_anchor_in_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "anchor,mouse,out,*", "elm.text",
     _entry_anchor_out_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "entry,key,enter", "elm.text",
     _entry_key_enter_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "entry,key,escape", "elm.text",
     _entry_key_escape_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "mouse,down,1", "elm.text",
     _entry_mouse_down_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "mouse,clicked,1", "elm.text",
     _entry_mouse_clicked_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "mouse,down,1,double", "elm.text",
     _entry_mouse_double_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "mouse,down,1,triple", "elm.text",
     _entry_mouse_triple_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "entry,undo,request", "elm.text",
     _entry_undo_request_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "entry,redo,request", "elm.text",
     _entry_redo_request_signal_cb, obj);

   elm_layout_text_set(obj, "elm.text", "");

   elm_object_sub_cursor_set
     (wd->resize_obj, obj, ELM_CURSOR_XTERM);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   if (_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set
       (priv->entry_edje, "elm.text", EINA_TRUE);

   elm_layout_sizing_eval(obj);

   efl_ui_text_input_panel_layout_set(obj, ELM_INPUT_PANEL_LAYOUT_NORMAL);
   efl_ui_text_input_panel_enabled_set(obj, EINA_TRUE);
   efl_ui_text_prediction_allow_set(obj, EINA_TRUE);
   efl_ui_text_input_hint_set(obj, ELM_INPUT_HINT_AUTO_COMPLETE);

   priv->autocapital_type = (Elm_Autocapital_Type)edje_object_part_text_autocapital_type_get
       (priv->entry_edje, "elm.text");

   entries = eina_list_prepend(entries, obj);

   // module - find module for entry
   priv->api = _module_find(obj);
   // if found - hook in
   if ((priv->api) && (priv->api->obj_hook)) priv->api->obj_hook(obj);

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   // access
   _elm_access_object_register(obj, priv->entry_edje);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Entry"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, NULL);

   if (_elm_config->desktop_entry)
     priv->sel_handler_disabled = EINA_TRUE;

   _create_text_cursors(obj, priv);
}

static void
_create_selection_handlers(Evas_Object *obj, Efl_Ui_Text_Data *sd,
      const char *file)
{
   Evas_Object *handle;

   handle = _decoration_create(sd, file, "elm/entry/handler/start", EINA_TRUE);
   evas_object_pass_events_set(handle, EINA_FALSE);
   sd->start_handler = handle;
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_DOWN,
                                  _start_handler_mouse_down_cb, obj);
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_MOVE,
                                  _start_handler_mouse_move_cb, obj);
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_UP,
                                  _start_handler_mouse_up_cb, obj);
   evas_object_show(handle);

   handle = _decoration_create(sd, file, "elm/entry/handler/end", EINA_TRUE);
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
_efl_ui_text_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Text_Data *sd)
{
   Elm_Entry_Context_Menu_Item *it;
   Elm_Entry_Item_Provider *ip;
   Elm_Entry_Markup_Filter *tf;
   Eo *text_obj;

   if (sd->delay_write)
     {
        ELM_SAFE_FREE(sd->delay_write, ecore_timer_del);
        if (sd->auto_save) _save_do(obj);
     }

   if (sd->scroll)
     elm_interface_scrollable_content_viewport_resize_cb_set(obj, NULL);

   _efl_ui_text_anchor_hover_end(obj, sd);
   _efl_ui_text_anchor_hover_parent_set(obj, sd, NULL);

   evas_event_freeze(evas_object_evas_get(obj));

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
   ecore_job_del(sd->deferred_recalc_job);
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
   EINA_LIST_FREE(sd->item_providers, ip)
     {
        free(ip);
     }
   EINA_LIST_FREE(sd->markup_filters, tf)
     {
        _filter_free(tf);
     }
   ELM_SAFE_FREE(sd->delay_write, ecore_timer_del);
   free(sd->input_panel_imdata);
   eina_stringshare_del(sd->anchor_hover.hover_style);

   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   if (sd->start_handler)
     {
        evas_object_del(sd->start_handler);
        evas_object_del(sd->end_handler);
     }

   _anchors_clear_all(obj, sd);
   _unused_item_objs_free(sd);
   _clear_text_selection(sd);

   text_obj = edje_object_part_swallow_get(sd->entry_edje, "elm.text");
   efl_event_callback_del(text_obj, EFL_UI_TEXT_INTERACTIVE_EVENT_CHANGED_USER,
         _efl_ui_text_changed_cb, obj);
   efl_event_callback_del(text_obj, EFL_CANVAS_TEXT_EVENT_CHANGED,
         _efl_ui_text_changed_cb, obj);
   efl_event_callback_del(text_obj, EFL_UI_TEXT_INTERACTIVE_EVENT_SELECTION_CHANGED,
         _efl_ui_text_selection_changed_cb, obj);
   efl_event_callback_del(text_obj, EFL_CANVAS_TEXT_EVENT_CURSOR_CHANGED,
         _efl_ui_text_cursor_changed_cb, obj);
   evas_object_event_callback_del_full(sd->entry_edje, EVAS_CALLBACK_MOVE,
         _efl_ui_text_move_cb, obj);

   // XXX: explicitly delete the object, as it's been reparented to the canvas, due to
   // a specific behavior of SWALLOW parts.
   efl_del(sd->text_obj);
   sd->text_obj = NULL;
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_text_efl_gfx_position_set(Eo *obj, Efl_Ui_Text_Data *sd, Evas_Coord x, Evas_Coord y)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, x, y))
     return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);

   evas_object_move(sd->hit_rect, x, y);

   if (sd->hoversel) _hoversel_position(obj);

   _update_selection_handler(obj);
}

EOLIAN static void
_efl_ui_text_efl_gfx_size_set(Eo *obj, Efl_Ui_Text_Data *sd, Evas_Coord w, Evas_Coord h)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, w, h))
     return;

   evas_object_resize(sd->hit_rect, w, h);
   _update_selection_handler(obj);

   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);
}

EOLIAN static void
_efl_ui_text_efl_gfx_visible_set(Eo *obj, Efl_Ui_Text_Data *sd EINA_UNUSED, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_visible_set(efl_super(obj, MY_CLASS), vis);
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
   const Efl_Callback_Array_Item *event = ev->info;

   EFL_UI_TEXT_DATA_GET(ev->object, sd);
   if (event->desc == EFL_UI_TEXT_EVENT_VALIDATE)
     sd->validators++;
}

static void
_cb_deleted(void *data EINA_UNUSED, const Efl_Event *ev)
{
   const Efl_Callback_Array_Item *event = ev->info;

   EFL_UI_TEXT_DATA_GET(ev->object, sd);
   if (event->desc == EFL_UI_TEXT_EVENT_VALIDATE)
     sd->validators--;
   return;

}

EOLIAN static Eo *
_efl_ui_text_efl_object_constructor(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_ENTRY);
   efl_event_callback_add(obj, EFL_EVENT_CALLBACK_ADD, _cb_added, NULL);
   efl_event_callback_add(obj, EFL_EVENT_CALLBACK_DEL, _cb_deleted, NULL);
   efl_ui_text_interactive_editable_set(obj, EINA_FALSE);

   return obj;
}

EOLIAN static void
_efl_ui_text_efl_object_destructor(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *pd)
{
   ecore_job_del(pd->deferred_decoration_job);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_text_password_set(Eo *obj, Efl_Ui_Text_Data *sd, Eina_Bool password)
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
        elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_PASSWORD_TEXT);
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
        elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_ENTRY);
     }

   elm_obj_widget_theme_apply(obj);
}

EOLIAN static Eina_Bool
_efl_ui_text_password_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->password;
}

static void
_efl_ui_text_calc_force(Eo *obj, Efl_Ui_Text_Data *sd)
{
   edje_object_calc_force(sd->entry_edje);
   sd->changed = EINA_TRUE;
   elm_layout_sizing_eval(obj);
}

static const char*
_efl_ui_text_selection_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   Efl_Text_Cursor_Cursor *start_obj, *end_obj;

   if ((sd->password)) return NULL;

   efl_ui_text_interactive_selection_cursors_get(obj, &start_obj, &end_obj);
   return efl_canvas_text_range_text_get(obj, start_obj, end_obj);
}

EOLIAN static void
_efl_ui_text_selection_handler_disabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Eina_Bool disabled)
{
   if (sd->sel_handler_disabled == disabled) return;
   sd->sel_handler_disabled = disabled;
}

EOLIAN static Eina_Bool
_efl_ui_text_selection_handler_disabled_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->sel_handler_disabled;
}

static void
_efl_ui_text_entry_insert(Eo *obj, Efl_Ui_Text_Data *sd, const char *entry)
{
   Efl_Text_Cursor_Cursor *cur_obj = efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN);
   efl_text_cursor_text_insert(obj, cur_obj, entry);
   sd->changed = EINA_TRUE;
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_text_efl_ui_text_interactive_editable_set(Eo *obj, Efl_Ui_Text_Data *sd, Eina_Bool editable)
{
   efl_ui_text_interactive_editable_set(efl_super(obj, MY_CLASS), editable);
   if (sd->editable == editable) return;
   sd->editable = editable;
   elm_obj_widget_theme_apply(obj);

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
            (sd->entry_edje, "elm.text", EINA_FALSE);
        edje_object_signal_emit(sd->entry_edje, "elm,state,select,off", "elm");
     }
   if (sd->have_selection)
     efl_event_callback_legacy_call
       (obj, EFL_UI_EVENT_SELECTION_CLEARED, NULL);

   sd->have_selection = EINA_FALSE;
   edje_object_part_text_select_none(sd->entry_edje, "elm.text");

   _hide_selection_handler(obj);
}

static void
_efl_ui_text_select_region_set(Eo *obj, Efl_Ui_Text_Data *sd, int start, int end)
{
   Efl_Text_Cursor_Cursor *sel_start, *sel_end;

   if ((sd->password)) return;

   efl_ui_text_interactive_selection_cursors_get(obj, &sel_start, &sel_end);

   efl_text_cursor_position_set(obj, sel_start, start);
   efl_text_cursor_position_set(obj, sel_end, end);
}

EOLIAN static void
_efl_ui_text_cursor_selection_end(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   edje_object_part_text_select_extend(sd->entry_edje, "elm.text");
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
_efl_ui_text_context_menu_disabled_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return !sd->context_menu;
}

EOLIAN static void
_efl_ui_text_item_provider_append(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Entry_Item_Provider_Cb func, void *data)
{
   Elm_Entry_Item_Provider *ip;

   EINA_SAFETY_ON_NULL_RETURN(func);

   ip = calloc(1, sizeof(Elm_Entry_Item_Provider));
   if (!ip) return;

   ip->func = func;
   ip->data = data;
   sd->item_providers = eina_list_append(sd->item_providers, ip);
}

EOLIAN static void
_efl_ui_text_item_provider_prepend(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Entry_Item_Provider_Cb func, void *data)
{
   Elm_Entry_Item_Provider *ip;

   EINA_SAFETY_ON_NULL_RETURN(func);

   ip = calloc(1, sizeof(Elm_Entry_Item_Provider));
   if (!ip) return;

   ip->func = func;
   ip->data = data;
   sd->item_providers = eina_list_prepend(sd->item_providers, ip);
}

EOLIAN static void
_efl_ui_text_item_provider_remove(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Entry_Item_Provider_Cb func, void *data)
{
   Eina_List *l;
   Elm_Entry_Item_Provider *ip;

   EINA_SAFETY_ON_NULL_RETURN(func);

   EINA_LIST_FOREACH(sd->item_providers, l, ip)
     {
        if ((ip->func == func) && ((!data) || (ip->data == data)))
          {
             sd->item_providers = eina_list_remove_list(sd->item_providers, l);
             free(ip);
             return;
          }
     }
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
_efl_ui_text_efl_file_file_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, const char **file, const char **group)
{
   if (file) *file = sd->file;
   if (group) *group = NULL;
}

EOLIAN static void
_efl_ui_text_cnp_mode_set(Eo *obj, Efl_Ui_Text_Data *sd, Elm_Cnp_Mode cnp_mode)
{
   Elm_Sel_Format format = ELM_SEL_FORMAT_MARKUP;


   if (sd->cnp_mode == cnp_mode) return;
   sd->cnp_mode = cnp_mode;
   if (sd->cnp_mode == ELM_CNP_MODE_PLAINTEXT)
     format = ELM_SEL_FORMAT_TEXT;
   else if (cnp_mode == ELM_CNP_MODE_MARKUP)
     format |= ELM_SEL_FORMAT_IMAGE;

   elm_drop_target_del(obj, sd->drop_format,
                       _dnd_enter_cb, NULL,
                       _dnd_leave_cb, NULL,
                       _dnd_pos_cb, NULL,
                       _dnd_drop_cb, NULL);
   sd->drop_format = format;
   elm_drop_target_add(obj, sd->drop_format,
                       _dnd_enter_cb, NULL,
                       _dnd_leave_cb, NULL,
                       _dnd_pos_cb, NULL,
                       _dnd_drop_cb, NULL);
}

EOLIAN static Elm_Cnp_Mode
_efl_ui_text_cnp_mode_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->cnp_mode;
}

static void
_efl_ui_text_content_viewport_resize_cb(Evas_Object *obj,
                                      Evas_Coord w EINA_UNUSED, Evas_Coord h EINA_UNUSED)
{
   _efl_ui_text_resize_internal(obj);
}

static void
_scroll_cb(Evas_Object *obj, void *data EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);
   /* here we need to emit the signal that the elm_scroller would have done */
   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SCROLL, NULL);

   if (sd->have_selection)
     _update_selection_handler(obj);
}

EOLIAN static void
_efl_ui_text_scrollable_set(Eo *obj, Efl_Ui_Text_Data *sd, Eina_Bool scroll)
{
   scroll = !!scroll;
   if (sd->scroll == scroll) return;
   sd->scroll = scroll;

   if (sd->scroll)
     {
        /* we now must re-theme ourselves to a scroller decoration
         * and move the entry looking object to be the content of the
         * scrollable view */
        elm_widget_resize_object_set(obj, NULL, EINA_TRUE);
        elm_widget_sub_object_add(obj, sd->entry_edje);

        if (!sd->scr_edje)
          {
             sd->scr_edje = edje_object_add(evas_object_evas_get(obj));

             elm_widget_theme_object_set
               (obj, sd->scr_edje, "scroller", "entry",
               elm_widget_style_get(obj));

             evas_object_size_hint_weight_set
               (sd->scr_edje, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set
               (sd->scr_edje, EVAS_HINT_FILL, EVAS_HINT_FILL);

             evas_object_propagate_events_set(sd->scr_edje, EINA_TRUE);
          }

        elm_widget_resize_object_set(obj, sd->scr_edje, EINA_TRUE);

        elm_interface_scrollable_objects_set(obj, sd->scr_edje, sd->hit_rect);

        elm_interface_scrollable_scroll_cb_set(obj, _scroll_cb);

        elm_interface_scrollable_bounce_allow_set(obj, sd->h_bounce, sd->v_bounce);
        if (sd->single_line)
           elm_interface_scrollable_policy_set(obj, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
        else
           elm_interface_scrollable_policy_set(obj, sd->policy_h, sd->policy_v);
        elm_interface_scrollable_content_set(obj, sd->entry_edje);
        elm_interface_scrollable_content_viewport_resize_cb_set(obj, _efl_ui_text_content_viewport_resize_cb);
        elm_widget_on_show_region_hook_set(obj, _show_region_hook, NULL);
     }
   else
     {
        if (sd->scr_edje)
          {
             elm_interface_scrollable_content_set(obj, NULL);
             evas_object_hide(sd->scr_edje);
          }
        elm_widget_resize_object_set(obj, sd->entry_edje, EINA_TRUE);

        if (sd->scr_edje)
          elm_widget_sub_object_add(obj, sd->scr_edje);

        elm_interface_scrollable_objects_set(obj, sd->entry_edje, sd->hit_rect);

        elm_widget_on_show_region_hook_set(obj, NULL, NULL);
     }
   _update_decorations(obj);
   elm_obj_widget_theme_apply(obj);
}

EOLIAN static Eina_Bool
_efl_ui_text_scrollable_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->scroll;
}

EOLIAN static void
_efl_ui_text_elm_interface_scrollable_policy_set(Eo *obj, Efl_Ui_Text_Data *sd, Elm_Scroller_Policy h, Elm_Scroller_Policy v)
{
   sd->policy_h = h;
   sd->policy_v = v;
   elm_interface_scrollable_policy_set(efl_super(obj, MY_CLASS), sd->policy_h, sd->policy_v);
}

EOLIAN static void
_efl_ui_text_elm_interface_scrollable_bounce_allow_set(Eo *obj, Efl_Ui_Text_Data *sd, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   sd->h_bounce = h_bounce;
   sd->v_bounce = v_bounce;
   elm_interface_scrollable_bounce_allow_set(efl_super(obj, MY_CLASS), h_bounce, v_bounce);
}

EOLIAN static void
_efl_ui_text_input_panel_layout_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Input_Panel_Layout layout)
{
   sd->input_panel_layout = layout;

   edje_object_part_text_input_panel_layout_set
     (sd->entry_edje, "elm.text", (Edje_Input_Panel_Layout)layout);

   if (layout == ELM_INPUT_PANEL_LAYOUT_PASSWORD)
     efl_ui_text_input_hint_set(obj, ((sd->input_hints & ~ELM_INPUT_HINT_AUTO_COMPLETE) | ELM_INPUT_HINT_SENSITIVE_DATA));
   else if (layout == ELM_INPUT_PANEL_LAYOUT_TERMINAL)
     efl_ui_text_input_hint_set(obj, (sd->input_hints & ~ELM_INPUT_HINT_AUTO_COMPLETE));
}

EOLIAN static Elm_Input_Panel_Layout
_efl_ui_text_input_panel_layout_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_panel_layout;
}

EOLIAN static void
_efl_ui_text_input_panel_layout_variation_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, int variation)
{
   sd->input_panel_layout_variation = variation;

   edje_object_part_text_input_panel_layout_variation_set
     (sd->entry_edje, "elm.text", variation);
}

EOLIAN static int
_efl_ui_text_input_panel_layout_variation_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_panel_layout_variation;
}

EOLIAN static void
_efl_ui_text_autocapital_type_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Autocapital_Type autocapital_type)
{
   sd->autocapital_type = autocapital_type;
   edje_object_part_text_autocapital_type_set
     (sd->entry_edje, "elm.text", (Edje_Text_Autocapital_Type)autocapital_type);
}

EOLIAN static Elm_Autocapital_Type
_efl_ui_text_autocapital_type_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->autocapital_type;
}

EOLIAN static void
_efl_ui_text_prediction_allow_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Eina_Bool prediction)
{
   sd->prediction_allow = prediction;
   edje_object_part_text_prediction_allow_set
     (sd->entry_edje, "elm.text", prediction);
}

EOLIAN static Eina_Bool
_efl_ui_text_prediction_allow_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->prediction_allow;
}

EOLIAN static void
_efl_ui_text_input_hint_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Input_Hints hints)
{
   sd->input_hints = hints;

   edje_object_part_text_input_hint_set
     (sd->entry_edje, "elm.text", (Edje_Input_Hints)hints);
}

EOLIAN static Elm_Input_Hints
_efl_ui_text_input_hint_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_hints;
}

EOLIAN static void
_efl_ui_text_input_panel_enabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Eina_Bool enabled)
{
   sd->input_panel_enable = enabled;
   edje_object_part_text_input_panel_enabled_set
     (sd->entry_edje, "elm.text", enabled);
}

EOLIAN static Eina_Bool
_efl_ui_text_input_panel_enabled_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_panel_enable;
}

EOLIAN static void
_efl_ui_text_input_panel_show(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   edje_object_part_text_input_panel_show(sd->entry_edje, "elm.text");
}

EOLIAN static void
_efl_ui_text_input_panel_hide(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{

   edje_object_part_text_input_panel_hide(sd->entry_edje, "elm.text");
}

EOLIAN static void
_efl_ui_text_input_panel_language_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Input_Panel_Lang lang)
{
   sd->input_panel_lang = lang;
   edje_object_part_text_input_panel_language_set
     (sd->entry_edje, "elm.text", (Edje_Input_Panel_Lang)lang);
}

EOLIAN static Elm_Input_Panel_Lang
_efl_ui_text_input_panel_language_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
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
     (sd->entry_edje, "elm.text", sd->input_panel_imdata,
     sd->input_panel_imdata_len);
}

EOLIAN static void
_efl_ui_text_input_panel_imdata_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, void *data, int *len)
{
   edje_object_part_text_input_panel_imdata_get
     (sd->entry_edje, "elm.text", data, len);
}

EOLIAN static void
_efl_ui_text_input_panel_return_key_type_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Elm_Input_Panel_Return_Key_Type return_key_type)
{
   sd->input_panel_return_key_type = return_key_type;

   edje_object_part_text_input_panel_return_key_type_set
     (sd->entry_edje, "elm.text", (Edje_Input_Panel_Return_Key_Type)return_key_type);
}

EOLIAN static Elm_Input_Panel_Return_Key_Type
_efl_ui_text_input_panel_return_key_type_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_panel_return_key_type;
}

EOLIAN static void
_efl_ui_text_input_panel_return_key_disabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd, Eina_Bool disabled)
{
   sd->input_panel_return_key_disabled = disabled;

   edje_object_part_text_input_panel_return_key_disabled_set
     (sd->entry_edje, "elm.text", disabled);
}

EOLIAN static Eina_Bool
_efl_ui_text_input_panel_return_key_disabled_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
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
     (sd->entry_edje, "elm.text", ondemand);
}

EOLIAN static Eina_Bool
_efl_ui_text_input_panel_show_on_demand_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   return sd->input_panel_show_on_demand;
}

/* START - ANCHOR HOVER */
static void
_anchor_parent_del_cb(void *data,
                      Evas *e EINA_UNUSED,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   sd->anchor_hover.hover_parent = NULL;
}

static void
_efl_ui_text_anchor_hover_parent_set(Eo *obj, Efl_Ui_Text_Data *sd, Evas_Object *parent)
{
   if (sd->anchor_hover.hover_parent)
     evas_object_event_callback_del_full
       (sd->anchor_hover.hover_parent, EVAS_CALLBACK_DEL,
       _anchor_parent_del_cb, obj);
   sd->anchor_hover.hover_parent = parent;
   if (sd->anchor_hover.hover_parent)
     evas_object_event_callback_add
       (sd->anchor_hover.hover_parent, EVAS_CALLBACK_DEL,
       _anchor_parent_del_cb, obj);
}

static void
_efl_ui_text_anchor_hover_end(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   ELM_SAFE_FREE(sd->anchor_hover.hover, evas_object_del);
   ELM_SAFE_FREE(sd->anchor_hover.pop, evas_object_del);
}
/* END - ANCHOR HOVER */

EOLIAN static Eina_Bool
_efl_ui_text_elm_widget_activate(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, Elm_Activate act)
{
   if (act != ELM_ACTIVATE_DEFAULT) return EINA_FALSE;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (!elm_widget_disabled_get(obj) &&
       !evas_object_freeze_events_get(obj))
     {
        efl_event_callback_legacy_call
          (obj, EFL_UI_EVENT_CLICKED, NULL);
        if (sd->editable && sd->input_panel_enable)
          edje_object_part_text_input_panel_show(sd->entry_edje, "elm.text");
     }
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

static void
_efl_ui_text_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

// ATSPI Accessibility

EOLIAN static Eina_Unicode
_efl_ui_text_elm_interface_atspi_text_character_get(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, int offset)
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
_efl_ui_text_elm_interface_atspi_text_character_count_get(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   const char *txt;

   txt = efl_text_get(obj);
   if (!txt) return -1;
   return eina_unicode_utf8_get_len(txt);
}

EOLIAN static char*
_efl_ui_text_elm_interface_atspi_text_string_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *pd, Elm_Atspi_Text_Granularity granularity, int *start_offset, int *end_offset)
{
   Evas_Textblock_Cursor *cur = NULL, *cur2 = NULL;
   char *ret = NULL;
   Eo *text_obj = edje_object_part_swallow_get(pd->entry_edje, "elm.text");

   cur = evas_object_textblock_cursor_new(text_obj);
   cur2 = evas_object_textblock_cursor_new(text_obj);
   if (!cur || !cur2) goto fail;

   evas_textblock_cursor_pos_set(cur, *start_offset);
   if (evas_textblock_cursor_pos_get(cur) != *start_offset) goto fail;

   switch (granularity)
     {
      case ELM_ATSPI_TEXT_GRANULARITY_CHAR:
         break;
      case ELM_ATSPI_TEXT_GRANULARITY_WORD:
         evas_textblock_cursor_word_start(cur);
         break;
      case ELM_ATSPI_TEXT_GRANULARITY_SENTENCE:
         // TODO - add sentence support in textblock first
         break;
      case ELM_ATSPI_TEXT_GRANULARITY_LINE:
         evas_textblock_cursor_line_char_first(cur);
         break;
      case ELM_ATSPI_TEXT_GRANULARITY_PARAGRAPH:
         evas_textblock_cursor_paragraph_char_first(cur);
         break;
     }

   *start_offset = evas_textblock_cursor_pos_get(cur);
   evas_textblock_cursor_copy(cur, cur2);

   switch (granularity)
     {
      case ELM_ATSPI_TEXT_GRANULARITY_CHAR:
         evas_textblock_cursor_char_next(cur2);
         break;
      case ELM_ATSPI_TEXT_GRANULARITY_WORD:
         evas_textblock_cursor_word_end(cur2);
         // since word_end sets cursor position ON (before) last
         // char of word, we need to manually advance cursor to get
         // proper string from function range_text_get
         evas_textblock_cursor_char_next(cur2);
         break;
      case ELM_ATSPI_TEXT_GRANULARITY_SENTENCE:
         // TODO - add sentence support in textblock first
         break;
      case ELM_ATSPI_TEXT_GRANULARITY_LINE:
         evas_textblock_cursor_line_char_last(cur2);
         break;
      case ELM_ATSPI_TEXT_GRANULARITY_PARAGRAPH:
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
_efl_ui_text_elm_interface_atspi_text_text_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *pd EINA_UNUSED, int start_offset, int end_offset)
{
   Evas_Textblock_Cursor *cur = NULL, *cur2 = NULL;
   char *ret = NULL;
   Eo *text_obj = edje_object_part_swallow_get(pd->entry_edje, "elm.text");

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
_efl_ui_text_elm_interface_atspi_text_caret_offset_get(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   return efl_text_cursor_position_get(obj, efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN));
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_interface_atspi_text_caret_offset_set(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, int offset)
{
   efl_text_cursor_position_set(obj, efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN), offset);
   return EINA_TRUE;
}

EOLIAN static int
_efl_ui_text_elm_interface_atspi_text_selections_count_get(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   return _efl_ui_text_selection_get(obj, _pd) ? 1 : 0;
}

EOLIAN static void
_efl_ui_text_elm_interface_atspi_text_selection_get(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, int selection_number, int *start_offset, int *end_offset)
{
   if (selection_number != 0) return;

   elm_obj_entry_select_region_get(obj, start_offset, end_offset);
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_interface_atspi_text_selection_set(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, int selection_number, int start_offset, int end_offset)
{
   if (selection_number != 0) return EINA_FALSE;

   _efl_ui_text_select_region_set(obj, _pd, start_offset, end_offset);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_interface_atspi_text_selection_remove(Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, int selection_number)
{
   if (selection_number != 0) return EINA_FALSE;
   _efl_ui_text_select_none(obj, pd);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_interface_atspi_text_selection_add(Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, int start_offset, int end_offset)
{
   _efl_ui_text_select_region_set(obj, pd, start_offset, end_offset);

   return EINA_TRUE;
}

EOLIAN static Eina_List*
_efl_ui_text_elm_interface_atspi_text_bounded_ranges_get(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *_pd EINA_UNUSED, Eina_Bool screen_coods EINA_UNUSED, Eina_Rectangle rect EINA_UNUSED, Elm_Atspi_Text_Clip_Type xclip EINA_UNUSED, Elm_Atspi_Text_Clip_Type yclip EINA_UNUSED)
{
   return NULL;
}

EOLIAN static int
_efl_ui_text_elm_interface_atspi_text_offset_at_point_get(Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, Eina_Bool screen_coods, int x, int y)
{
   Evas_Textblock_Cursor *cur;
   int ret;
   Eo *text_obj = edje_object_part_swallow_get(pd->entry_edje, "elm.text");

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
_efl_ui_text_elm_interface_atspi_text_character_extents_get(Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, int offset, Eina_Bool screen_coods, Eina_Rectangle *rect)
{
   Evas_Textblock_Cursor *cur;
   int ret;
   Eo *text_obj = edje_object_part_swallow_get(pd->entry_edje, "elm.text");

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
_efl_ui_text_elm_interface_atspi_text_range_extents_get(Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, Eina_Bool screen_coods, int start_offset, int end_offset, Eina_Rectangle *rect)
{
   Evas_Textblock_Cursor *cur1, *cur2;
   int ret;
   int x, xx, y, yy;
   Eo *text_obj = edje_object_part_swallow_get(pd->entry_edje, "elm.text");

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

static Elm_Atspi_Text_Attribute*
_textblock_node_format_to_atspi_text_attr(Eo *obj,
      Efl_Text_Annotate_Annotation *annotation)
{
   Elm_Atspi_Text_Attribute *ret;
   const char *txt;

   txt = efl_text_annotation_get(obj, annotation);
   if (!txt) return NULL;

   ret = calloc(1, sizeof(Elm_Atspi_Text_Attribute));
   if (!ret) return NULL;

   ret->value = eina_stringshare_add(txt);
   int size = strlen(txt);
   ret->name = eina_stringshare_add_length(txt, size);

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_interface_atspi_text_attribute_get(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, const char *attr_name EINA_UNUSED, int *start_offset, int *end_offset, char **value)
{
   Evas_Textblock_Cursor *cur1, *cur2;
   Elm_Atspi_Text_Attribute *attr;
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
_efl_ui_text_elm_interface_atspi_text_attributes_get(Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, int *start_offset, int *end_offset)
{
   Evas_Textblock_Cursor *cur1, *cur2;
   Eina_List *ret = NULL;
   Elm_Atspi_Text_Attribute *attr;
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
_efl_ui_text_elm_interface_atspi_text_default_attributes_get(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   Eina_List *ret = NULL;
   Elm_Atspi_Text_Attribute *attr;
   Efl_Text_Cursor_Cursor *start, *end;
   Eina_Iterator *annotations;
   Efl_Text_Annotate_Annotation *an;

   /* Retrieve all annotations in the text. */
   start = efl_text_cursor_new(obj);
   end = efl_text_cursor_new(obj);

   efl_text_cursor_paragraph_first(obj, start);
   efl_text_cursor_paragraph_last(obj, end);

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
_efl_ui_text_elm_interface_atspi_text_editable_content_set(Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, const char *content)
{
   efl_text_set(obj, content);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_interface_atspi_text_editable_insert(Eo *obj, Efl_Ui_Text_Data *pd, const char *string, int position)
{
   Efl_Text_Cursor_Cursor *cur_obj = efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN);
   efl_text_cursor_position_set(obj, cur_obj, position);
   _efl_ui_text_entry_insert(obj, pd, string);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_interface_atspi_text_editable_copy(Eo *obj, Efl_Ui_Text_Data *pd, int start, int end)
{
   _efl_ui_text_select_region_set(obj, pd, start, end);
   efl_ui_text_selection_copy(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_interface_atspi_text_editable_delete(Eo *obj, Efl_Ui_Text_Data *pd, int start_offset, int end_offset)
{
   Evas_Textblock_Cursor *cur1, *cur2;
   Eo *text_obj = edje_object_part_swallow_get(pd->entry_edje, "elm.text");

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
_efl_ui_text_elm_interface_atspi_text_editable_paste(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED, int position)
{
   Efl_Text_Cursor_Cursor *cur_obj = efl_text_cursor_get(obj, EFL_TEXT_CURSOR_GET_MAIN);
   efl_text_cursor_position_set(obj, cur_obj, position);
   efl_ui_text_selection_paste(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_elm_interface_atspi_text_editable_cut(Eo *obj, Efl_Ui_Text_Data *pd EINA_UNUSED, int start, int end)
{
   _efl_ui_text_select_region_set(obj, pd, start, end);
   efl_ui_text_selection_cut(obj);
   return EINA_TRUE;
}

EOLIAN static Elm_Atspi_State_Set
_efl_ui_text_elm_interface_atspi_accessible_state_set_get(Eo *obj, Efl_Ui_Text_Data *_pd EINA_UNUSED)
{
   Elm_Atspi_State_Set ret;
   ret = elm_interface_atspi_accessible_state_set_get(efl_super(obj, EFL_UI_TEXT_CLASS));

   if (efl_ui_text_interactive_editable_get(obj))
     STATE_TYPE_SET(ret, ELM_ATSPI_STATE_EDITABLE);

   return ret;
}

EOLIAN static const char*
_efl_ui_text_elm_interface_atspi_accessible_name_get(Eo *obj, Efl_Ui_Text_Data *pd)
{
   const char *name;
   name = elm_interface_atspi_accessible_name_get(efl_super(obj, EFL_UI_TEXT_CLASS));
   if (name && strncmp("", name, 1)) return name;
   const char *ret = edje_object_part_text_get(pd->entry_edje, "elm.guide");
   return ret;
}

EOLIAN static Efl_Text_Cursor_Cursor *
_efl_ui_text_cursor_new(Eo *obj EINA_UNUSED, Efl_Ui_Text_Data *sd EINA_UNUSED)
{
   Eo *text_obj = edje_object_part_swallow_get(sd->entry_edje, "elm.text");
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
_decoration_create(Efl_Ui_Text_Data *sd, const char *file,
      const char *source, Eina_Bool above)
{
   Eo *ret = efl_add(EDJE_OBJECT_CLASS, sd->entry_edje);
   Eo *text_obj = edje_object_part_swallow_get(sd->entry_edje, "elm.text");
   edje_object_file_set(ret, file, source);
   evas_object_smart_member_add(ret, sd->entry_edje);
   if (above)
     {
        evas_object_stack_above(ret, text_obj);
     }
   else
     {
        evas_object_stack_below(ret, text_obj);
     }
   evas_object_clip_set(ret, evas_object_clip_get(text_obj));
   evas_object_pass_events_set(ret, EINA_TRUE);
   return ret;
}

/**
 * Creates the cursors, if not created.
 */

static void
_create_text_cursors(Eo *obj, Efl_Ui_Text_Data *sd)
{
   const char *file;
   efl_file_get(sd->entry_edje, &file, NULL);
   sd->cursor = _decoration_create(sd, file, "elm/entry/cursor/default", EINA_TRUE);
   sd->cursor_bidi = _decoration_create(sd, file, "elm/entry/cursor/default", EINA_TRUE);

   if (!efl_ui_text_interactive_editable_get(obj))
     {
        evas_object_hide(sd->cursor);
        evas_object_hide(sd->cursor_bidi);
     }
}

static void
_decoration_calc_offset(Efl_Ui_Text_Data *sd, Evas_Coord *_x, Evas_Coord *_y)
{
   Evas_Coord x, y;
   Eo *text_obj = edje_object_part_swallow_get(sd->entry_edje, "elm.text");

   evas_object_geometry_get(text_obj, &x, &y, NULL, NULL);

   if (_x) *_x = x;
   if (_y) *_y = y;
}

static void
_update_text_cursors(Eo *obj)
{
   Evas_Coord x, y, w, h, xx, yy, ww, hh;
   Evas_Coord xx2, yy2;
   Eina_Bool bidi_cursor;

   EFL_UI_TEXT_DATA_GET(obj, sd);
   if (!sd->deferred_decoration_cursor) return;
   sd->deferred_decoration_cursor = EINA_FALSE;

   Eo *text_obj = edje_object_part_swallow_get(sd->entry_edje, "elm.text");

   x = y = w = h = -1;
   xx = yy = ww = hh = -1;
   _decoration_calc_offset(sd, &x, &y);
   bidi_cursor = efl_text_cursor_geometry_get(obj, 
         efl_text_cursor_get(text_obj, EFL_TEXT_CURSOR_GET_MAIN),
         EFL_TEXT_CURSOR_TYPE_BEFORE, &xx, &yy, &ww, &hh, &xx2, &yy2,
         NULL, NULL);
   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;
   if (sd->cursor)
     {
        evas_object_move(sd->cursor, x + xx, y + yy);
        evas_object_resize(sd->cursor, ww, hh);
     }
   if (sd->cursor_bidi)
     {
        if (bidi_cursor)
          {
             evas_object_move(sd->cursor_bidi, x + xx2, y + yy2 + (hh / 2));
             evas_object_resize(sd->cursor, ww, hh / 2);
             evas_object_resize(sd->cursor_bidi, ww, hh / 2);
             evas_object_show(sd->cursor_bidi);
          }
        else
          {
             evas_object_hide(sd->cursor_bidi);
          }
     }
   _cursor_geometry_recalc(obj);
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
   Evas_Coord x, y;
   Efl_Text_Cursor_Cursor *sel_start, *sel_end;

   Eina_List *l;
   Eina_Iterator *range;
   Efl_Ui_Text_Rectangle *rect;
   Eina_Rectangle *r;
   const char *file;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   if (!sd->deferred_decoration_selection) return;
   sd->deferred_decoration_selection = EINA_FALSE;

   _decoration_calc_offset(sd, &x, &y);

   efl_file_get(sd->entry_edje, &file, NULL);

   efl_ui_text_interactive_selection_cursors_get(text_obj, &sel_start, &sel_end);

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

             rect->obj_bg = _decoration_create(sd, file, "elm/entry/selection/default", EINA_FALSE);
             evas_object_show(rect->obj_bg);
          }
        else
          {
             rect = eina_list_data_get(l);
             l = l->next;
          }

        if (rect->obj_bg)
          {
             evas_object_move(rect->obj_bg, x + r->x, y + r->y);
             evas_object_resize(rect->obj_bg, r->w, r->h);
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
_item_obj_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Item_Obj *io = data;
   Anchor *an = io->an;

   if (!an)
     {
        ERR("Failed to free item object struct. Anchor is NULL!");
        return;
     }

   EFL_UI_TEXT_DATA_GET(an->obj, sd);
   sd->item_objs = (Item_Obj *)eina_inlist_remove(EINA_INLIST_GET(sd->item_objs),
                                                  EINA_INLIST_GET(io));
   io->an = NULL;
   free(io->name);
   free(io);
}

static Evas_Object *
_item_obj_get(Anchor *an, Evas_Object *o, Evas_Object *smart, Evas_Object *clip)
{
   Item_Obj *io;
   Eo *obj = an->obj;
   Evas_Object *item_obj;

   EFL_UI_TEXT_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH(sd->item_objs, io)
     {
        if (!io->an && io->name && !strcmp(an->name, io->name))
          {
             io->an = an;
             return io->obj;
          }
     }

   io = calloc(1, sizeof(Item_Obj));

   item_obj = _item_get(obj, an->name);
   evas_object_event_callback_add(item_obj, EVAS_CALLBACK_DEL, _item_obj_del_cb, io);
   evas_object_smart_member_add(item_obj, smart);
   evas_object_stack_above(item_obj, o);
   evas_object_clip_set(item_obj, clip);
   evas_object_pass_events_set(item_obj, EINA_TRUE);
   evas_object_show(item_obj);

   io->an = an;
   io->name = strdup(an->name);
   io->obj = item_obj;
   sd->item_objs = (Item_Obj *)eina_inlist_append(EINA_INLIST_GET(sd->item_objs),
                                                  EINA_INLIST_GET(io));

   return io->obj;
}

static void
_unused_item_objs_free(Efl_Ui_Text_Data *sd)
{
   Item_Obj *io;
   Eina_Inlist *l;

   EINA_INLIST_FOREACH_SAFE(sd->item_objs, l, io)
     {
        if (!io->an)
          {
             if (io->obj)
               {
                  evas_object_event_callback_del_full(io->obj, EVAS_CALLBACK_DEL, _item_obj_del_cb, io);
                  evas_object_del(io->obj);
               }

             sd->item_objs = (Item_Obj *)eina_inlist_remove(EINA_INLIST_GET(sd->item_objs),
                                                            EINA_INLIST_GET(io));
             free(io->name);
             free(io);
          }
     }
}

static void
_anchors_clear(Eina_List **_list)
{
   Eina_List *list = *_list;
   while (list)
     {
        Anchor *an = list->data;

        while (an->sel)
          {
             Efl_Ui_Text_Rectangle *sel = an->sel->data;
             if (sel->obj_bg) evas_object_del(sel->obj_bg);
             if (sel->obj_fg) evas_object_del(sel->obj_fg);
             if (!an->item && sel->obj) evas_object_del(sel->obj);
             free(sel);
             an->sel = eina_list_remove_list(an->sel, an->sel);
          }
        free(an->name);
        free(an);
        list = eina_list_remove_list(list, list);
     }

   *_list = list;
}

static void
_anchors_clear_all(Evas_Object *o EINA_UNUSED, Efl_Ui_Text_Data *sd)
{
   Item_Obj *io;

   _anchors_clear(&sd->anchors);
   _anchors_clear(&sd->item_anchors);

   /* Detach anchors from cached objects */
   EINA_INLIST_FOREACH(sd->item_objs, io)
      io->an = NULL;
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

/* Recreates the anchors in the text. */
static void
_anchors_create(Eo *obj, Efl_Ui_Text_Data *sd)
{
   Eina_Iterator *it;
   Anchor *an = NULL;
   Efl_Text_Cursor_Cursor *start, *end;
   Efl_Text_Annotate_Annotation *anchor;

   Eo *text_obj = edje_object_part_swallow_get(sd->entry_edje, "elm.text");
   _anchors_clear_all(obj, sd);

   start = efl_text_cursor_new(text_obj); 
   end = efl_text_cursor_new(text_obj); 

   /* Retrieve all annotations in the text. */
   efl_text_cursor_paragraph_first(obj, start);
   efl_text_cursor_paragraph_last(obj, end);

   it = efl_text_range_annotations_get(obj, start, end);
   efl_text_cursor_free(text_obj, start);
   efl_text_cursor_free(text_obj, end);

   EINA_ITERATOR_FOREACH(it, anchor)
     {
        Eina_Bool is_anchor = EINA_FALSE;
        Eina_Bool is_item = EINA_FALSE;

        if (efl_text_object_item_geometry_get(obj, anchor,
                 NULL, NULL, NULL, NULL))
          {
             is_anchor = EINA_TRUE;
             is_item = EINA_TRUE;
          }
        else if (!strncmp(efl_text_annotation_get(obj, anchor), "a ", 2))
          {
             is_anchor = EINA_TRUE;
          }

        if (is_anchor)
          {
             const char *p;
             const char *item_str = efl_text_annotation_get(obj, anchor);

             an = calloc(1, sizeof(Anchor));
             if (!an)
                break;

             an->obj = obj;
             an->annotation = anchor;
             an->item = is_item;
             p = strstr(item_str, "href=");
             if (p)
               {
                  an->name = _anchor_format_parse(p);
               }
             sd->anchors = eina_list_append(sd->anchors, an);
          }
     }
   eina_iterator_free(it);
}

#if 0
static Eina_Bool
_is_anchors_outside_viewport(Evas_Coord oxy, Evas_Coord axy, Evas_Coord awh,
                                                 Evas_Coord vxy, Evas_Coord vwh)
{
   if (((oxy + axy + awh) < vxy) || ((oxy + axy) > vwh))
     {
        return EINA_TRUE;
     }
   return EINA_FALSE;
}
#endif

static void
_text_anchor_mouse_down_cb(void *data, Evas *e EINA_UNUSED,
      Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(obj, pd);

   Anchor *an = data;
   Efl_Ui_Text_Anchor_Info ei;

   ei.x = ei.y = ei.w = ei.h = 0;
   ei.name = an->name;
   evas_object_geometry_get(obj, &ei.x, &ei.y, &ei.w, &ei.h);
   if (!pd->disabled)
     efl_event_callback_call(an->obj, EFL_UI_TEXT_EVENT_ANCHOR_DOWN, &ei);
}

static void
_text_anchor_mouse_up_cb(void *data, Evas *e EINA_UNUSED,
      Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(obj, pd);

   Efl_Ui_Text_Anchor_Info ei;
   Anchor *an = data;

   ei.x = ei.y = ei.w = ei.h = 0;
   evas_object_geometry_get(obj, &ei.x, &ei.y, &ei.w, &ei.h);
   ei.name = an->name;

   _entry_hover_anchor_clicked_do(an->obj, &ei);
   if (!pd->disabled)
     efl_event_callback_call(an->obj, EFL_UI_TEXT_EVENT_ANCHOR_UP, &ei);
}

static void
_anchors_update(Eo *o, Efl_Ui_Text_Data *sd)
{
   Eina_List *l, *ll, *rl;
   Evas_Coord x, y;
   Evas_Object *smart, *clip;
   Efl_Ui_Text_Rectangle *rect;
   Anchor *an;
   const char *file;

   efl_file_get(sd->entry_edje, &file, NULL);

   if (!sd->deferred_decoration_anchor) return;
   sd->deferred_decoration_anchor = EINA_FALSE;

   _anchors_create(o, sd);

   /* Better not to update anchors outside the view port. */
   if (sd->anchors_updated) return;

   smart = evas_object_smart_parent_get(o);
   clip = evas_object_clip_get(
         edje_object_part_swallow_get(sd->entry_edje, "elm.text"));
   _decoration_calc_offset(sd, &x, &y);
   EINA_LIST_FOREACH(sd->anchors, l, an)
     {
        // for item anchors
        if (an->item)
          {
             Evas_Object *ob;
             Evas_Coord cx, cy, cw, ch;

             if (!an->sel)
               {
                  rect = calloc(1, sizeof(Efl_Ui_Text_Rectangle));
                  an->sel = eina_list_append(an->sel, rect);

                  ob = _item_obj_get(an, o, smart, clip);
                  rect->obj = ob;

                  efl_text_object_item_geometry_get(an->obj,
                           an->annotation, &cx, &cy, &cw, &ch);
                  evas_object_move(rect->obj, x + cx, y + cy);
                  evas_object_resize(rect->obj, cw, ch);
               }
          }
        // for link anchors
        else
          {
             Eina_Iterator *range;
             Efl_Text_Cursor_Cursor *start, *end;
             Eina_List *range_list;
             Eina_Rectangle *r;

             start = efl_text_cursor_new(o);
             end = efl_text_cursor_new(o);
             efl_text_annotation_positions_get(o, an->annotation,
                   start, end);

             range = efl_canvas_text_range_geometry_get(o, start, end);
             range_list = eina_iterator_container_get(range);

             if (eina_list_count(range_list) != eina_list_count(an->sel))
               {
                  while (an->sel)
                    {
                       rect = an->sel->data;
                       if (rect->obj_bg) evas_object_del(rect->obj_bg);
                       if (rect->obj) evas_object_del(rect->obj);
                       free(rect);
                       an->sel = eina_list_remove_list(an->sel, an->sel);
                    }

                  r = range_list->data;
#if 0
                  Eina_Rectangle *r_last;
                  r_last = eina_list_last_data_get(range_list);
                  if (r->y != r_last->y)
                    {
                       /* For multiple range */
                       r->h = r->y + r_last->y + r_last->h;
                    }
#endif
                  /* For vertically layout entry */
#if 0
                  if (_is_anchors_outside_viewport(y, r->y, r->h, vy, tvh))
                    {
                       EINA_LIST_FREE(range, r)
                         free(r);
                       continue;
                    }
                  else
                    {
                       /* XXX: Should consider for horizontal entry but has
                        * very minimal usage. Probably we should get the min x
                        * and max w for range and then decide whether it is in
                        * the viewport or not. Unnecessary calculation for this
                        * minimal usage. Please test with large number of anchors
                        * after implementing it, if its needed to be.
                        */
                    }
#endif

                  /* XXX: the iterator isn't powerful enought to iterate more
                   * than once on the list. We have to resort to this workaround
                   * since for this optimization port to work, we need to
                   * have another go on the list. */
                  EINA_LIST_FOREACH(range_list, ll, r)
                    {
                       Evas_Object *ob;

                       rect = calloc(1, sizeof(Efl_Ui_Text_Rectangle));
                       an->sel = eina_list_append(an->sel, rect);

                       ob = _decoration_create(sd, file, "elm/entry/anchor/default", EINA_TRUE);
                       rect->obj_fg = ob;

                       /* Create hit rectangle to catch events */
                       ob = evas_object_rectangle_add(o);
                       evas_object_color_set(ob, 0, 0, 0, 0);
                       evas_object_smart_member_add(ob, smart);
                       evas_object_stack_above(ob, o);
                       evas_object_clip_set(ob, clip);
                       evas_object_repeat_events_set(ob, EINA_TRUE);
                       rect->obj = ob;
                       evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_DOWN, _text_anchor_mouse_down_cb, an);
                       evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_UP, _text_anchor_mouse_up_cb, an);
#if 0
                       evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_MOVE, _text_anchor_mouse_move_cb, an);
                       evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_IN, _text_anchor_mouse_in_cb, an);
                       evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_OUT, _text_anchor_mouse_out_cb, an);
#endif
                    }
               }

             ll = an->sel;
             EINA_LIST_FOREACH(range_list, rl, r)
               {
                  rect = ll->data;

#if 0
                  if (_is_anchors_outside_viewport(y, r->y, r->h, vy, tvh) ||
                      _is_anchors_outside_viewport(x, r->x, r->w, vx, tvw))
                    {
                       range = eina_list_remove_list(range, range);
                       free(r);
                       evas_object_hide(sel->obj_bg);
                       evas_object_hide(sel->obj_fg);
                       evas_object_hide(sel->obj);
                       continue;
                    }
#endif

                  if (rect->obj_bg)
                    {
                       evas_object_move(rect->obj_bg, x + r->x, y + r->y);
                       evas_object_resize(rect->obj_bg, r->w, r->h);
                       evas_object_show(rect->obj_bg);
                    }
                  if (rect->obj_fg)
                    {
                       evas_object_move(rect->obj_fg, x + r->x, y + r->y);
                       evas_object_resize(rect->obj_fg, r->w, r->h);
                       evas_object_show(rect->obj_fg);
                    }
                  if (rect->obj)
                    {
                       evas_object_move(rect->obj, x + r->x, y + r->y);
                       evas_object_resize(rect->obj, r->w, r->h);
                       evas_object_show(rect->obj);
                    }

                  ll = ll->next;
               }
             eina_iterator_free(range);
          }
     }

   _unused_item_objs_free(sd);
}

static void
_update_decorations(Eo *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);
   Eo *text_obj = edje_object_part_swallow_get(sd->entry_edje, "elm.text");

   _update_text_cursors(obj);
   _update_text_selection(obj, text_obj);
   _anchors_update(obj, sd);
   //_update_text_hover(obj, sd);
}

static void
_deferred_decoration_job(void *data)
{
   EFL_UI_TEXT_DATA_GET(data, sd);

   sd->deferred_decoration_job = NULL;
   _update_decorations(data);
}

static void
_decoration_defer(Eo *obj)
{
   EFL_UI_TEXT_DATA_GET(obj, sd);
   ecore_job_del(sd->deferred_decoration_job);
   sd->deferred_decoration_job =
      ecore_job_add(_deferred_decoration_job, obj);
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
   _decoration_defer_all(data);
   _entry_changed_handle(data, EFL_UI_TEXT_EVENT_CHANGED);
}

static void
_efl_ui_text_cursor_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   EFL_UI_TEXT_DATA_GET(data, sd);
   sd->cur_changed = EINA_TRUE;
   sd->deferred_decoration_cursor = EINA_TRUE;
   _decoration_defer(data);
}

static void
_efl_ui_text_selection_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   Efl_Text_Cursor_Cursor *start, *end;
   char *text;
   EFL_UI_TEXT_DATA_GET(obj, sd);

   efl_ui_text_interactive_selection_cursors_get(obj, &start, &end);

   text = efl_canvas_text_range_text_get(obj, start, end);
   if (!text || (text[0] == '\0'))
     {
        _edje_signal_emit(sd, "selection,cleared", "elm.text");
        sd->have_selection = EINA_FALSE;
     }
   else
     {
        if (!sd->have_selection)
          {
             _edje_signal_emit(sd, "selection,start", "elm.text");
          }
        _edje_signal_emit(sd, "selection,changed", "elm.text");
        sd->have_selection = EINA_TRUE;
     }
   if (text) free(text);
   sd->deferred_decoration_selection = EINA_TRUE;
   _decoration_defer(data);
}

static void
_efl_ui_text_move_cb(void *data, Evas *e EINA_UNUSED,
      Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _decoration_defer_all(data);
}

#if 0
/* Efl.Part begin */

ELM_PART_OVERRIDE(elm_entry, EFL_UI_TEXT, ELM_LAYOUT, Efl_Ui_Text_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_CONTENT_SET(elm_entry, EFL_UI_TEXT, ELM_LAYOUT, Efl_Ui_Text_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(elm_entry, EFL_UI_TEXT, ELM_LAYOUT, Efl_Ui_Text_Data, Elm_Part_Data)
#include "elm_entry_internal_part.eo.c"

/* Efl.Part end */
#endif

/* Internal EO APIs and hidden overrides */

ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT()

#define EFL_UI_TEXT_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_text), \
   ELM_LAYOUT_CONTENT_ALIASES_OPS()

#include "efl_ui_text.eo.c"

#undef MY_CLASS
#define MY_CLASS EFL_UI_TEXT_EDITABLE_CLASS

EOLIAN static Eo *
_efl_ui_text_editable_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_ui_text_interactive_editable_set(obj, EINA_TRUE);

   return obj;
}

#include "efl_ui_text_editable.eo.c"
