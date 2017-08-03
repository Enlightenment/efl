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
#include "elm_widget_entry.h"

#include "elm_entry_internal_part.eo.h"
#include "elm_part_helper.h"
#include "elm_hoversel.eo.h"

#define MY_CLASS ELM_ENTRY_CLASS
#define MY_CLASS_PFX elm_entry

#define MY_CLASS_NAME "Elm_Entry"
#define MY_CLASS_NAME_LEGACY "elm_entry"

/* Maximum chunk size to be inserted to the entry at once
 * FIXME: This size is arbitrary, should probably choose a better size.
 * Possibly also find a way to set it to a low value for weak computers,
 * and to a big value for better computers. */
#define ELM_ENTRY_CHUNK_SIZE 10000
#define ELM_ENTRY_DELAY_WRITE_TIME 2.0

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

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {"guide", "elm.guide"},
   {NULL, NULL}
};

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

static void _create_selection_handlers(Evas_Object *obj, Elm_Entry_Data *sd);
static void _magnifier_move(void *data);

static Evas_Object *
_entry_win_get(Evas_Object *obj)
{
   Evas_Object *top;
   top = elm_widget_top_get(obj);
   if ((!elm_win_window_id_get(top)) && (elm_win_type_get(top) == ELM_WIN_INLINED_IMAGE))
     top = elm_widget_parent2_get(top);
   return top;
}

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
   char *text;

   text = _file_load(file);
   if (text)
     {
        char *text2;

        text2 = elm_entry_utf8_to_markup(text);
        free(text);
        return text2;
     }

   return NULL;
}

static Eina_Bool
_load_do(Evas_Object *obj)
{
   char *text;

   ELM_ENTRY_DATA_GET(obj, sd);

   if (!sd->file)
     {
        elm_object_text_set(obj, "");
        return EINA_TRUE;
     }

   switch (sd->format)
     {
      case ELM_TEXT_FORMAT_PLAIN_UTF8:
        text = _plain_load(sd->file);
        break;

      case ELM_TEXT_FORMAT_MARKUP_UTF8:
        text = _file_load(sd->file);
        break;

      default:
        text = NULL;
        break;
     }

   if (text)
     {
        elm_object_text_set(obj, text);
        free(text);

        return EINA_TRUE;
     }
   else
     {
        elm_object_text_set(obj, "");

        return EINA_FALSE;
     }
}

static void
_utf8_markup_save(const char *file,
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
_utf8_plain_save(const char *file,
                 const char *text)
{
   char *text2;

   text2 = elm_entry_markup_to_utf8(text);
   if (!text2)
     return;

   _utf8_markup_save(file, text2);
   free(text2);
}

static void
_save_do(Evas_Object *obj)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   if (!sd->file) return;
   switch (sd->format)
     {
      case ELM_TEXT_FORMAT_PLAIN_UTF8:
        _utf8_plain_save(sd->file, elm_object_text_get(obj));
        break;

      case ELM_TEXT_FORMAT_MARKUP_UTF8:
        _utf8_markup_save(sd->file, elm_object_text_get(obj));
        break;

      default:
        break;
     }
}

static Eina_Bool
_delay_write(void *data)
{
   ELM_ENTRY_DATA_GET(data, sd);

   _save_do(data);
   sd->delay_write = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_elm_entry_guide_update(Evas_Object *obj,
                        Eina_Bool has_text)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   if ((has_text) && (!sd->has_text))
     edje_object_signal_emit(sd->entry_edje, "elm,guide,disabled", "elm");
   else if ((!has_text) && (sd->has_text))
     edje_object_signal_emit(sd->entry_edje, "elm,guide,enabled", "elm");

   sd->has_text = has_text;
}

static void
_validate(Evas_Object *obj)
{
   ELM_ENTRY_DATA_GET(obj, sd);
   Eina_Bool res;
   Elm_Validate_Content vc;
   Eina_Strbuf *buf;

   if (sd->validators == 0) return;

   vc.text = edje_object_part_text_get(sd->entry_edje, "elm.text");
   res = efl_event_callback_legacy_call(obj, ELM_ENTRY_EVENT_VALIDATE, (void *)&vc);
   buf = eina_strbuf_new();
   eina_strbuf_append_printf(buf, "validation,%s,%s", vc.signal, res == EINA_FALSE ? "fail" : "pass");
   edje_object_signal_emit(sd->scr_edje, eina_strbuf_string_get(buf), "elm");
   eina_tmpstr_del(vc.signal);
   eina_strbuf_free(buf);
}

static Elm_Entry_Markup_Filter *
_filter_new(Elm_Entry_Filter_Cb func,
            void *data)
{
   Elm_Entry_Markup_Filter *tf = ELM_NEW(Elm_Entry_Markup_Filter);

   if (!tf) return NULL;

   tf->func = func;
   tf->orig_data = data;
   if (func == elm_entry_filter_limit_size)
     {
        Elm_Entry_Filter_Limit_Size *lim = data, *lim2;

        if (!data)
          {
             free(tf);

             return NULL;
          }

        lim2 = malloc(sizeof(Elm_Entry_Filter_Limit_Size));
        if (!lim2)
          {
             free(tf);

             return NULL;
          }
        memcpy(lim2, lim, sizeof(Elm_Entry_Filter_Limit_Size));
        tf->data = lim2;
     }
   else if (func == elm_entry_filter_accept_set)
     {
        Elm_Entry_Filter_Accept_Set *as = data, *as2;

        if (!data)
          {
             free(tf);

             return NULL;
          }
        as2 = malloc(sizeof(Elm_Entry_Filter_Accept_Set));
        if (!as2)
          {
             free(tf);

             return NULL;
          }
        if (as->accepted)
          as2->accepted = eina_stringshare_add(as->accepted);
        else
          as2->accepted = NULL;
        if (as->rejected)
          as2->rejected = eina_stringshare_add(as->rejected);
        else
          as2->rejected = NULL;
        tf->data = as2;
     }
   else
     tf->data = data;
   return tf;
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
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_mirrored_set(sd->entry_edje, rtl);

   if (sd->anchor_hover.hover)
     efl_ui_mirrored_set(sd->anchor_hover.hover, rtl);
}

static void
_hide_selection_handler(Evas_Object *obj)
{
   ELM_ENTRY_DATA_GET(obj, sd);

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
   ELM_ENTRY_DATA_GET(obj, sd);
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
_update_selection_handler(Evas_Object *obj)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   Evas_Coord sx, sy, sh;
   Evas_Coord ent_x, ent_y;
   Evas_Coord ex, ey, eh;
   int start_pos, end_pos, last_pos;

   if (!sd->sel_handler_disabled)
     {
        Eina_Rectangle *rect;
        Evas_Coord hx, hy;
        Eina_Bool hidden = EINA_FALSE;

        if (!sd->start_handler)
          _create_selection_handlers(obj, sd);

        rect = _viewport_region_get(obj);
        start_pos = edje_object_part_text_cursor_pos_get
           (sd->entry_edje, "elm.text", EDJE_CURSOR_SELECTION_BEGIN);
        end_pos = edje_object_part_text_cursor_pos_get
           (sd->entry_edje, "elm.text", EDJE_CURSOR_SELECTION_END);

        evas_object_geometry_get(sd->entry_edje, &ent_x, &ent_y, NULL, NULL);
        last_pos = edje_object_part_text_cursor_pos_get(sd->entry_edje, "elm.text",
                                                        EDJE_CURSOR_MAIN);
        edje_object_part_text_cursor_pos_set(sd->entry_edje, "elm.text",
                                             EDJE_CURSOR_MAIN, start_pos);
        edje_object_part_text_cursor_geometry_get(sd->entry_edje, "elm.text",
                                                  &sx, &sy, NULL, &sh);
        edje_object_part_text_cursor_pos_set(sd->entry_edje, "elm.text",
                                             EDJE_CURSOR_MAIN, end_pos);
        edje_object_part_text_cursor_geometry_get(sd->entry_edje, "elm.text",
                                                  &ex, &ey, NULL, &eh);
        edje_object_part_text_cursor_pos_set(sd->entry_edje, "elm.text",
                                             EDJE_CURSOR_MAIN, last_pos);
        if (start_pos < end_pos)
          {
             hx = ent_x + sx;
             hy = ent_y + sy + sh;
             evas_object_move(sd->start_handler, hx, hy);
          }
        else
          {
             hx = ent_x + ex;
             hy = ent_y + ey + eh;
             evas_object_move(sd->start_handler, hx, hy);
          }
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

        hidden = EINA_FALSE;
        if (start_pos < end_pos)
          {
             hx = ent_x + ex;
             hy = ent_y + ey + eh;
             evas_object_move(sd->end_handler, hx, hy);
          }
        else
          {
             hx = ent_x + sx;
             hy = ent_y + sy + sh;
             evas_object_move(sd->end_handler, hx, hy);
          }
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
_elm_entry_theme_group_get(Evas_Object *obj)
{
   ELM_ENTRY_DATA_GET(obj, sd);

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
   ELM_ENTRY_DATA_GET(obj, sd);

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
   ELM_ENTRY_DATA_GET(obj, sd);

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
        char *txt = _elm_util_text_to_mkup(buf);
        if (txt)
          {
             _edje_entry_user_insert(obj, txt);
             free(txt);
          }
        else
          {
             ERR("Failed to convert text to markup text!");
          }
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

   ELM_ENTRY_DATA_GET(obj, sd);

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

   ELM_ENTRY_DATA_GET(obj, sd);

   rv = edje_object_part_text_cursor_coord_set
       (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN, drop->x, drop->y);

   if (!rv) WRN("Warning: Failed to position cursor: paste anyway");

   rv = _selection_data_cb(NULL, obj, drop);

   return rv;
}

static Elm_Sel_Format
_get_drop_format(Evas_Object *obj)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   if ((sd->editable) && (!sd->single_line) && (!sd->password) && (!sd->disabled))
     return ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE;
   return ELM_SEL_FORMAT_MARKUP;
}

/* we can't reuse layout's here, because it's on entry_edje only */
EOLIAN static Eina_Bool
_elm_entry_elm_widget_disable(Eo *obj, Elm_Entry_Data *sd)
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
_elm_entry_background_switch(Evas_Object *from_edje, Evas_Object *to_edje)
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
_elm_entry_elm_widget_theme_apply(Eo *obj, Elm_Entry_Data *sd)
{
   const char *str;
   const char *t;
   const char *stl_user;
   const char *style = elm_widget_style_get(obj);
   Elm_Theme_Apply theme_apply;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ELM_THEME_APPLY_FAILED);

   // Note: We are skipping elm_layout here! This is by design.
   // This assumes the following inheritance: my_class -> layout -> widget ...
   theme_apply = elm_obj_widget_theme_apply(efl_cast(obj, ELM_WIDGET_CLASS));
   if (!theme_apply) return ELM_THEME_APPLY_FAILED;

   evas_event_freeze(evas_object_evas_get(obj));

   edje_object_part_text_hide_visible_password(sd->entry_edje, "elm.text");

   edje_object_mirrored_set
     (wd->resize_obj, efl_ui_mirrored_get(obj));

   edje_object_scale_set
     (wd->resize_obj,
     efl_ui_scale_get(obj) * elm_config_scale_get());

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   stl_user = eina_stringshare_add(edje_object_part_text_style_user_peek(sd->entry_edje, "elm.text"));
   t = eina_stringshare_add(elm_object_text_get(obj));

   elm_widget_theme_object_set
     (obj, sd->entry_edje, "entry", _elm_entry_theme_group_get(obj), style);

   if (sd->sel_allow && _elm_config->desktop_entry)
     edje_obj_part_text_select_allow_set
        (sd->entry_edje, "elm.text", EINA_TRUE);
   else
     edje_obj_part_text_select_allow_set
        (sd->entry_edje, "elm.text", EINA_FALSE);

   edje_object_part_text_style_user_push(sd->entry_edje, "elm.text", stl_user);
   eina_stringshare_del(stl_user);

   elm_object_text_set(obj, t);
   eina_stringshare_del(t);

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
   edje_object_part_text_prediction_hint_set
     (sd->entry_edje, "elm.text", sd->prediction_hint);

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

        _elm_entry_background_switch(sd->entry_edje, sd->scr_edje);

        str = edje_object_data_get(sd->scr_edje, "focus_highlight");
     }
   else
     {
        _elm_entry_background_switch(sd->scr_edje, sd->entry_edje);

        str = edje_object_data_get(sd->entry_edje, "focus_highlight");
     }

   if ((str) && (!strcmp(str, "on")))
     elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_highlight_in_theme_set(obj, EINA_FALSE);

   if (sd->start_handler)
     {
        _elm_theme_object_set(obj, sd->start_handler,
                                    "entry", "handler/start", style);
        _elm_theme_object_set(obj, sd->end_handler,
                                    "entry", "handler/end", style);
     }
   elm_entry_icon_visible_set(obj, EINA_TRUE);
   elm_entry_end_visible_set(obj, EINA_TRUE);

   if (sd->scroll)
     efl_canvas_layout_signal_emit(sd->entry_edje, "elm,scroll,enable", "elm");
   else
     efl_canvas_layout_signal_emit(sd->entry_edje, "elm,scroll,disable", "elm");

   sd->changed = EINA_TRUE;
   elm_layout_sizing_eval(obj);

   sd->has_text = !sd->has_text;
   _elm_entry_guide_update(obj, !sd->has_text);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   efl_event_callback_legacy_call(obj, ELM_LAYOUT_EVENT_THEME_CHANGED, NULL);

   evas_object_unref(obj);

   return theme_apply;
}

static void
_cursor_geometry_recalc(Evas_Object *obj)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   if (!sd->deferred_recalc_job)
     {
        Evas_Coord cx, cy, cw, ch;

        edje_object_part_text_cursor_geometry_get
          (sd->entry_edje, "elm.text", &cx, &cy, &cw, &ch);
        if (sd->cur_changed)
          {
             sd->cur_changed = EINA_FALSE;
             elm_widget_show_region_set(obj, cx, cy, cw, ch, EINA_FALSE);
          }
     }
   else
     sd->deferred_cur = EINA_TRUE;
}

static void
_deferred_recalc_job(void *data)
{
   Evas_Coord minh = -1, resw = -1, minw = -1, fw = 0, fh = 0;

   ELM_ENTRY_DATA_GET(data, sd);

   sd->deferred_recalc_job = NULL;

   evas_object_geometry_get(sd->entry_edje, NULL, NULL, &resw, NULL);
   edje_object_size_min_restricted_calc(sd->entry_edje, &minw, &minh, resw, 0);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);

   /* This is a hack to workaround the way min size hints are treated.
    * If the minimum width is smaller than the restricted width, it
    * means the minimum doesn't matter. */
   if (minw <= resw)
     {
        Evas_Coord ominw = -1;

        efl_gfx_size_hint_combined_min_get(data, &ominw, NULL);
        minw = ominw;
     }

   sd->ent_mw = minw;
   sd->ent_mh = minh;

   elm_coords_finger_size_adjust(1, &fw, 1, &fh);
   if (sd->scroll)
     {
        Evas_Coord vmw = 0, vmh = 0;

        edje_object_size_min_calc(sd->scr_edje, &vmw, &vmh);
        if (sd->single_line)
          {
             evas_object_size_hint_min_set(data, vmw, minh + vmh);
             evas_object_size_hint_max_set(data, -1, minh + vmh);
          }
        else
          {
             evas_object_size_hint_min_set(data, vmw, vmh);
             evas_object_size_hint_max_set(data, -1, -1);
          }
     }
   else
     {
        if (sd->single_line)
          {
             evas_object_size_hint_min_set(data, minw, minh);
             evas_object_size_hint_max_set(data, -1, minh);
          }
        else
          {
             evas_object_size_hint_min_set(data, fw, minh);
             evas_object_size_hint_max_set(data, -1, -1);
          }
     }

   if (sd->deferred_cur)
     {
        Evas_Coord cx, cy, cw, ch;

        edje_object_part_text_cursor_geometry_get
          (sd->entry_edje, "elm.text", &cx, &cy, &cw, &ch);
        if (sd->cur_changed)
          {
             sd->cur_changed = EINA_FALSE;
             elm_widget_show_region_set(data, cx, cy, cw, ch, EINA_FALSE);
          }
     }
}

EOLIAN static void
_elm_entry_elm_layout_sizing_eval(Eo *obj, Elm_Entry_Data *sd)
{
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord resw, resh;

   evas_object_geometry_get(obj, NULL, NULL, &resw, &resh);

   if (sd->line_wrap)
     {
        if ((resw == sd->last_w) && (!sd->changed))
          {
             if (sd->scroll)
               {
                  Evas_Coord vw = 0, vh = 0, w = 0, h = 0;

                  elm_interface_scrollable_content_viewport_geometry_get
                        (obj, NULL, NULL, &vw, &vh);

                  w = sd->ent_mw;
                  h = sd->ent_mh;
                  if (vw > sd->ent_mw) w = vw;
                  if (vh > sd->ent_mh) h = vh;
                  evas_object_resize(sd->entry_edje, w, h);

                  return;
               }

             return;
          }

        evas_event_freeze(evas_object_evas_get(obj));
        sd->changed = EINA_FALSE;
        sd->last_w = resw;
        if (sd->scroll)
          {
             Evas_Coord vw = 0, vh = 0, vmw = 0, vmh = 0, w = -1, h = -1;

             evas_object_resize(sd->scr_edje, resw, resh);
             edje_object_size_min_calc(sd->scr_edje, &vmw, &vmh);
             elm_interface_scrollable_content_viewport_geometry_get
                   (obj, NULL, NULL, &vw, &vh);
             edje_object_size_min_restricted_calc
               (sd->entry_edje, &minw, &minh, vw, 0);
             elm_coords_finger_size_adjust(1, &minw, 1, &minh);

             /* This is a hack to workaround the way min size hints
              * are treated.  If the minimum width is smaller than the
              * restricted width, it means the minimum doesn't
              * matter. */
             if (minw <= vw)
               {
                  Evas_Coord ominw = -1;

                  efl_gfx_size_hint_combined_min_get(sd->entry_edje, &ominw, NULL);
                  minw = ominw;
               }
             sd->ent_mw = minw;
             sd->ent_mh = minh;

             if ((minw > 0) && (vw < minw)) vw = minw;
             if (minh > vh) vh = minh;

             if (sd->single_line) h = vmh + minh;
             else h = vmh;

             evas_object_resize(sd->entry_edje, vw, vh);
             evas_object_size_hint_min_set(obj, w, h);

             if (sd->single_line)
               evas_object_size_hint_max_set(obj, -1, h);
             else
               evas_object_size_hint_max_set(obj, -1, -1);
          }
        else
          {
             ecore_job_del(sd->deferred_recalc_job);
             sd->deferred_recalc_job =
               ecore_job_add(_deferred_recalc_job, obj);
          }

        evas_event_thaw(evas_object_evas_get(obj));
        evas_event_thaw_eval(evas_object_evas_get(obj));
     }
   else
     {
        if (!sd->changed) return;
        evas_event_freeze(evas_object_evas_get(obj));
        sd->changed = EINA_FALSE;
        sd->last_w = resw;
        if (sd->scroll)
          {
             Evas_Coord vw = 0, vh = 0, vmw = 0, vmh = 0, w = -1, h = -1;

             edje_object_size_min_calc(sd->entry_edje, &minw, &minh);
             sd->ent_mw = minw;
             sd->ent_mh = minh;
             elm_coords_finger_size_adjust(1, &minw, 1, &minh);

             elm_interface_scrollable_content_viewport_geometry_get
                   (obj, NULL, NULL, &vw, &vh);

             if (minw > vw) vw = minw;
             if (minh > vh) vh = minh;

             evas_object_resize(sd->entry_edje, vw, vh);
             edje_object_size_min_calc(sd->scr_edje, &vmw, &vmh);
             if (sd->single_line) h = vmh + minh;
             else h = vmh;

             evas_object_size_hint_min_set(obj, w, h);
             if (sd->single_line)
               evas_object_size_hint_max_set(obj, -1, h);
             else
               evas_object_size_hint_max_set(obj, -1, -1);
          }
        else
          {
             edje_object_size_min_calc(sd->entry_edje, &minw, &minh);
             sd->ent_mw = minw;
             sd->ent_mh = minh;
             elm_coords_finger_size_adjust(1, &minw, 1, &minh);
             evas_object_size_hint_min_set(obj, minw, minh);

             if (sd->single_line)
               evas_object_size_hint_max_set(obj, -1, minh);
             else
               evas_object_size_hint_max_set(obj, -1, -1);
          }
        evas_event_thaw(evas_object_evas_get(obj));
        evas_event_thaw_eval(evas_object_evas_get(obj));
     }

   _cursor_geometry_recalc(obj);
}

static void
_return_key_enabled_check(Evas_Object *obj)
{
   Eina_Bool return_key_disabled = EINA_FALSE;

   ELM_ENTRY_DATA_GET(obj, sd);

   if (!sd->auto_return_key) return;

   if (elm_entry_is_empty(obj) == EINA_TRUE)
     return_key_disabled = EINA_TRUE;

   elm_entry_input_panel_return_key_disabled_set(obj, return_key_disabled);
}

static void
_elm_entry_focus_update(Eo *obj, Elm_Entry_Data *sd)
{
   Evas_Object *top;
   Eina_Bool top_is_win = EINA_FALSE;

   top = elm_widget_top_get(obj);
   if (top && efl_isa(top, EFL_UI_WIN_CLASS))
     top_is_win = EINA_TRUE;

   if (elm_widget_focus_get(obj) && sd->editable)
     {
        evas_object_focus_set(sd->entry_edje, EINA_TRUE);
        edje_object_signal_emit(sd->entry_edje, "elm,action,focus", "elm");
        if (sd->scroll)
          edje_object_signal_emit(sd->scr_edje, "elm,action,focus", "elm");

        if (top && top_is_win && sd->input_panel_enable && !sd->input_panel_show_on_demand &&
            !edje_object_part_text_imf_context_get(sd->entry_edje, "elm.text"))
          elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_ON);
        efl_event_callback_legacy_call(obj, ELM_WIDGET_EVENT_FOCUSED, NULL);
        if (_elm_config->atspi_mode)
          elm_interface_atspi_accessible_state_changed_signal_emit(obj, ELM_ATSPI_STATE_FOCUSED, EINA_TRUE);
        _return_key_enabled_check(obj);
        _validate(obj);
     }
   else
     {
        edje_object_signal_emit(sd->entry_edje, "elm,action,unfocus", "elm");
        if (sd->scroll)
          edje_object_signal_emit(sd->scr_edje, "elm,action,unfocus", "elm");
        evas_object_focus_set(sd->entry_edje, EINA_FALSE);
        if (top && top_is_win && sd->input_panel_enable &&
            !edje_object_part_text_imf_context_get(sd->entry_edje, "elm.text"))
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
}

EOLIAN static Eina_Bool
_elm_entry_elm_widget_on_focus(Eo *obj, Elm_Entry_Data *sd, Elm_Object_Item *item EINA_UNUSED)
{
   _elm_entry_focus_update(obj, sd);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_entry_elm_widget_on_focus_region(Eo *obj, Elm_Entry_Data *sd, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Coord cx, cy, cw, ch;
   Evas_Coord edx, edy;
   Evas_Coord elx, ely, elw, elh;

   edje_object_part_text_cursor_geometry_get
     (sd->entry_edje, "elm.text", &cx, &cy, &cw, &ch);

   if (sd->single_line)
     {
        evas_object_geometry_get(sd->entry_edje, &edx, &edy, NULL, &ch);
        cy = 0;
     }
   else
     {
        evas_object_geometry_get(sd->entry_edje, &edx, &edy, NULL, NULL);
     }
   evas_object_geometry_get(obj, &elx, &ely, &elw, &elh);

   if (x)
     {    
       *x = cx + edx - elx;
       if ((cw < elw) && (*x + cw > elw)) *x = elw - cw;
     }
   if (y)
     {    
       *y = cy + edy - ely;
       if ((ch < elh) && (*y + ch > elh)) *y = elh - ch;
     }
   if (w) *w = cw;
   if (h) *h = ch;


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
_elm_entry_elm_widget_sub_object_del(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, Evas_Object *sobj)
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

   ELM_ENTRY_DATA_GET(obj, sd);

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

   efl_gfx_size_hint_combined_min_get(sd->hoversel, &mw, &mh);
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
   ELM_ENTRY_DATA_GET(data, sd);

   ELM_SAFE_FREE(sd->hoversel, evas_object_del);
   sd->hov_deljob = NULL;
}

static void
_hover_dismissed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(data, sd);

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
   ELM_ENTRY_DATA_GET(data, sd);

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
_elm_entry_entry_paste(Evas_Object *obj,
                       const char *entry)
{
   char *str = NULL;

   if (!entry) return;

   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

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
_paste_cb(void *data,
          Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   Elm_Sel_Format formats = ELM_SEL_FORMAT_MARKUP;

   ELM_ENTRY_DATA_GET(data, sd);

   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_SELECTION_PASTE, NULL);

   sd->selection_asked = EINA_TRUE;

   if (sd->cnp_mode == ELM_CNP_MODE_PLAINTEXT)
     formats = ELM_SEL_FORMAT_TEXT;
   else if (sd->cnp_mode != ELM_CNP_MODE_NO_IMAGE)
     formats |= ELM_SEL_FORMAT_IMAGE;

   elm_cnp_selection_get
     (data, ELM_SEL_TYPE_CLIPBOARD, formats, _selection_data_cb, NULL);
}

static void
_selection_clear(void *data, Elm_Sel_Type selection)
{
   ELM_ENTRY_DATA_GET(data, sd);

   if (!sd->have_selection) return;
   if ((selection == ELM_SEL_TYPE_CLIPBOARD) ||
       (selection == ELM_SEL_TYPE_PRIMARY))
     {
        elm_entry_select_none(data);
     }
}

static void
_selection_store(Elm_Sel_Type seltype,
                 Evas_Object *obj)
{
   const char *sel;

   ELM_ENTRY_DATA_GET(obj, sd);

   sel = edje_object_part_text_selection_get(sd->entry_edje, "elm.text");
   if ((!sel) || (!sel[0])) return;  /* avoid deleting our own selection */

   elm_cnp_selection_set
     (obj, seltype, ELM_SEL_FORMAT_MARKUP, sel, strlen(sel));
   elm_cnp_selection_loss_callback_set(obj, seltype, _selection_clear, obj);
   if (seltype == ELM_SEL_TYPE_CLIPBOARD)
     eina_stringshare_replace(&sd->cut_sel, sel);
}

static void
_cut_cb(void *data,
        Evas_Object *obj EINA_UNUSED,
        void *event_info EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(data, sd);

   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_SELECTION_CUT, NULL);
   /* Store it */
   sd->sel_mode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set
       (sd->entry_edje, "elm.text", EINA_FALSE);
   edje_object_signal_emit(sd->entry_edje, "elm,state,select,off", "elm");

   if (!_elm_config->desktop_entry)
     elm_widget_scroll_hold_pop(data);

   _selection_store(ELM_SEL_TYPE_CLIPBOARD, data);
   _edje_entry_user_insert(data, "");
}

static void
_copy_cb(void *data,
         Evas_Object *obj EINA_UNUSED,
         void *event_info EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(data, sd);

   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_SELECTION_COPY, NULL);
   sd->sel_mode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     {
        edje_object_part_text_select_allow_set
          (sd->entry_edje, "elm.text", EINA_FALSE);
        edje_object_signal_emit(sd->entry_edje, "elm,state,select,off", "elm");
        elm_widget_scroll_hold_pop(data);
     }
   _selection_store(ELM_SEL_TYPE_CLIPBOARD, data);
}

static void
_hover_cancel_cb(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(data, sd);

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

   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->anchor_hover.hover) return;

   efl_event_callback_legacy_call(obj, ELM_ENTRY_EVENT_CONTEXT_OPEN, NULL);

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
                        _cut_cb, obj);
                  elm_hoversel_item_add
                     (sd->hoversel, E_("Copy"), NULL, ELM_ICON_NONE,
                      _copy_cb, obj);
                  if (sd->editable && ownersel)
                    elm_hoversel_item_add
                       (sd->hoversel, E_("Paste"), NULL, ELM_ICON_NONE,
                        _paste_cb, obj);
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
                           _paste_cb, obj);
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
   ELM_ENTRY_DATA_GET(data, sd);

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
   ELM_ENTRY_DATA_GET(data, sd);

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
   ELM_ENTRY_DATA_GET(data, sd);
   edje_object_signal_emit(sd->mgf_bg, "elm,action,hide,magnifier", "elm");
   elm_widget_scroll_freeze_pop(data);
   evas_object_hide(sd->mgf_clip);
}

static void
_magnifier_show(void *data)
{
   ELM_ENTRY_DATA_GET(data, sd);
   edje_object_signal_emit(sd->mgf_bg, "elm,action,show,magnifier", "elm");
   elm_widget_scroll_freeze_push(data);
   evas_object_show(sd->mgf_clip);
}

static Eina_Bool
_long_press_cb(void *data)
{
   ELM_ENTRY_DATA_GET(data, sd);

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
   /* First check if context menu disabled is false or not, and
    * then check for key id */
   if ((!_elm_config->context_menu_disabled) && !strcmp(ev->key, "Menu"))
     _menu_call(data);
}

static void
_mouse_down_cb(void *data,
               Evas *evas EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;

   ELM_ENTRY_DATA_GET(data, sd);

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

   ELM_ENTRY_DATA_GET(data, sd);

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

                  if (top_is_win && sd->input_panel_enable && sd->input_panel_show_on_demand &&
                      !edje_object_part_text_imf_context_get(sd->entry_edje, "elm.text"))
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

   ELM_ENTRY_DATA_GET(data, sd);

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

   ELM_ENTRY_DATA_GET(data, sd);

   evas_event_freeze(evas_object_evas_get(data));
   sd->changed = EINA_TRUE;
   /* Reset the size hints which are no more relevant. Keep the
    * height, this is a hack, but doesn't really matter cause we'll
    * re-eval in a moment. */
   efl_gfx_size_hint_combined_min_get(data, NULL, &minh);
   evas_object_size_hint_min_set(data, -1, minh);

   elm_layout_sizing_eval(data);
   ELM_SAFE_FREE(sd->text, eina_stringshare_del);
   ELM_SAFE_FREE(sd->delay_write, ecore_timer_del);
   evas_event_thaw(evas_object_evas_get(data));
   evas_event_thaw_eval(evas_object_evas_get(data));
   if ((sd->auto_save) && (sd->file))
     sd->delay_write = ecore_timer_add(ELM_ENTRY_DELAY_WRITE_TIME,
                                       _delay_write, data);

   _return_key_enabled_check(data);
   text = edje_object_part_text_get(sd->entry_edje, "elm.text");
   if (text)
     {
        if (text[0])
          _elm_entry_guide_update(data, EINA_TRUE);
        else
          _elm_entry_guide_update(data, EINA_FALSE);
     }
   _validate(data);

   /* callback - this could call callbacks that delete the
    * entry... thus... any access to sd after this could be
    * invalid */
   efl_event_callback_legacy_call(data, event, NULL);
}

static void
_entry_changed_signal_cb(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         const char *emission EINA_UNUSED,
                         const char *source EINA_UNUSED)
{
   _entry_changed_handle(data, ELM_ENTRY_EVENT_CHANGED);
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
        efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_CHANGED_USER, &info);
     }
   else
     {
        efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_CHANGED_USER, NULL);
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
   char *text = NULL;
   Edje_Entry_Change_Info *edje_info = (Edje_Entry_Change_Info *)
                                  edje_object_signal_callback_extra_data_get();
   _entry_changed_handle(data, ELM_ENTRY_EVENT_PREEDIT_CHANGED);

   if (_elm_config->atspi_mode)
     {
        Elm_Atspi_Text_Change_Info atspi_info;
        if (edje_info && edje_info->insert)
          {
             text = elm_entry_markup_to_utf8(edje_info->change.insert.content);
             atspi_info.content = text;
             atspi_info.pos = edje_info->change.insert.pos;
             atspi_info.len = edje_info->change.insert.plain_length;
             elm_interface_atspi_accessible_event_emit(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN,
                                                       data,
                                                       ELM_INTERFACE_ATSPI_TEXT_EVENT_ACCESS_TEXT_INSERTED,
                                                       &atspi_info);
             free(text);
          }
     }
}

static void
_entry_undo_request_signal_cb(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              const char *emission EINA_UNUSED,
                              const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_UNDO_REQUEST, NULL);
}

static void
_entry_redo_request_signal_cb(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              const char *emission EINA_UNUSED,
                              const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_REDO_REQUEST, NULL);
}

static void
_entry_selection_start_signal_cb(void *data,
                                 Evas_Object *obj EINA_UNUSED,
                                 const char *emission EINA_UNUSED,
                                 const char *source EINA_UNUSED)
{
   const Eina_List *l;
   Evas_Object *entry;

   EINA_LIST_FOREACH(entries, l, entry)
     {
        if (entry != data) elm_entry_select_none(entry);
     }
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_SELECTION_START, NULL);

   elm_object_focus_set(data, EINA_TRUE);
}

static void
_entry_selection_all_signal_cb(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               const char *emission EINA_UNUSED,
                               const char *source EINA_UNUSED)
{
   elm_entry_select_all(data);
}

static void
_entry_selection_none_signal_cb(void *data,
                                Evas_Object *obj EINA_UNUSED,
                                const char *emission EINA_UNUSED,
                                const char *source EINA_UNUSED)
{
   elm_entry_select_none(data);
}

static inline Eina_Bool
_entry_win_is_wl(Evas_Object *obj)
{
   Evas_Object *win = _entry_win_get(obj);
   /* primary selection does not exist (yet) */
   return win && elm_win_wl_window_get(win);
}

static void
_entry_selection_changed_signal_cb(void *data,
                                   Evas_Object *obj EINA_UNUSED,
                                   const char *emission EINA_UNUSED,
                                   const char *source EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(data, sd);

   sd->have_selection = EINA_TRUE;
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_SELECTION_CHANGED, NULL);
   if (!_entry_win_is_wl(data))
     _selection_store(ELM_SEL_TYPE_PRIMARY, data);
   _update_selection_handler(data);
   if (_elm_config->atspi_mode)
     elm_interface_atspi_accessible_event_emit(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, data, ELM_INTERFACE_ATSPI_TEXT_EVENT_ACCESS_TEXT_SELECTION_CHANGED, NULL);
}

static void
_entry_selection_cleared_signal_cb(void *data,
                                   Evas_Object *obj EINA_UNUSED,
                                   const char *emission EINA_UNUSED,
                                   const char *source EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(data, sd);

   if (!sd->have_selection) return;

   sd->have_selection = EINA_FALSE;
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_SELECTION_CLEARED, NULL);
   if (!_entry_win_is_wl(data))
     {
        if (sd->cut_sel)
          {
             elm_cnp_selection_set
                (data, ELM_SEL_TYPE_PRIMARY, ELM_SEL_FORMAT_MARKUP,
                 sd->cut_sel, eina_stringshare_strlen(sd->cut_sel));
             elm_cnp_selection_loss_callback_set(data, ELM_SEL_TYPE_PRIMARY, _selection_clear, data);

             ELM_SAFE_FREE(sd->cut_sel, eina_stringshare_del);
          }
        else
          {
             elm_object_cnp_selection_clear(data, ELM_SEL_TYPE_PRIMARY);
          }
     }
   _hide_selection_handler(data);
}

static void
_entry_paste_request_signal_cb(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               const char *emission,
                               const char *source EINA_UNUSED)
{
   Evas_Object *top;

   ELM_ENTRY_DATA_GET(data, sd);

   Elm_Sel_Type type = (emission[sizeof("ntry,paste,request,")] == '1') ?
     ELM_SEL_TYPE_PRIMARY : ELM_SEL_TYPE_CLIPBOARD;

   if (!sd->editable) return;
   if ((type == ELM_SEL_TYPE_PRIMARY) && _entry_win_is_wl(data)) return;
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_SELECTION_PASTE, NULL);

   top = _entry_win_get(data);
   if (top)
     {
        Elm_Sel_Format formats = ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_TEXT;

        sd->selection_asked = EINA_TRUE;

        if (sd->cnp_mode == ELM_CNP_MODE_PLAINTEXT)
          formats = ELM_SEL_FORMAT_TEXT;
        else if (sd->cnp_mode != ELM_CNP_MODE_NO_IMAGE)
          formats |= ELM_SEL_FORMAT_IMAGE;

        elm_cnp_selection_get(data, type, formats, _selection_data_cb, NULL);
     }
}

static void
_entry_copy_notify_signal_cb(void *data,
                             Evas_Object *obj EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
{
   _copy_cb(data, NULL, NULL);
}

static void
_entry_cut_notify_signal_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *emission EINA_UNUSED,
                            const char *source EINA_UNUSED)
{
   _cut_cb(data, NULL, NULL);
}

static void
_entry_cursor_changed_signal_cb(void *data,
                                Evas_Object *obj EINA_UNUSED,
                                const char *emission EINA_UNUSED,
                                const char *source EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(data, sd);
   sd->cursor_pos = edje_object_part_text_cursor_pos_get
       (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
   sd->cur_changed = EINA_TRUE;
   if (elm_widget_focus_get(data))
     edje_object_signal_emit(sd->entry_edje, "elm,action,show,cursor", "elm");
   _cursor_geometry_recalc(data);

   efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_CURSOR_CHANGED, NULL);

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
     (data, ELM_ENTRY_EVENT_CURSOR_CHANGED_MANUAL, NULL);
   if (_elm_config->atspi_mode)
     elm_interface_atspi_accessible_event_emit(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN, data, ELM_INTERFACE_ATSPI_TEXT_EVENT_ACCESS_TEXT_CARET_MOVED, NULL);
}

static void
_signal_anchor_geoms_do_things_with_lol(Elm_Entry_Data *sd,
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

   ELM_ENTRY_DATA_GET(data, sd);

   p = emission + sizeof("nchor,mouse,down,");
   ei.button = strtol(p, &p2, 10);
   ei.name = p2 + 1;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with_lol(sd, &ei);

   if (!sd->disabled)
     efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_ANCHOR_DOWN, &ei);
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

   ELM_ENTRY_DATA_GET(data, sd);

   p = emission + sizeof("nchor,mouse,up,");
   ei.button = strtol(p, &p2, 10);
   ei.name = p2 + 1;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with_lol(sd, &ei);

   if (!sd->disabled)
     efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_ANCHOR_UP, &ei);
}

static void
_anchor_hover_del_cb(void *data,
                     Evas *e EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(data, sd);

   ELM_SAFE_FREE(sd->anchor_hover.pop, evas_object_del);
   evas_object_event_callback_del_full
     (sd->anchor_hover.hover, EVAS_CALLBACK_DEL, _anchor_hover_del_cb, obj);
}

static void
_anchor_hover_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   elm_entry_anchor_hover_end(data);
}

static void
_entry_hover_anchor_clicked_do(Evas_Object *obj,
                               Elm_Entry_Anchor_Info *info)
{
   Evas_Object *hover_parent;
   Evas_Coord x, w, y, h, px, py;
   Elm_Entry_Anchor_Hover_Info ei;

   ELM_ENTRY_DATA_GET(obj, sd);

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

   efl_event_callback_legacy_call(obj, ELM_ENTRY_EVENT_ANCHOR_HOVER_OPENED, &ei);
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
_entry_anchor_clicked_signal_cb(void *data,
                                Evas_Object *obj EINA_UNUSED,
                                const char *emission,
                                const char *source EINA_UNUSED)
{
   Elm_Entry_Anchor_Info ei;
   const char *p;
   char *p2;

   ELM_ENTRY_DATA_GET(data, sd);

   p = emission + sizeof("nchor,mouse,clicked,");
   ei.button = strtol(p, &p2, 10);
   ei.name = p2 + 1;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with_lol(sd, &ei);

   if (!sd->disabled)
     {
        efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_ANCHOR_CLICKED, &ei);
        _entry_hover_anchor_clicked_do(data, &ei);
     }
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

   ELM_ENTRY_DATA_GET(data, sd);

   ei.name = emission + sizeof("nchor,mouse,in,");
   ei.button = 0;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with_lol(sd, &ei);

   if (!sd->disabled)
     efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_ANCHOR_IN, &ei);
}

static void
_entry_anchor_out_signal_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *emission EINA_UNUSED,
                            const char *source EINA_UNUSED)
{
   Elm_Entry_Anchor_Info ei;

   ELM_ENTRY_DATA_GET(data, sd);

   ei.name = emission + sizeof("nchor,mouse,out,");
   ei.button = 0;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with_lol(sd, &ei);

   if (!sd->disabled)
     efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_ANCHOR_OUT, &ei);
}

static void
_entry_key_enter_signal_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_ACTIVATED, NULL);
}

static void
_entry_key_escape_signal_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *emission EINA_UNUSED,
                            const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_ABORTED, NULL);
}

static void
_entry_mouse_down_signal_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *emission EINA_UNUSED,
                            const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, ELM_ENTRY_EVENT_PRESS, NULL);
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
_item_get(void *data,
          Evas_Object *edje EINA_UNUSED,
          const char *part EINA_UNUSED,
          const char *item)
{
   Eina_List *l;
   Evas_Object *o;
   Elm_Entry_Item_Provider *ip;
   const char *style = elm_widget_style_get(data);

   ELM_ENTRY_DATA_GET(data, sd);

   EINA_LIST_FOREACH(sd->item_providers, l, ip)
     {
        o = ip->func(ip->data, data, item);
        if (o) return o;
     }
   if (item && !strncmp(item, "file://", 7))
     {
        const char *fname = item + 7;

        o = evas_object_image_filled_add(evas_object_evas_get(data));
        evas_object_image_load_orientation_set(o, EINA_TRUE);
        evas_object_image_file_set(o, fname, NULL);
        if (evas_object_image_load_error_get(o) == EVAS_LOAD_ERROR_NONE)
          {
             evas_object_show(o);
          }
        else
          {
             evas_object_del(o);
             o = edje_object_add(evas_object_evas_get(data));
             _elm_theme_object_set
               (data, o, "entry/emoticon", "wtf", style);
          }
        return o;
     }

   o = edje_object_add(evas_object_evas_get(data));
   if (!_elm_theme_object_set
         (data, o, "entry", item, style))
     _elm_theme_object_set
       (data, o, "entry/emoticon", "wtf", style);
   return o;
}

static Eina_Bool
_entry_has_new_line(const char *text)
{
   if (!text) return EINA_FALSE;

   while (*text)
     {
        if (!strncmp(text, "<br", 3) || !strncmp(text, "<ps", 3))
          {
             if (text[4] == '>' || ((text[4] == '/') && (text[5] == '>')))
               {
                  return EINA_TRUE;
               }
          }
        text++;
     }

   return EINA_FALSE;
}

static char *
_entry_remove_new_line(const char *text)
{
   Eina_Strbuf *str;
   char *new_text;

   if (!_entry_has_new_line(text)) return NULL;

   str = eina_strbuf_new();
   eina_strbuf_append(str, text);
   eina_strbuf_replace_all(str, "<br>", "");
   eina_strbuf_replace_all(str, "<br/>", "");
   eina_strbuf_replace_all(str, "<ps>", "");
   eina_strbuf_replace_all(str, "<ps/>", "");
   new_text = eina_strbuf_string_steal(str);
   eina_strbuf_free(str);
   return new_text;
}

static void
_entry_new_line_filter_init(Evas_Object *obj)
{
   const char *text;
   char *text2 = NULL;

   if (elm_entry_is_empty(obj)) return;

   text = elm_entry_entry_get(obj);
   text2 = _entry_remove_new_line(text);
   if (text2)
     {
        elm_entry_entry_set(obj, text2);
        free(text2);
     }
}

static void
_entry_new_line_filter_cb(void *data EINA_UNUSED,
                          Evas_Object *entry EINA_UNUSED,
                          char **text)
{
   char *ret;

   if (!*text) return;

   ret = _entry_remove_new_line(*text);

   if (ret)
     {
        free(*text);
        *text = ret;
     }
}

static void
_markup_filter_cb(void *data,
                  Evas_Object *edje EINA_UNUSED,
                  const char *part EINA_UNUSED,
                  char **text)
{
   Eina_List *l;
   Elm_Entry_Markup_Filter *tf;

   ELM_ENTRY_DATA_GET(data, sd);

   EINA_LIST_FOREACH(sd->markup_filters, l, tf)
     {
        tf->func(tf->data, data, text);
        if (!*text)
          break;
     }
}

/* This function is used to insert text by chunks in jobs */
static Eina_Bool
_text_append_idler(void *data)
{
   int start;
   char backup;
   Evas_Object *obj = (Evas_Object *)data;

   ELM_ENTRY_DATA_GET(obj, sd);

   evas_event_freeze(evas_object_evas_get(obj));
   ELM_SAFE_FREE(sd->text, eina_stringshare_del);
   sd->changed = EINA_TRUE;

   start = sd->append_text_position;
   if ((start + ELM_ENTRY_CHUNK_SIZE) < sd->append_text_len)
     {
        int pos = start;
        int tag_start, esc_start;

        tag_start = esc_start = -1;
        /* Find proper markup cut place */
        while (pos - start < ELM_ENTRY_CHUNK_SIZE)
          {
             int prev_pos = pos;
             Eina_Unicode tmp =
               eina_unicode_utf8_next_get(sd->append_text_left, &pos);

             if (esc_start == -1)
               {
                  if (tmp == '<')
                    tag_start = prev_pos;
                  else if (tmp == '>')
                    tag_start = -1;
               }
             if (tag_start == -1)
               {
                  if (tmp == '&')
                    esc_start = prev_pos;
                  else if (tmp == ';')
                    esc_start = -1;
               }
          }

        if (tag_start >= 0)
          {
             sd->append_text_position = tag_start;
          }
        else if (esc_start >= 0)
          {
             sd->append_text_position = esc_start;
          }
        else
          {
             sd->append_text_position = pos;
          }
     }
   else
     {
        sd->append_text_position = sd->append_text_len;
     }

   backup = sd->append_text_left[sd->append_text_position];
   sd->append_text_left[sd->append_text_position] = '\0';

   edje_object_part_text_append
     (sd->entry_edje, "elm.text", sd->append_text_left + start);

   sd->append_text_left[sd->append_text_position] = backup;

   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   _elm_entry_guide_update(obj, EINA_TRUE);

   /* If there's still more to go, renew the idler, else, cleanup */
   if (sd->append_text_position < sd->append_text_len)
     {
        return ECORE_CALLBACK_RENEW;
     }
   else
     {
        edje_object_part_text_cursor_pos_set(sd->entry_edje, "elm.text",
              EDJE_CURSOR_MAIN, sd->cursor_pos);
        free(sd->append_text_left);
        sd->append_text_left = NULL;
        sd->append_text_idler = NULL;
        efl_event_callback_legacy_call
          (obj, ELM_ENTRY_EVENT_TEXT_SET_DONE, NULL);
        return ECORE_CALLBACK_CANCEL;
     }
}

static void
_chars_add_till_limit(Evas_Object *obj,
                      char **text,
                      int can_add,
                      Length_Unit unit)
{
   int i = 0, current_len = 0;
   char *new_text;

   if (!*text) return;
   if (unit >= LENGTH_UNIT_LAST) return;
   if (strstr(*text, "<preedit")) return;

   new_text = *text;
   current_len = strlen(*text);
   while (*new_text)
     {
        int idx = 0, unit_size = 0;
        char *markup, *utfstr;

        if (*new_text == '<')
          {
             while (*(new_text + idx) != '>')
               {
                  idx++;
                  if (!*(new_text + idx)) break;
               }
          }
        else if (*new_text == '&')
          {
             while (*(new_text + idx) != ';')
               {
                  idx++;
                  if (!*(new_text + idx)) break;
               }
          }
        idx = evas_string_char_next_get(new_text, idx, NULL);
        markup = malloc(idx + 1);
        if (markup)
          {
             strncpy(markup, new_text, idx);
             markup[idx] = 0;
             utfstr = elm_entry_markup_to_utf8(markup);
             if (utfstr)
               {
                  if (unit == LENGTH_UNIT_BYTE)
                    unit_size = strlen(utfstr);
                  else if (unit == LENGTH_UNIT_CHAR)
                    unit_size = evas_string_char_len_get(utfstr);
                  ELM_SAFE_FREE(utfstr, free);
               }
             ELM_SAFE_FREE(markup, free);
          }
        if (can_add < unit_size)
          {
             if (!i)
               {
                  efl_event_callback_legacy_call
                    (obj, ELM_ENTRY_EVENT_MAXLENGTH_REACHED, NULL);
                  ELM_SAFE_FREE(*text, free);
                  return;
               }
             can_add = 0;
             strncpy(new_text, new_text + idx,
                     current_len - ((new_text + idx) - *text));
             current_len -= idx;
             (*text)[current_len] = 0;
          }
        else
          {
             new_text += idx;
             can_add -= unit_size;
          }
        i++;
     }

   efl_event_callback_legacy_call(obj, ELM_ENTRY_EVENT_MAXLENGTH_REACHED, NULL);
}

EOLIAN static void
_elm_entry_efl_canvas_layout_signal_signal_emit(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, const char *emission, const char *source)
{
   /* always pass to both edje objs */
   edje_object_signal_emit(sd->entry_edje, emission, source);
   edje_object_message_signal_process(sd->entry_edje);

   if (sd->scr_edje)
     {
        edje_object_signal_emit(sd->scr_edje, emission, source);
        edje_object_message_signal_process(sd->scr_edje);
     }
}

EOLIAN static Eina_Bool
_elm_entry_efl_canvas_layout_signal_signal_callback_add(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Eina_Bool ok;

   ok = efl_canvas_layout_signal_callback_add(sd->entry_edje, emission, source, func_cb, data);
   if (sd->scr_edje)
     ok = efl_canvas_layout_signal_callback_add(sd->scr_edje, emission, source, func_cb, data);

   return ok;
}

EOLIAN static Eina_Bool
_elm_entry_efl_canvas_layout_signal_signal_callback_del(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Eina_Bool ok;

   ok = efl_canvas_layout_signal_callback_del(sd->entry_edje, emission, source, func_cb, data);
   if (sd->scr_edje)
     ok = efl_canvas_layout_signal_callback_del(sd->scr_edje, emission, source, func_cb, data);

   return ok;
}

// Legacy support... del() returns the user data.
void
_elm_entry_signal_callback_add_legacy(Eo *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Elm_Entry_Data *sd;

   sd = efl_data_scope_safe_get(obj, MY_CLASS);
   if (!sd) return;

   _elm_layout_signal_callback_add_legacy(obj, sd->entry_edje, &sd->edje_signals,
                                          emission, source, func_cb, data);

   if (sd->scr_edje)
     efl_canvas_layout_signal_callback_add(sd->scr_edje, emission, source, func_cb, data);
}

void *
_elm_entry_signal_callback_del_legacy(Eo *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb)
{
   Elm_Entry_Data *sd;
   void *data;

   sd = efl_data_scope_safe_get(obj, MY_CLASS);
   if (!sd) return NULL;

   data = _elm_layout_signal_callback_del_legacy(obj, sd->entry_edje, &sd->edje_signals,
                                                 emission, source, func_cb);

   if (sd->scr_edje)
     efl_canvas_layout_signal_callback_del(sd->scr_edje, emission, source, func_cb, data);

   return data;
}

static Eina_Bool
_elm_entry_content_set(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;
   int_ret = efl_content_set(efl_part(efl_super(obj, MY_CLASS), part), content);
   if (!int_ret) return EINA_FALSE;

   /* too bad entry does not follow the pattern
    * "elm,state,{icon,end},visible", we have to repeat ourselves */
   if (!part || !strcmp(part, "icon") || !strcmp(part, "elm.swallow.icon"))
     elm_entry_icon_visible_set(obj, EINA_TRUE);

   if (part && (!strcmp(part, "end") || !strcmp(part, "elm.swallow.end")))
     elm_entry_end_visible_set(obj, EINA_TRUE);

   return EINA_TRUE;
}

static Evas_Object*
_elm_entry_content_unset(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, const char *part)
{
   Evas_Object *ret = NULL;
   ret = efl_content_unset(efl_part(efl_super(obj, MY_CLASS), part));
   if (!ret) return NULL;

   /* too bad entry does not follow the pattern
    * "elm,state,{icon,end},hidden", we have to repeat ourselves */
   if (!part || !strcmp(part, "icon") || !strcmp(part, "elm.swallow.icon"))
     elm_entry_icon_visible_set(obj, EINA_FALSE);

   if (part && (!strcmp(part, "end") || !strcmp(part, "elm.swallow.end")))
     elm_entry_end_visible_set(obj, EINA_FALSE);

   return ret;
}

static void
_entry_text_append(Evas_Object* obj, const char* entry, Eina_Bool set)
{
   int len = 0;
   if (!entry) return;

   ELM_ENTRY_DATA_GET(obj, sd);
   len = strlen(entry);

   if (sd->append_text_left)
     {
        char *tmpbuf;

        tmpbuf = realloc(sd->append_text_left, sd->append_text_len + len + 1);
        if (!tmpbuf)
          {
             /* Do something */
             return;
          }
        sd->append_text_left = tmpbuf;
        memcpy(sd->append_text_left + sd->append_text_len, entry, len + 1);
        sd->append_text_len += len;
     }
   else
     {
        if (len > ELM_ENTRY_CHUNK_SIZE)
          {
             sd->append_text_left = (char *)malloc(len + 1);
          }

        if (sd->append_text_left)
          {
             memcpy(sd->append_text_left, entry, len + 1);
             sd->append_text_position = 0;
             sd->append_text_len = len;
             sd->append_text_idler = ecore_idler_add(_text_append_idler, obj);
          }
        else
          {
             if (set)
               {
                  edje_object_part_text_set(sd->entry_edje, "elm.text", entry);
               }
             else
               {
                  edje_object_part_text_append(sd->entry_edje, "elm.text", entry);
               }
             edje_object_part_text_cursor_pos_set(sd->entry_edje, "elm.text",
                   EDJE_CURSOR_MAIN, sd->cursor_pos);
             efl_event_callback_legacy_call(obj, ELM_ENTRY_EVENT_TEXT_SET_DONE, NULL);
          }
     }
}

static Eina_Bool
_elm_entry_text_set(Eo *obj, Elm_Entry_Data *sd, const char *part, const char *entry)
{
   int len = 0;

   if (!entry) entry = "";
   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE))
     return EINA_FALSE;

   if (strcmp(part, "elm.text"))
     {
        edje_object_part_text_set(sd->entry_edje, part, entry);

        return EINA_TRUE;
     }

   evas_event_freeze(evas_object_evas_get(obj));
   ELM_SAFE_FREE(sd->text, eina_stringshare_del);
   sd->changed = EINA_TRUE;

   /* Clear currently pending job if there is one */
   if (sd->append_text_idler)
     {
        ecore_idler_del(sd->append_text_idler);
        ELM_SAFE_FREE(sd->append_text_left, free);
        sd->append_text_idler = NULL;
     }

   len = strlen(entry);
   if (sd->append_text_left)
     {
        free(sd->append_text_left);
        sd->append_text_left = NULL;
     }

   /* Need to clear the entry first */
   sd->cursor_pos = edje_object_part_text_cursor_pos_get
       (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
   edje_object_part_text_set(sd->entry_edje, "elm.text", "");
   _entry_text_append(obj, entry, EINA_TRUE);

   if (len > 0)
     _elm_entry_guide_update(obj, EINA_TRUE);
   else
     _elm_entry_guide_update(obj, EINA_FALSE);

   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
   return EINA_TRUE;
}

static const char *
_elm_entry_text_get(Eo *obj, Elm_Entry_Data *sd, const char *item)
{
   const char *text;

   if (!_elm_layout_part_aliasing_eval(obj, &item, EINA_TRUE))
     return NULL;

   if (strcmp(item, "elm.text"))
     return edje_object_part_text_get(sd->entry_edje, item);

   text = edje_object_part_text_get(sd->entry_edje, "elm.text");
   if (!text)
     {
        ERR("text=NULL for edje %p, part 'elm.text'", sd->entry_edje);

        return NULL;
     }

   if (sd->append_text_len > 0)
     {
        char *tmpbuf;
        size_t len, tlen;

        tlen = strlen(text);
        len = tlen + sd->append_text_len - sd->append_text_position;
        /* FIXME: need that or we do copy uninitialised data */
        tmpbuf = calloc(1, len + 1);
        if (!tmpbuf)
          {
             ERR("Failed to allocate memory for entry's text %p", obj);
             return NULL;
          }
        memcpy(tmpbuf, text, tlen);

        if (sd->append_text_left)
          memcpy(tmpbuf + tlen, sd->append_text_left
                 + sd->append_text_position, sd->append_text_len
                 - sd->append_text_position);
        tmpbuf[len] = '\0';
        eina_stringshare_replace(&sd->text, tmpbuf);
        free(tmpbuf);
     }
   else
     {
        eina_stringshare_replace(&sd->text, text);
     }

   return sd->text;
}

static char *
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   const char *txt;

   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->password) return NULL;

   txt = elm_widget_access_info_get(obj);

   if (!txt)
     return _elm_util_mkup_to_text(elm_entry_entry_get(obj));
   else return strdup(txt);
}

static char *
_access_state_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   Eina_Strbuf *buf;
   char *ret;

   ELM_ENTRY_DATA_GET(obj, sd);

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
_entry_selection_callbacks_unregister(Evas_Object *obj)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_signal_callback_del_full
     (sd->entry_edje, "selection,start", "elm.text",
     _entry_selection_start_signal_cb, obj);
   edje_object_signal_callback_del_full
     (sd->entry_edje, "selection,changed", "elm.text",
     _entry_selection_changed_signal_cb, obj);
   edje_object_signal_callback_del_full
     (sd->entry_edje, "entry,selection,all,request",
     "elm.text", _entry_selection_all_signal_cb, obj);
   edje_object_signal_callback_del_full
     (sd->entry_edje, "entry,selection,none,request",
     "elm.text", _entry_selection_none_signal_cb, obj);
   edje_object_signal_callback_del_full
     (sd->entry_edje, "selection,cleared", "elm.text",
     _entry_selection_cleared_signal_cb, obj);
   edje_object_signal_callback_del_full
     (sd->entry_edje, "entry,paste,request,*", "elm.text",
     _entry_paste_request_signal_cb, obj);
   edje_object_signal_callback_del_full
     (sd->entry_edje, "entry,copy,notify", "elm.text",
     _entry_copy_notify_signal_cb, obj);
   edje_object_signal_callback_del_full
     (sd->entry_edje, "entry,cut,notify", "elm.text",
     _entry_cut_notify_signal_cb, obj);
}

static void
_entry_selection_callbacks_register(Evas_Object *obj)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_signal_callback_add
     (sd->entry_edje, "selection,start", "elm.text",
     _entry_selection_start_signal_cb, obj);
   edje_object_signal_callback_add
     (sd->entry_edje, "selection,changed", "elm.text",
     _entry_selection_changed_signal_cb, obj);
   edje_object_signal_callback_add
     (sd->entry_edje, "entry,selection,all,request",
     "elm.text", _entry_selection_all_signal_cb, obj);
   edje_object_signal_callback_add
     (sd->entry_edje, "entry,selection,none,request",
     "elm.text", _entry_selection_none_signal_cb, obj);
   edje_object_signal_callback_add
     (sd->entry_edje, "selection,cleared", "elm.text",
     _entry_selection_cleared_signal_cb, obj);
   edje_object_signal_callback_add
     (sd->entry_edje, "entry,paste,request,*", "elm.text",
     _entry_paste_request_signal_cb, obj);
   edje_object_signal_callback_add
     (sd->entry_edje, "entry,copy,notify", "elm.text",
     _entry_copy_notify_signal_cb, obj);
   edje_object_signal_callback_add
     (sd->entry_edje, "entry,cut,notify", "elm.text",
     _entry_cut_notify_signal_cb, obj);
}

static void
_elm_entry_resize_internal(Evas_Object *obj)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->line_wrap)
     {
        elm_layout_sizing_eval(obj);
     }
   else if (sd->scroll)
     {
        Evas_Coord vw = 0, vh = 0;

        elm_interface_scrollable_content_viewport_geometry_get
              (obj, NULL, NULL, &vw, &vh);
        if (vw < sd->ent_mw) vw = sd->ent_mw;
        if (vh < sd->ent_mh) vh = sd->ent_mh;
        evas_object_resize(sd->entry_edje, vw, vh);
     }

   if (sd->hoversel) _hoversel_position(obj);
}

static void
_resize_cb(void *data,
           Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   _elm_entry_resize_internal(data);
}

static void
_selection_handlers_offset_calc(Evas_Object *obj, Evas_Object *handler, Evas_Coord canvasx, Evas_Coord canvasy)
{
   Evas_Coord ex, ey;
   Evas_Coord cx, cy, cw, ch;
   Evas_Coord hh;

   ELM_ENTRY_DATA_GET(obj, sd);

   evas_object_geometry_get(sd->entry_edje, &ex, &ey, NULL, NULL);
   edje_object_part_text_cursor_geometry_get(sd->entry_edje, "elm.text",
                                                           &cx, &cy, &cw, &ch);
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
   ELM_ENTRY_DATA_GET(data, sd);

   Evas_Event_Mouse_Down *ev = event_info;
   int start_pos, end_pos, main_pos, pos;

   sd->start_handler_down = EINA_TRUE;
   start_pos = edje_object_part_text_cursor_pos_get(sd->entry_edje, "elm.text",
                                              EDJE_CURSOR_SELECTION_BEGIN);
   end_pos = edje_object_part_text_cursor_pos_get(sd->entry_edje, "elm.text",
                                              EDJE_CURSOR_SELECTION_END);
   main_pos = edje_object_part_text_cursor_pos_get(sd->entry_edje, "elm.text",
                                                   EDJE_CURSOR_MAIN);
   if (start_pos <= end_pos)
     {
        pos = start_pos;
        sd->sel_handler_cursor = EDJE_CURSOR_SELECTION_BEGIN;
     }
   else
     {
        pos = end_pos;
        sd->sel_handler_cursor = EDJE_CURSOR_SELECTION_END;
     }
   if (pos != main_pos)
     edje_object_part_text_cursor_pos_set(sd->entry_edje, "elm.text",
                                          EDJE_CURSOR_MAIN, pos);
   _selection_handlers_offset_calc(data, sd->start_handler, ev->canvas.x, ev->canvas.y);
}

static void
_start_handler_mouse_up_cb(void *data,
                           Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(data, sd);

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
   ELM_ENTRY_DATA_GET(data, sd);

   if (!sd->start_handler_down) return;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord ex, ey;
   Evas_Coord cx, cy;
   int pos;

   evas_object_geometry_get(sd->entry_edje, &ex, &ey, NULL, NULL);
   cx = ev->cur.canvas.x - sd->ox - ex;
   cy = ev->cur.canvas.y - sd->oy - ey;
   if (cx <= 0) cx = 1;

   edje_object_part_text_cursor_coord_set(sd->entry_edje, "elm.text",
                                        sd->sel_handler_cursor, cx, cy);
   pos = edje_object_part_text_cursor_pos_get(sd->entry_edje, "elm.text",
                                               sd->sel_handler_cursor);
   edje_object_part_text_cursor_pos_set(sd->entry_edje, "elm.text",
                                        EDJE_CURSOR_MAIN, pos);
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
   ELM_ENTRY_DATA_GET(data, sd);

   Evas_Event_Mouse_Down *ev = event_info;
   int pos, start_pos, end_pos, main_pos;

   sd->end_handler_down = EINA_TRUE;
   start_pos = edje_object_part_text_cursor_pos_get(sd->entry_edje, "elm.text",
                                              EDJE_CURSOR_SELECTION_BEGIN);
   end_pos = edje_object_part_text_cursor_pos_get(sd->entry_edje, "elm.text",
                                              EDJE_CURSOR_SELECTION_END);
   if (start_pos < end_pos)
     {
        pos = end_pos;
        sd->sel_handler_cursor = EDJE_CURSOR_SELECTION_END;
     }
   else
     {
        pos = start_pos;
        sd->sel_handler_cursor = EDJE_CURSOR_SELECTION_BEGIN;
     }
   main_pos = edje_object_part_text_cursor_pos_get(sd->entry_edje, "elm.text",
                                                   EDJE_CURSOR_MAIN);
   if (pos != main_pos)
     edje_object_part_text_cursor_pos_set(sd->entry_edje, "elm.text",
                                          EDJE_CURSOR_MAIN, pos);
   _selection_handlers_offset_calc(data, sd->end_handler, ev->canvas.x, ev->canvas.y);
}

static void
_end_handler_mouse_up_cb(void *data,
                         Evas *e EINA_UNUSED,
                         Evas_Object *obj EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(data, sd);

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
   ELM_ENTRY_DATA_GET(data, sd);

   if (!sd->end_handler_down) return;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord ex, ey;
   Evas_Coord cx, cy;
   int pos;

   evas_object_geometry_get(sd->entry_edje, &ex, &ey, NULL, NULL);
   cx = ev->cur.canvas.x - sd->ox - ex;
   cy = ev->cur.canvas.y - sd->oy - ey;
   if (cx <= 0) cx = 1;

   edje_object_part_text_cursor_coord_set(sd->entry_edje, "elm.text",
                                          sd->sel_handler_cursor, cx, cy);
   pos = edje_object_part_text_cursor_pos_get(sd->entry_edje, "elm.text",
                                              sd->sel_handler_cursor);
   edje_object_part_text_cursor_pos_set(sd->entry_edje, "elm.text",
                                        EDJE_CURSOR_MAIN, pos);
   ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
   sd->long_pressed = EINA_FALSE;
   if (_elm_config->magnifier_enable)
     _magnifier_move(data);
}

static void
_entry_on_size_evaluate_signal(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               const char *emission EINA_UNUSED,
                               const char *source EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(data, sd);
   sd->cur_changed = EINA_TRUE;
   elm_entry_calc_force(data);
}

EOLIAN static void
_elm_entry_efl_canvas_group_group_add(Eo *obj, Elm_Entry_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->entry_edje = wd->resize_obj;

   priv->cnp_mode = ELM_CNP_MODE_MARKUP;
   priv->line_wrap = ELM_WRAP_WORD;
   priv->context_menu = EINA_TRUE;
   priv->auto_save = EINA_TRUE;
   priv->editable = EINA_TRUE;
   priv->sel_allow = _elm_config->entry_select_allow;

   priv->drop_format = ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE;
   elm_drop_target_add(obj, priv->drop_format,
                       _dnd_enter_cb, NULL,
                       _dnd_leave_cb, NULL,
                       _dnd_pos_cb, NULL,
                       _dnd_drop_cb, NULL);

   if (!elm_layout_theme_set(obj, "entry", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

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

   edje_object_item_provider_set(priv->entry_edje, _item_get, obj);

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
     (priv->entry_edje, "entry,changed", "elm.text",
     _entry_changed_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "entry,changed,user", "elm.text",
     _entry_changed_user_signal_cb, obj);
   edje_object_signal_callback_add
     (priv->entry_edje, "preedit,changed", "elm.text",
     _entry_preedit_changed_signal_cb, obj);

   _entry_selection_callbacks_register(obj);

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
     (priv->entry_edje, "anchor,mouse,clicked,*", "elm.text",
     _entry_anchor_clicked_signal_cb, obj);
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
   if (priv->sel_allow && _elm_config->desktop_entry)
     edje_object_part_text_select_allow_set
       (priv->entry_edje, "elm.text", EINA_TRUE);

   elm_layout_sizing_eval(obj);

   elm_entry_input_panel_layout_set(obj, ELM_INPUT_PANEL_LAYOUT_NORMAL);
   elm_entry_input_panel_enabled_set(obj, EINA_TRUE);
   elm_entry_prediction_allow_set(obj, EINA_TRUE);
   elm_entry_input_hint_set(obj, ELM_INPUT_HINT_AUTO_COMPLETE);

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

   edje_object_signal_callback_add
      (priv->entry_edje, "size,eval", "elm",
       _entry_on_size_evaluate_signal, obj);
}

static void
_create_selection_handlers(Evas_Object *obj, Elm_Entry_Data *sd)
{
   Evas_Object *handle;
   const char *style = elm_widget_style_get(obj);

   handle = edje_object_add(evas_object_evas_get(obj));
   sd->start_handler = handle;
   _elm_theme_object_set(obj, handle, "entry", "handler/start", style);
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_DOWN,
                                  _start_handler_mouse_down_cb, obj);
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_MOVE,
                                  _start_handler_mouse_move_cb, obj);
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_UP,
                                  _start_handler_mouse_up_cb, obj);
   evas_object_show(handle);

   handle = edje_object_add(evas_object_evas_get(obj));
   sd->end_handler = handle;
   _elm_theme_object_set(obj, handle, "entry", "handler/end", style);
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_DOWN,
                                  _end_handler_mouse_down_cb, obj);
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_MOVE,
                                  _end_handler_mouse_move_cb, obj);
   evas_object_event_callback_add(handle, EVAS_CALLBACK_MOUSE_UP,
                                  _end_handler_mouse_up_cb, obj);
   evas_object_show(handle);
}

EOLIAN static void
_elm_entry_efl_canvas_group_group_del(Eo *obj, Elm_Entry_Data *sd)
{
   Elm_Entry_Context_Menu_Item *it;
   Elm_Entry_Item_Provider *ip;
   Elm_Entry_Markup_Filter *tf;

   if (sd->delay_write)
     {
        ELM_SAFE_FREE(sd->delay_write, ecore_timer_del);
        if (sd->auto_save) _save_do(obj);
     }

   edje_object_signal_callback_del_full
      (sd->entry_edje, "size,eval", "elm",
       _entry_on_size_evaluate_signal, obj);

   if (sd->scroll)
     elm_interface_scrollable_content_viewport_resize_cb_set(obj, NULL);

   elm_entry_anchor_hover_end(obj);
   elm_entry_anchor_hover_parent_set(obj, NULL);

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

   if (sd->prediction_hint)
     {
        ELM_SAFE_FREE(sd->prediction_hint, free);
     }

   eina_stringshare_del(sd->anchor_hover.hover_style);

   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   if (sd->start_handler)
     {
        evas_object_del(sd->start_handler);
        evas_object_del(sd->end_handler);
     }

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_elm_entry_efl_gfx_position_set(Eo *obj, Elm_Entry_Data *sd, Evas_Coord x, Evas_Coord y)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, x, y))
     return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);

   evas_object_move(sd->hit_rect, x, y);

   if (sd->hoversel) _hoversel_position(obj);

   if (edje_object_part_text_selection_get(sd->entry_edje, "elm.text"))
     _update_selection_handler(obj);
}

EOLIAN static void
_elm_entry_efl_gfx_size_set(Eo *obj, Elm_Entry_Data *sd, Evas_Coord w, Evas_Coord h)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, w, h))
     return;

   evas_object_resize(sd->hit_rect, w, h);
   if (sd->have_selection)
     _update_selection_handler(obj);

   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);
}

EOLIAN static void
_elm_entry_efl_gfx_visible_set(Eo *obj, Elm_Entry_Data *sd, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_visible_set(efl_super(obj, MY_CLASS), vis);

   if (sd->have_selection)
     {
        if (vis) _update_selection_handler(obj);
        else _hide_selection_handler(obj);
     }
}

EOLIAN static void
_elm_entry_efl_canvas_group_group_member_add(Eo *obj, Elm_Entry_Data *sd, Evas_Object *member)
{
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

EAPI Evas_Object *
elm_entry_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return efl_add(MY_CLASS, parent, efl_canvas_object_legacy_ctor(efl_added));
}

static void
_cb_added(void *data EINA_UNUSED, const Efl_Event *ev)
{
   const Efl_Callback_Array_Item *event = ev->info;

   ELM_ENTRY_DATA_GET(ev->object, sd);
   if (event->desc == ELM_ENTRY_EVENT_VALIDATE)
     sd->validators++;
}

static void
_cb_deleted(void *data EINA_UNUSED, const Efl_Event *ev)
{
   const Efl_Callback_Array_Item *event = ev->info;

   ELM_ENTRY_DATA_GET(ev->object, sd);
   if (event->desc == ELM_ENTRY_EVENT_VALIDATE)
     sd->validators--;
   return;

}

EOLIAN static Eo *
_elm_entry_efl_object_constructor(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_ENTRY);
   efl_event_callback_add(obj, EFL_EVENT_CALLBACK_ADD, _cb_added, NULL);
   efl_event_callback_add(obj, EFL_EVENT_CALLBACK_DEL, _cb_deleted, NULL);

   return obj;
}

EOLIAN static void
_elm_entry_text_style_user_push(Eo *obj, Elm_Entry_Data *sd, const char *style)
{
   edje_object_part_text_style_user_push(sd->entry_edje, "elm.text", style);
   elm_obj_widget_theme_apply(obj);
}

EOLIAN static void
_elm_entry_text_style_user_pop(Eo *obj, Elm_Entry_Data *sd)
{
   edje_object_part_text_style_user_pop(sd->entry_edje, "elm.text");

   elm_obj_widget_theme_apply(obj);
}

EOLIAN static const char*
_elm_entry_text_style_user_peek(const Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return edje_object_part_text_style_user_peek(sd->entry_edje, "elm.text");
}

EOLIAN static void
_elm_entry_single_line_set(Eo *obj, Elm_Entry_Data *sd, Eina_Bool single_line)
{
   if (sd->single_line == single_line) return;

   sd->single_line = single_line;
   sd->line_wrap = ELM_WRAP_NONE;
   if (elm_entry_cnp_mode_get(obj) == ELM_CNP_MODE_MARKUP)
     elm_entry_cnp_mode_set(obj, ELM_CNP_MODE_NO_IMAGE);
   if (sd->single_line)
     {
        _entry_new_line_filter_init(obj);
        elm_entry_markup_filter_append(obj, _entry_new_line_filter_cb, NULL);
     }
   else
     {
        elm_entry_markup_filter_remove(obj, _entry_new_line_filter_cb, NULL);
     }
   elm_obj_widget_theme_apply(obj);

   if (sd->scroll)
     {
        if (sd->single_line)
           elm_interface_scrollable_policy_set(obj, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
        else
          {
             elm_interface_scrollable_policy_set(obj, sd->policy_h, sd->policy_v);
          }
        elm_layout_sizing_eval(obj);
     }
}

EOLIAN static Eina_Bool
_elm_entry_single_line_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->single_line;
}

EOLIAN static void
_elm_entry_password_set(Eo *obj, Elm_Entry_Data *sd, Eina_Bool password)
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
        elm_entry_input_hint_set(obj, ((sd->input_hints & ~ELM_INPUT_HINT_AUTO_COMPLETE) | ELM_INPUT_HINT_SENSITIVE_DATA));
        _entry_selection_callbacks_unregister(obj);
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

        elm_entry_input_hint_set(obj, ((sd->input_hints | ELM_INPUT_HINT_AUTO_COMPLETE) & ~ELM_INPUT_HINT_SENSITIVE_DATA));
        _entry_selection_callbacks_register(obj);
        elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_ENTRY);
     }

   elm_obj_widget_theme_apply(obj);
}

EOLIAN static Eina_Bool
_elm_entry_password_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->password;
}

EAPI void
elm_entry_entry_set(Evas_Object *obj,
                    const char *entry)
{
   ELM_ENTRY_CHECK(obj);
   efl_text_set(efl_part(obj, "elm.text"), entry);
}

EAPI const char *
elm_entry_entry_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) NULL;
   const char *text = NULL;
   text = efl_text_get(efl_part(obj, "elm.text"));
   return text;
}

EOLIAN static void
_elm_entry_entry_append(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, const char *entry)
{
   if (!entry) entry = "";

   sd->changed = EINA_TRUE;
   _entry_text_append(obj, entry, EINA_FALSE);
}

EOLIAN static Eina_Bool
_elm_entry_is_empty(const Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   edje_object_part_text_cursor_copy
               (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN, EDJE_CURSOR_USER);
   edje_object_part_text_cursor_pos_set
                              (sd->entry_edje, "elm.text", EDJE_CURSOR_USER, 1);
   if (edje_object_part_text_cursor_pos_get
                            (sd->entry_edje, "elm.text", EDJE_CURSOR_USER) == 1)
     return EINA_FALSE;

   return EINA_TRUE;
}

EOLIAN static Evas_Object*
_elm_entry_textblock_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return (Evas_Object *)edje_object_part_object_get
        (sd->entry_edje, "elm.text");
}

EOLIAN static void
_elm_entry_calc_force(Eo *obj, Elm_Entry_Data *sd)
{
   edje_object_calc_force(sd->entry_edje);
   sd->changed = EINA_TRUE;
   elm_layout_sizing_eval(obj);
}

EOLIAN static const char*
_elm_entry_selection_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   if ((sd->password)) return NULL;
   return edje_object_part_text_selection_get(sd->entry_edje, "elm.text");
}

EOLIAN static void
_elm_entry_selection_handler_disabled_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Eina_Bool disabled)
{
   if (sd->sel_handler_disabled == disabled) return;
   sd->sel_handler_disabled = disabled;
}

EOLIAN static Eina_Bool
_elm_entry_selection_handler_disabled_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->sel_handler_disabled;
}

EOLIAN static void
_elm_entry_entry_insert(Eo *obj, Elm_Entry_Data *sd, const char *entry)
{
   edje_object_part_text_insert(sd->entry_edje, "elm.text", entry);
   sd->changed = EINA_TRUE;
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_entry_line_wrap_set(Eo *obj, Elm_Entry_Data *sd, Elm_Wrap_Type wrap)
{
   if (sd->line_wrap == wrap) return;
   sd->last_w = -1;
   sd->line_wrap = wrap;
   if (wrap == ELM_WRAP_NONE)
     ELM_SAFE_FREE(sd->deferred_recalc_job, ecore_job_del);
   elm_obj_widget_theme_apply(obj);
}

EOLIAN static Elm_Wrap_Type
_elm_entry_line_wrap_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->line_wrap;
}

EOLIAN static void
_elm_entry_editable_set(Eo *obj, Elm_Entry_Data *sd, Eina_Bool editable)
{
   if (sd->editable == editable) return;
   sd->editable = editable;
   elm_obj_widget_theme_apply(obj);
   _elm_entry_focus_update(obj, sd);

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
     }
}

EOLIAN static Eina_Bool
_elm_entry_editable_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->editable;
}

EOLIAN static void
_elm_entry_select_none(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
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

EOLIAN static void
_elm_entry_select_all(Eo *obj, Elm_Entry_Data *sd)
{
   if (elm_entry_is_empty(obj)) return;
   if ((sd->password)) return;
   if (sd->sel_mode)
     {
        sd->sel_mode = EINA_FALSE;
        if (!_elm_config->desktop_entry)
          edje_object_part_text_select_allow_set
            (sd->entry_edje, "elm.text", EINA_FALSE);
        edje_object_signal_emit(sd->entry_edje, "elm,state,select,off", "elm");
     }
   edje_object_part_text_select_all(sd->entry_edje, "elm.text");
}

EOLIAN static void
_elm_entry_select_region_set(Eo *obj, Elm_Entry_Data *sd, int start, int end)
{
   if (elm_entry_is_empty(obj)) return;
   if ((sd->password)) return;
   if (sd->sel_mode)
     {
        sd->sel_mode = EINA_FALSE;
        if (!_elm_config->desktop_entry)
          edje_object_part_text_select_allow_set
            (sd->entry_edje, "elm.text", EINA_FALSE);
        edje_object_signal_emit(sd->entry_edje, "elm,state,select,off", "elm");
     }

   /* Set have selection false to not be cleared handler in
      selection_cleared_signal_cb() since that callback will be called while
      resetting edje text. */
   sd->have_selection = EINA_FALSE;

   edje_object_part_text_cursor_pos_set(sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN, start);
   edje_object_part_text_select_begin(sd->entry_edje, "elm.text");
   edje_object_part_text_cursor_pos_set(sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN, end);
   edje_object_part_text_select_extend(sd->entry_edje, "elm.text");
}

EOLIAN static void
_elm_entry_select_region_get(Eo *obj, Elm_Entry_Data *sd, int *start, int *end)
{
   if (!elm_entry_selection_get(obj))
     {
        if (start) *start = -1;
        if (end) *end = -1;
        return;
     }

   if (start)
     *start = edje_object_part_text_cursor_pos_get(sd->entry_edje, "elm.text", EDJE_CURSOR_SELECTION_BEGIN);
   if (end)
     *end = edje_object_part_text_cursor_pos_get(sd->entry_edje, "elm.text", EDJE_CURSOR_SELECTION_END);
}

EOLIAN static Eina_Bool
_elm_entry_cursor_geometry_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   edje_object_part_text_cursor_geometry_get
     (sd->entry_edje, "elm.text", x, y, w, h);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_entry_cursor_next(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return edje_object_part_text_cursor_next
        (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EOLIAN static Eina_Bool
_elm_entry_cursor_prev(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return edje_object_part_text_cursor_prev
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EOLIAN static Eina_Bool
_elm_entry_cursor_up(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return edje_object_part_text_cursor_up
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EOLIAN static Eina_Bool
_elm_entry_cursor_down(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return edje_object_part_text_cursor_down
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EOLIAN static void
_elm_entry_cursor_begin_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   edje_object_part_text_cursor_begin_set
     (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EOLIAN static void
_elm_entry_cursor_end_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   edje_object_part_text_cursor_end_set
     (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EOLIAN static void
_elm_entry_cursor_line_begin_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   edje_object_part_text_cursor_line_begin_set
     (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EOLIAN static void
_elm_entry_cursor_line_end_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   edje_object_part_text_cursor_line_end_set
     (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EOLIAN static void
_elm_entry_cursor_selection_begin(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   edje_object_part_text_select_begin(sd->entry_edje, "elm.text");
}

EOLIAN static void
_elm_entry_cursor_selection_end(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   edje_object_part_text_select_extend(sd->entry_edje, "elm.text");
}

EOLIAN static Eina_Bool
_elm_entry_cursor_is_format_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return edje_object_part_text_cursor_is_format_get
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EOLIAN static Eina_Bool
_elm_entry_cursor_is_visible_format_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return edje_object_part_text_cursor_is_visible_format_get
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EOLIAN static char*
_elm_entry_cursor_content_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return edje_object_part_text_cursor_content_get
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EOLIAN static void
_elm_entry_cursor_pos_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, int pos)
{
   edje_object_part_text_cursor_pos_set
     (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN, pos);
   edje_object_message_signal_process(sd->entry_edje);
}

EOLIAN static int
_elm_entry_cursor_pos_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return edje_object_part_text_cursor_pos_get
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EOLIAN static void
_elm_entry_selection_cut(Eo *obj, Elm_Entry_Data *sd)
{
   if ((sd->password)) return;
   _cut_cb(obj, NULL, NULL);
}

EOLIAN static void
_elm_entry_selection_copy(Eo *obj, Elm_Entry_Data *sd)
{
   if ((sd->password)) return;
   _copy_cb(obj, NULL, NULL);
}

EOLIAN static void
_elm_entry_selection_paste(Eo *obj, Elm_Entry_Data *sd)
{
   if ((sd->password)) return;
   _paste_cb(obj, NULL, NULL);
}

EOLIAN static void
_elm_entry_context_menu_clear(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
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
_elm_entry_context_menu_item_add(Eo *obj, Elm_Entry_Data *sd, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data)
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
_elm_entry_context_menu_disabled_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Eina_Bool disabled)
{
   if (sd->context_menu == !disabled) return;
   sd->context_menu = !disabled;
}

EOLIAN static Eina_Bool
_elm_entry_context_menu_disabled_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return !sd->context_menu;
}

EAPI const char *
elm_entry_context_menu_item_label_get(const Elm_Entry_Context_Menu_Item *item)
{
   if (!item) return NULL;
   return item->label;
}

EAPI void
elm_entry_context_menu_item_icon_get(const Elm_Entry_Context_Menu_Item *item,
                                     const char **icon_file,
                                     const char **icon_group,
                                     Elm_Icon_Type *icon_type)
{
   if (!item) return;
   if (icon_file) *icon_file = item->icon_file;
   if (icon_group) *icon_group = item->icon_group;
   if (icon_type) *icon_type = item->icon_type;
}

EOLIAN static void
_elm_entry_item_provider_append(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Elm_Entry_Item_Provider_Cb func, void *data)
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
_elm_entry_item_provider_prepend(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Elm_Entry_Item_Provider_Cb func, void *data)
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
_elm_entry_item_provider_remove(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Elm_Entry_Item_Provider_Cb func, void *data)
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

EOLIAN static void
_elm_entry_markup_filter_append(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Elm_Entry_Filter_Cb func, void *data)
{
   Elm_Entry_Markup_Filter *tf;

   EINA_SAFETY_ON_NULL_RETURN(func);

   tf = _filter_new(func, data);
   if (!tf) return;

   sd->markup_filters = eina_list_append(sd->markup_filters, tf);
}

EOLIAN static void
_elm_entry_markup_filter_prepend(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Elm_Entry_Filter_Cb func, void *data)
{
   Elm_Entry_Markup_Filter *tf;

   EINA_SAFETY_ON_NULL_RETURN(func);

   tf = _filter_new(func, data);
   if (!tf) return;

   sd->markup_filters = eina_list_prepend(sd->markup_filters, tf);
}

EOLIAN static void
_elm_entry_markup_filter_remove(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Elm_Entry_Filter_Cb func, void *data)
{
   Eina_List *l;
   Elm_Entry_Markup_Filter *tf;

   EINA_SAFETY_ON_NULL_RETURN(func);

   EINA_LIST_FOREACH(sd->markup_filters, l, tf)
     {
        if ((tf->func == func) && ((!data) || (tf->orig_data == data)))
          {
             sd->markup_filters = eina_list_remove_list(sd->markup_filters, l);
             _filter_free(tf);
             return;
          }
     }
}

EAPI char *
elm_entry_markup_to_utf8(const char *s)
{
   char *ss = _elm_util_mkup_to_text(s);
   if (!ss) ss = strdup("");
   return ss;
}

EAPI char *
elm_entry_utf8_to_markup(const char *s)
{
   char *ss = _elm_util_text_to_mkup(s);
   if (!ss) ss = strdup("");
   return ss;
}

static const char *
_text_get(const Evas_Object *obj)
{
   return elm_object_text_get(obj);
}

EAPI void
elm_entry_filter_limit_size(void *data,
                            Evas_Object *entry,
                            char **text)
{
   const char *(*text_get)(const Evas_Object *);
   Elm_Entry_Filter_Limit_Size *lim = data;
   char *current, *utfstr;
   int len, newlen;

   EINA_SAFETY_ON_NULL_RETURN(data);
   EINA_SAFETY_ON_NULL_RETURN(entry);
   EINA_SAFETY_ON_NULL_RETURN(text);

   /* hack. I don't want to copy the entire function to work with
    * scrolled_entry */
   text_get = _text_get;

   current = elm_entry_markup_to_utf8(text_get(entry));
   utfstr = elm_entry_markup_to_utf8(*text);

   if (lim->max_char_count > 0)
     {
        len = evas_string_char_len_get(current);
        newlen = evas_string_char_len_get(utfstr);
        if ((len >= lim->max_char_count) && (newlen > 0))
          {
             efl_event_callback_legacy_call
               (entry, ELM_ENTRY_EVENT_MAXLENGTH_REACHED, NULL);
             ELM_SAFE_FREE(*text, free);
             free(current);
             free(utfstr);
             return;
          }
        if ((len + newlen) > lim->max_char_count)
          _chars_add_till_limit
            (entry, text, (lim->max_char_count - len), LENGTH_UNIT_CHAR);
     }
   else if (lim->max_byte_count > 0)
     {
        len = strlen(current);
        newlen = strlen(utfstr);
        if ((len >= lim->max_byte_count) && (newlen > 0))
          {
             efl_event_callback_legacy_call
               (entry, ELM_ENTRY_EVENT_MAXLENGTH_REACHED, NULL);
             ELM_SAFE_FREE(*text, free);
             free(current);
             free(utfstr);
             return;
          }
        if ((len + newlen) > lim->max_byte_count)
          _chars_add_till_limit
            (entry, text, (lim->max_byte_count - len), LENGTH_UNIT_BYTE);
     }

   free(current);
   free(utfstr);
}

EAPI void
elm_entry_filter_accept_set(void *data,
                            Evas_Object *entry,
                            char **text)
{
   int read_idx, last_read_idx = 0, read_char;
   Elm_Entry_Filter_Accept_Set *as = data;
   Eina_Bool goes_in;
   Eina_Bool rejected = EINA_FALSE;
   const char *set;
   char *insert;

   EINA_SAFETY_ON_NULL_RETURN(data);
   EINA_SAFETY_ON_NULL_RETURN(text);

   if ((!as->accepted) && (!as->rejected))
     return;

   if (as->accepted)
     {
        set = as->accepted;
        goes_in = EINA_TRUE;
     }
   else
     {
        set = as->rejected;
        goes_in = EINA_FALSE;
     }

   insert = *text;
   read_idx = evas_string_char_next_get(*text, 0, &read_char);
   while (read_char)
     {
        int cmp_idx, cmp_char;
        Eina_Bool in_set = EINA_FALSE;

        if (read_char == '<')
          {
             while (read_char && (read_char != '>'))
               read_idx = evas_string_char_next_get(*text, read_idx, &read_char);

             if (goes_in) in_set = EINA_TRUE;
             else in_set = EINA_FALSE;
          }
        else
          {
             if (read_char == '&')
               {
                  while (read_char && (read_char != ';'))
                    read_idx = evas_string_char_next_get(*text, read_idx, &read_char);

                  if (!read_char)
                    {
                       if (goes_in) in_set = EINA_TRUE;
                       else in_set = EINA_FALSE;
                       goto inserting;
                    }
                  if (read_char == ';')
                    {
                       char *tag;
                       int utf8 = 0;
                       tag = malloc(read_idx - last_read_idx + 1);
                       if (tag)
                         {
                            char *markup;
                            strncpy(tag, (*text) + last_read_idx, read_idx - last_read_idx);
                            tag[read_idx - last_read_idx] = 0;
                            markup = elm_entry_markup_to_utf8(tag);
                            free(tag);
                            if (markup)
                              {
                                 utf8 = *markup;
                                 free(markup);
                              }
                            if (!utf8)
                              {
                                 in_set = EINA_FALSE;
                                 goto inserting;
                              }
                            read_char = utf8;
                         }
                    }
               }

             cmp_idx = evas_string_char_next_get(set, 0, &cmp_char);
             while (cmp_char)
               {
                  if (read_char == cmp_char)
                    {
                       in_set = EINA_TRUE;
                       break;
                    }
                  cmp_idx = evas_string_char_next_get(set, cmp_idx, &cmp_char);
               }
          }

inserting:

        if (in_set == goes_in)
          {
             int size = read_idx - last_read_idx;
             const char *src = (*text) + last_read_idx;
             if (src != insert)
               memcpy(insert, *text + last_read_idx, size);
             insert += size;
          }
        else
          {
             rejected = EINA_TRUE;
          }

        if (read_char)
          {
             last_read_idx = read_idx;
             read_idx = evas_string_char_next_get(*text, read_idx, &read_char);
          }
     }
   *insert = 0;
   if (rejected)
     efl_event_callback_legacy_call(entry, ELM_ENTRY_EVENT_REJECTED, NULL);
}

EOLIAN static void
_elm_entry_file_text_format_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Elm_Text_Format format)
{
   sd->format = format;
}

EAPI Eina_Bool
elm_entry_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format)
{
   Eina_Bool ret;
   elm_obj_entry_file_text_format_set(obj, format);
   ret = efl_file_set(obj, file, NULL);
   return ret;
}

EOLIAN static Eina_Bool
_elm_entry_efl_file_file_set(Eo *obj, Elm_Entry_Data *sd, const char *file, const char *group EINA_UNUSED)
{
   ELM_SAFE_FREE(sd->delay_write, ecore_timer_del);
   if (sd->auto_save) _save_do(obj);
   eina_stringshare_replace(&sd->file, file);
   Eina_Bool int_ret = _load_do(obj);
   return int_ret;
}

EAPI void
elm_entry_file_get(const Evas_Object *obj, const char **file, Elm_Text_Format *format)
{
   efl_file_get(obj, file, NULL);
   if (format)
     {
        ELM_ENTRY_DATA_GET(obj, sd);
        if (!sd) return;
        *format = sd->format;
     }
}

EOLIAN static void
_elm_entry_efl_file_file_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, const char **file, const char **group)
{
   if (file) *file = sd->file;
   if (group) *group = NULL;
}

EOLIAN static void
_elm_entry_file_save(Eo *obj, Elm_Entry_Data *sd)
{
   ELM_SAFE_FREE(sd->delay_write, ecore_timer_del);
   _save_do(obj);
   sd->delay_write = ecore_timer_add(ELM_ENTRY_DELAY_WRITE_TIME,
                                     _delay_write, obj);
}

EOLIAN static void
_elm_entry_autosave_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Eina_Bool auto_save)
{
   sd->auto_save = !!auto_save;
}

EOLIAN static Eina_Bool
_elm_entry_autosave_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->auto_save;
}

EINA_DEPRECATED EAPI void
elm_entry_cnp_textonly_set(Evas_Object *obj,
                           Eina_Bool textonly)
{
   Elm_Cnp_Mode cnp_mode = ELM_CNP_MODE_MARKUP;

   ELM_ENTRY_CHECK(obj);

   if (textonly)
     cnp_mode = ELM_CNP_MODE_NO_IMAGE;
   elm_entry_cnp_mode_set(obj, cnp_mode);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_entry_cnp_textonly_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;

   return elm_entry_cnp_mode_get(obj) != ELM_CNP_MODE_MARKUP;
}

EOLIAN static void
_elm_entry_cnp_mode_set(Eo *obj, Elm_Entry_Data *sd, Elm_Cnp_Mode cnp_mode)
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
_elm_entry_cnp_mode_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->cnp_mode;
}

static void
_elm_entry_content_viewport_resize_cb(Evas_Object *obj,
                                      Evas_Coord w EINA_UNUSED, Evas_Coord h EINA_UNUSED)
{
   _elm_entry_resize_internal(obj);
}

static void
_scroll_cb(Evas_Object *obj, void *data EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(obj, sd);
   /* here we need to emit the signal that the elm_scroller would have done */
   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SCROLL, NULL);

   if (sd->have_selection)
     _update_selection_handler(obj);
}

EOLIAN static void
_elm_entry_scrollable_set(Eo *obj, Elm_Entry_Data *sd, Eina_Bool scroll)
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
        elm_interface_scrollable_content_viewport_resize_cb_set(obj, _elm_entry_content_viewport_resize_cb);
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
   sd->last_w = -1;
   elm_obj_widget_theme_apply(obj);
}

EOLIAN static Eina_Bool
_elm_entry_scrollable_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->scroll;
}

EOLIAN static void
_elm_entry_icon_visible_set(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, Eina_Bool setting)
{
   if (!elm_layout_content_get(obj, "elm.swallow.icon")) return;

   if (setting)
     elm_layout_signal_emit(obj, "elm,action,show,icon", "elm");
   else
     elm_layout_signal_emit(obj, "elm,action,hide,icon", "elm");

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_entry_end_visible_set(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, Eina_Bool setting)
{
   if (!elm_layout_content_get(obj, "elm.swallow.end")) return;

   if (setting)
     elm_layout_signal_emit(obj, "elm,action,show,end", "elm");
   else
     elm_layout_signal_emit(obj, "elm,action,hide,end", "elm");

   elm_layout_sizing_eval(obj);
}

EAPI void
elm_entry_scrollbar_policy_set(Evas_Object *obj,
                               Elm_Scroller_Policy h,
                               Elm_Scroller_Policy v)
{
   ELM_ENTRY_CHECK(obj);
   elm_interface_scrollable_policy_set(obj, h, v);
}

EOLIAN static void
_elm_entry_elm_interface_scrollable_policy_set(Eo *obj, Elm_Entry_Data *sd, Elm_Scroller_Policy h, Elm_Scroller_Policy v)
{
   sd->policy_h = h;
   sd->policy_v = v;
   elm_interface_scrollable_policy_set(efl_super(obj, MY_CLASS), sd->policy_h, sd->policy_v);
}

EAPI void
elm_entry_bounce_set(Evas_Object *obj,
                     Eina_Bool h_bounce,
                     Eina_Bool v_bounce)
{
   ELM_ENTRY_CHECK(obj);
   elm_interface_scrollable_bounce_allow_set(obj, h_bounce, v_bounce);
}

EOLIAN static void
_elm_entry_elm_interface_scrollable_bounce_allow_set(Eo *obj, Elm_Entry_Data *sd, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   sd->h_bounce = h_bounce;
   sd->v_bounce = v_bounce;
   elm_interface_scrollable_bounce_allow_set(efl_super(obj, MY_CLASS), h_bounce, v_bounce);
}

EAPI void
elm_entry_bounce_get(const Evas_Object *obj,
                     Eina_Bool *h_bounce,
                     Eina_Bool *v_bounce)
{
   ELM_ENTRY_CHECK(obj);
   elm_interface_scrollable_bounce_allow_get((Eo *) obj, h_bounce, v_bounce);
}

EOLIAN static void
_elm_entry_input_panel_layout_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Elm_Input_Panel_Layout layout)
{
   sd->input_panel_layout = layout;

   edje_object_part_text_input_panel_layout_set
     (sd->entry_edje, "elm.text", (Edje_Input_Panel_Layout)layout);

   switch (layout)
     {
      case ELM_INPUT_PANEL_LAYOUT_URL:
      case ELM_INPUT_PANEL_LAYOUT_EMAIL:
      case ELM_INPUT_PANEL_LAYOUT_PASSWORD:
         elm_entry_autocapital_type_set(obj, ELM_AUTOCAPITAL_TYPE_NONE);
         break;
      default:
         elm_entry_autocapital_type_set(obj, ELM_AUTOCAPITAL_TYPE_SENTENCE);
         break;
     }

   if (layout == ELM_INPUT_PANEL_LAYOUT_PASSWORD)
     elm_entry_input_hint_set(obj, ((sd->input_hints & ~ELM_INPUT_HINT_AUTO_COMPLETE) | ELM_INPUT_HINT_SENSITIVE_DATA));
   else if (layout == ELM_INPUT_PANEL_LAYOUT_TERMINAL)
     elm_entry_input_hint_set(obj, (sd->input_hints & ~ELM_INPUT_HINT_AUTO_COMPLETE));
}

EOLIAN static Elm_Input_Panel_Layout
_elm_entry_input_panel_layout_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->input_panel_layout;
}

EOLIAN static void
_elm_entry_input_panel_layout_variation_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, int variation)
{
   sd->input_panel_layout_variation = variation;

   edje_object_part_text_input_panel_layout_variation_set
     (sd->entry_edje, "elm.text", variation);

   if (sd->input_panel_layout == ELM_INPUT_PANEL_LAYOUT_NORMAL &&
       variation == ELM_INPUT_PANEL_LAYOUT_NORMAL_VARIATION_PERSON_NAME)
     elm_entry_autocapital_type_set(obj, ELM_AUTOCAPITAL_TYPE_WORD);
}

EOLIAN static int
_elm_entry_input_panel_layout_variation_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->input_panel_layout_variation;
}

EOLIAN static void
_elm_entry_autocapital_type_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Elm_Autocapital_Type autocapital_type)
{
   sd->autocapital_type = autocapital_type;
   edje_object_part_text_autocapital_type_set
     (sd->entry_edje, "elm.text", (Edje_Text_Autocapital_Type)autocapital_type);
}

EOLIAN static Elm_Autocapital_Type
_elm_entry_autocapital_type_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->autocapital_type;
}

EOLIAN static void
_elm_entry_prediction_allow_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Eina_Bool prediction)
{
   sd->prediction_allow = prediction;
   edje_object_part_text_prediction_allow_set
     (sd->entry_edje, "elm.text", prediction);
}

EOLIAN static Eina_Bool
_elm_entry_prediction_allow_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->prediction_allow;
}

EOLIAN static void
_elm_entry_input_hint_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Elm_Input_Hints hints)
{
   sd->input_hints = hints;

   edje_object_part_text_input_hint_set
     (sd->entry_edje, "elm.text", (Edje_Input_Hints)hints);
}

EOLIAN static Elm_Input_Hints
_elm_entry_input_hint_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->input_hints;
}

EOLIAN static void
_elm_entry_prediction_hint_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, const char *prediction_hint)
{
   if (sd->prediction_hint)
     free(sd->prediction_hint);

   sd->prediction_hint = strdup(prediction_hint);

   edje_object_part_text_prediction_hint_set
     (sd->entry_edje, "elm.text", prediction_hint);
}

EOLIAN static void
_elm_entry_imf_context_reset(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   edje_object_part_text_imf_context_reset(sd->entry_edje, "elm.text");
}

EOLIAN static void
_elm_entry_input_panel_enabled_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Eina_Bool enabled)
{
   sd->input_panel_enable = enabled;
   edje_object_part_text_input_panel_enabled_set
     (sd->entry_edje, "elm.text", enabled);
}

EOLIAN static Eina_Bool
_elm_entry_input_panel_enabled_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->input_panel_enable;
}

EOLIAN static void
_elm_entry_input_panel_show(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   edje_object_part_text_input_panel_show(sd->entry_edje, "elm.text");
}

EOLIAN static void
_elm_entry_input_panel_hide(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{

   edje_object_part_text_input_panel_hide(sd->entry_edje, "elm.text");
}

EOLIAN static void
_elm_entry_input_panel_language_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Elm_Input_Panel_Lang lang)
{
   sd->input_panel_lang = lang;
   edje_object_part_text_input_panel_language_set
     (sd->entry_edje, "elm.text", (Edje_Input_Panel_Lang)lang);
}

EOLIAN static Elm_Input_Panel_Lang
_elm_entry_input_panel_language_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->input_panel_lang;
}

EOLIAN static void
_elm_entry_input_panel_imdata_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, const void *data, int len)
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
_elm_entry_input_panel_imdata_get(const Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, void *data, int *len)
{
   edje_object_part_text_input_panel_imdata_get
     (sd->entry_edje, "elm.text", data, len);
}

EOLIAN static void
_elm_entry_input_panel_return_key_type_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Elm_Input_Panel_Return_Key_Type return_key_type)
{
   sd->input_panel_return_key_type = return_key_type;

   edje_object_part_text_input_panel_return_key_type_set
     (sd->entry_edje, "elm.text", (Edje_Input_Panel_Return_Key_Type)return_key_type);
}

EOLIAN static Elm_Input_Panel_Return_Key_Type
_elm_entry_input_panel_return_key_type_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->input_panel_return_key_type;
}

EOLIAN static void
_elm_entry_input_panel_return_key_disabled_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Eina_Bool disabled)
{
   sd->input_panel_return_key_disabled = disabled;

   edje_object_part_text_input_panel_return_key_disabled_set
     (sd->entry_edje, "elm.text", disabled);
}

EOLIAN static Eina_Bool
_elm_entry_input_panel_return_key_disabled_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->input_panel_return_key_disabled;
}

EOLIAN static void
_elm_entry_input_panel_return_key_autoenabled_set(Eo *obj, Elm_Entry_Data *sd, Eina_Bool enabled)
{
   sd->auto_return_key = enabled;
   _return_key_enabled_check(obj);
}

EOLIAN static void
_elm_entry_input_panel_show_on_demand_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Eina_Bool ondemand)
{
   sd->input_panel_show_on_demand = ondemand;

   edje_object_part_text_input_panel_show_on_demand_set
     (sd->entry_edje, "elm.text", ondemand);
}

EOLIAN static Eina_Bool
_elm_entry_input_panel_show_on_demand_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->input_panel_show_on_demand;
}

EOLIAN static void*
_elm_entry_imf_context_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   if (!sd) return NULL;

   return edje_object_part_text_imf_context_get(sd->entry_edje, "elm.text");
}

/* START - ANCHOR HOVER */
static void
_anchor_parent_del_cb(void *data,
                      Evas *e EINA_UNUSED,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   ELM_ENTRY_DATA_GET(data, sd);

   sd->anchor_hover.hover_parent = NULL;
}

EOLIAN static void
_elm_entry_anchor_hover_parent_set(Eo *obj, Elm_Entry_Data *sd, Evas_Object *parent)
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

EOLIAN static Evas_Object*
_elm_entry_anchor_hover_parent_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->anchor_hover.hover_parent;
}

EOLIAN static void
_elm_entry_anchor_hover_style_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, const char *style)
{
   eina_stringshare_replace(&sd->anchor_hover.hover_style, style);
}

EOLIAN static const char*
_elm_entry_anchor_hover_style_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->anchor_hover.hover_style;
}

EOLIAN static void
_elm_entry_anchor_hover_end(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   ELM_SAFE_FREE(sd->anchor_hover.hover, evas_object_del);
   ELM_SAFE_FREE(sd->anchor_hover.pop, evas_object_del);
}
/* END - ANCHOR HOVER */

EOLIAN static Eina_Bool
_elm_entry_elm_widget_activate(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, Elm_Activate act)
{
   if (act != ELM_ACTIVATE_DEFAULT) return EINA_FALSE;

   ELM_ENTRY_DATA_GET(obj, sd);

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
_elm_entry_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Entry_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_entry_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Entry_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_elm_entry_select_allow_set(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd, Eina_Bool allow)
{
   if (sd->sel_allow == allow) return;
   sd->sel_allow = allow;

   edje_obj_part_text_select_allow_set(sd->entry_edje, "elm.text", allow);
}

EOLIAN static Eina_Bool
_elm_entry_select_allow_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *sd)
{
   return sd->sel_allow;
}

static void
_elm_entry_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

// ATSPI Accessibility

EOLIAN static Eina_Unicode
_elm_entry_elm_interface_atspi_text_character_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, int offset)
{
   char *txt;
   int idx = 0;
   Eina_Unicode ret = 0;
   if (offset < 0) return ret;

   txt = _elm_util_mkup_to_text(elm_entry_entry_get(obj));
   if (!txt) return ret;

   ret = eina_unicode_utf8_next_get(txt, &idx);
   while (offset--) ret = eina_unicode_utf8_next_get(txt, &idx);

   free(txt);

   if (_pd->password)
     ret = ENTRY_PASSWORD_MASK_CHARACTER;

   return ret;
}

EOLIAN static int
_elm_entry_elm_interface_atspi_text_character_count_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED)
{
   char *txt;
   int ret = -1;

   txt = _elm_util_mkup_to_text(elm_entry_entry_get(obj));
   if (!txt) return ret;

   ret = eina_unicode_utf8_get_len(txt);
   free(txt);

   return ret;
}

EOLIAN static char*
_elm_entry_elm_interface_atspi_text_string_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, Elm_Atspi_Text_Granularity granularity, int *start_offset, int *end_offset)
{
   Evas_Textblock_Cursor *cur = NULL, *cur2 = NULL;
   Evas_Object *tblk;
   char *ret = NULL;

   tblk = elm_entry_textblock_get(obj);
   if (!tblk) goto fail;

   cur = evas_object_textblock_cursor_new(tblk);
   cur2 = evas_object_textblock_cursor_new(tblk);
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

   if (ret && _pd->password)
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
_elm_entry_elm_interface_atspi_text_text_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, int start_offset, int end_offset)
{
   Evas_Textblock_Cursor *cur = NULL, *cur2 = NULL;
   Evas_Object *tblk;
   char *ret = NULL;

   tblk = elm_entry_textblock_get(obj);
   if (!tblk) goto fail;

   cur = evas_object_textblock_cursor_new(tblk);
   cur2 = evas_object_textblock_cursor_new(tblk);
   if (!cur || !cur2) goto fail;

   evas_textblock_cursor_pos_set(cur, start_offset);
   if (evas_textblock_cursor_pos_get(cur) != start_offset) goto fail;

   evas_textblock_cursor_pos_set(cur2, end_offset);
   if (evas_textblock_cursor_pos_get(cur2) != end_offset) goto fail;

   ret = evas_textblock_cursor_range_text_get(cur, cur2, EVAS_TEXTBLOCK_TEXT_PLAIN);

   evas_textblock_cursor_free(cur);
   evas_textblock_cursor_free(cur2);

   if (ret && _pd->password)
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
_elm_entry_elm_interface_atspi_text_caret_offset_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED)
{
   return elm_entry_cursor_pos_get(obj);
}

EOLIAN static Eina_Bool
_elm_entry_elm_interface_atspi_text_caret_offset_set(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, int offset)
{
   elm_entry_cursor_pos_set(obj, offset);
   return EINA_TRUE;
}

EOLIAN static int
_elm_entry_elm_interface_atspi_text_selections_count_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED)
{
   return elm_entry_selection_get(obj) ? 1 : 0;
}

EOLIAN static void
_elm_entry_elm_interface_atspi_text_selection_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, int selection_number, int *start_offset, int *end_offset)
{
   if (selection_number != 0) return;

   elm_obj_entry_select_region_get(obj, start_offset, end_offset);
}

EOLIAN static Eina_Bool
_elm_entry_elm_interface_atspi_text_selection_set(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, int selection_number, int start_offset, int end_offset)
{
   if (selection_number != 0) return EINA_FALSE;

   elm_entry_select_region_set(obj, start_offset, end_offset);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_entry_elm_interface_atspi_text_selection_remove(Eo *obj, Elm_Entry_Data *pd EINA_UNUSED, int selection_number)
{
   if (selection_number != 0) return EINA_FALSE;
   elm_entry_select_none(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_entry_elm_interface_atspi_text_selection_add(Eo *obj, Elm_Entry_Data *pd EINA_UNUSED, int start_offset, int end_offset)
{
   elm_entry_select_region_set(obj, start_offset, end_offset);

   return EINA_TRUE;
}

EOLIAN static Eina_List*
_elm_entry_elm_interface_atspi_text_bounded_ranges_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *_pd EINA_UNUSED, Eina_Bool screen_coods EINA_UNUSED, Eina_Rectangle rect EINA_UNUSED, Elm_Atspi_Text_Clip_Type xclip EINA_UNUSED, Elm_Atspi_Text_Clip_Type yclip EINA_UNUSED)
{
   return NULL;
}

EOLIAN static int
_elm_entry_elm_interface_atspi_text_offset_at_point_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, Eina_Bool screen_coods, int x, int y)
{
   Evas_Object *txtblk;
   Evas_Textblock_Cursor *cur;
   int ret;

   txtblk = elm_entry_textblock_get(obj);
   if (!txtblk) return -1;

   cur = evas_object_textblock_cursor_new(txtblk);
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
_elm_entry_elm_interface_atspi_text_character_extents_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, int offset, Eina_Bool screen_coods, Eina_Rectangle *rect)
{
   Evas_Object *txtblk;
   Evas_Textblock_Cursor *cur;
   int ret;

   txtblk = elm_entry_textblock_get(obj);
   if (!txtblk) return EINA_FALSE;

   cur = evas_object_textblock_cursor_new(txtblk);
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
_elm_entry_elm_interface_atspi_text_range_extents_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, Eina_Bool screen_coods, int start_offset, int end_offset, Eina_Rectangle *rect)
{
   Evas_Object *txtblk;
   Evas_Textblock_Cursor *cur1, *cur2;
   int ret;
   int x, xx, y, yy;

   txtblk = elm_entry_textblock_get(obj);
   if (!txtblk) return EINA_FALSE;

   cur1 = evas_object_textblock_cursor_new(txtblk);
   if (!cur1) return EINA_FALSE;

   cur2 = evas_object_textblock_cursor_new(txtblk);
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
_textblock_node_format_to_atspi_text_attr(const Evas_Object_Textblock_Node_Format *format)
{
   Elm_Atspi_Text_Attribute *ret = NULL;
   const char *txt;

   txt = evas_textblock_node_format_text_get(format);
   if (!txt) return NULL;

   if (txt[0] == '-') return NULL; // skip closing format

   if (!strncmp(txt, "+ ", 2))
     {
        const char *tmp = &txt[2];

        while (*tmp != '\0' && *tmp != '=') tmp++;
        if (*tmp++ != '=') return NULL;

        ret = calloc(1, sizeof(Elm_Atspi_Text_Attribute));
        if (!ret) return NULL;

        ret->value = eina_stringshare_add(tmp);
        int size = &txt[2] - tmp + 1;
        ret->name = eina_stringshare_add_length(&txt[2], size > 0 ? size : -size);
     }

   return ret;
}

EOLIAN static Eina_Bool
_elm_entry_elm_interface_atspi_text_attribute_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, const char *attr_name EINA_UNUSED, int *start_offset, int *end_offset, char **value)
{
   Evas_Object *txtblk;
   Evas_Textblock_Cursor *cur1, *cur2;
   Eina_List *formats, *l;
   Evas_Object_Textblock_Node_Format *format;
   Elm_Atspi_Text_Attribute *attr;

   txtblk = elm_entry_textblock_get(obj);
   if (!txtblk) return EINA_FALSE;

   cur1 = evas_object_textblock_cursor_new(txtblk);
   if (!cur1) return EINA_FALSE;

   cur2 = evas_object_textblock_cursor_new(txtblk);
   if (!cur2)
     {
        evas_textblock_cursor_free(cur1);
        return EINA_FALSE;
     }

   evas_textblock_cursor_pos_set(cur1, *start_offset);
   evas_textblock_cursor_pos_set(cur2, *end_offset);

   formats = evas_textblock_cursor_range_formats_get(cur1, cur2);

   evas_textblock_cursor_free(cur1);
   evas_textblock_cursor_free(cur2);

   if (!formats) return EINA_FALSE;

   EINA_LIST_FOREACH(formats, l , format)
     {
        attr = _textblock_node_format_to_atspi_text_attr(format);
        if (!attr) continue;
        if (!strcmp(attr->name, attr_name))
          {
             *value = attr->value ? strdup(attr->value) : NULL;
             elm_atspi_text_text_attribute_free(attr);
             return EINA_TRUE;
          }
        elm_atspi_text_text_attribute_free(attr);
     }

   return EINA_FALSE;
}

EOLIAN static Eina_List*
_elm_entry_elm_interface_atspi_text_attributes_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, int *start_offset, int *end_offset)
{
   Evas_Object *txtblk;
   Evas_Textblock_Cursor *cur1, *cur2;
   Eina_List *formats, *ret = NULL, *l;
   Evas_Object_Textblock_Node_Format *format;
   Elm_Atspi_Text_Attribute *attr;

   txtblk = elm_entry_textblock_get(obj);
   if (!txtblk) return NULL;

   cur1 = evas_object_textblock_cursor_new(txtblk);
   if (!cur1) return NULL;

   cur2 = evas_object_textblock_cursor_new(txtblk);
   if (!cur2)
     {
        evas_textblock_cursor_free(cur1);
        return NULL;
     }

   evas_textblock_cursor_pos_set(cur1, *start_offset);
   evas_textblock_cursor_pos_set(cur2, *end_offset);

   formats = evas_textblock_cursor_range_formats_get(cur1, cur2);

   evas_textblock_cursor_free(cur1);
   evas_textblock_cursor_free(cur2);

   if (!formats) return NULL;

   EINA_LIST_FOREACH(formats, l , format)
     {
        attr = _textblock_node_format_to_atspi_text_attr(format);
        if (!attr) continue;
        ret = eina_list_append(ret, attr);
     }

   return ret;
}

EOLIAN static Eina_List*
_elm_entry_elm_interface_atspi_text_default_attributes_get(Eo *obj EINA_UNUSED, Elm_Entry_Data *_pd EINA_UNUSED)
{
   Evas_Object *txtblk;
   Eina_List *ret = NULL;
   const Evas_Object_Textblock_Node_Format *format;
   Elm_Atspi_Text_Attribute *attr;

   txtblk = elm_entry_textblock_get(obj);
   if (!txtblk) return NULL;

   format = evas_textblock_node_format_first_get(txtblk);
   if (!format) return NULL;

   do
     {
        attr = _textblock_node_format_to_atspi_text_attr(format);
        if (!attr) continue;
        ret = eina_list_append(ret, attr);
     }
   while ((format = evas_textblock_node_format_next_get(format)) != NULL);

   return ret;
}

EOLIAN static Eina_Bool
_elm_entry_elm_interface_atspi_text_editable_content_set(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, const char *content)
{
   elm_entry_entry_set(obj, content);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_entry_elm_interface_atspi_text_editable_insert(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, const char *string, int position)
{
   elm_entry_cursor_pos_set(obj, position);
   elm_entry_entry_insert(obj, string);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_entry_elm_interface_atspi_text_editable_copy(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, int start, int end)
{
   elm_entry_select_region_set(obj, start, end);
   elm_entry_selection_copy(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_entry_elm_interface_atspi_text_editable_delete(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, int start_offset, int end_offset)
{
   Evas_Object *txtblk;
   Evas_Textblock_Cursor *cur1, *cur2;

   txtblk = elm_entry_textblock_get(obj);
   if (!txtblk) return EINA_FALSE;

   cur1 = evas_object_textblock_cursor_new(txtblk);
   if (!cur1) return EINA_FALSE;

   cur2 = evas_object_textblock_cursor_new(txtblk);
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

   elm_entry_calc_force(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_entry_elm_interface_atspi_text_editable_paste(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, int position)
{
   elm_entry_cursor_pos_set(obj, position);
   elm_entry_selection_paste(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_entry_elm_interface_atspi_text_editable_cut(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED, int start, int end)
{
   elm_entry_select_region_set(obj, start, end);
   elm_entry_selection_cut(obj);
   return EINA_TRUE;
}

EOLIAN static Elm_Atspi_State_Set
_elm_entry_elm_interface_atspi_accessible_state_set_get(Eo *obj, Elm_Entry_Data *_pd EINA_UNUSED)
{
   Elm_Atspi_State_Set ret;
   ret = elm_interface_atspi_accessible_state_set_get(efl_super(obj, ELM_ENTRY_CLASS));

   if (elm_entry_editable_get(obj))
     STATE_TYPE_SET(ret, ELM_ATSPI_STATE_EDITABLE);

   return ret;
}

EOLIAN static const char*
_elm_entry_elm_interface_atspi_accessible_name_get(Eo *obj, Elm_Entry_Data *sd)
{
   const char *name;
   name = elm_interface_atspi_accessible_name_get(efl_super(obj, ELM_ENTRY_CLASS));
   if (name && strncmp("", name, 1)) return name;
   const char *ret = edje_object_part_text_get(sd->entry_edje, "elm.guide");
   return ret;
}

/* Efl.Part begin */

ELM_PART_OVERRIDE(elm_entry, ELM_ENTRY, ELM_LAYOUT, Elm_Entry_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_CONTENT_SET(elm_entry, ELM_ENTRY, ELM_LAYOUT, Elm_Entry_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(elm_entry, ELM_ENTRY, ELM_LAYOUT, Elm_Entry_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_TEXT_SET(elm_entry, ELM_ENTRY, ELM_LAYOUT, Elm_Entry_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_TEXT_GET(elm_entry, ELM_ENTRY, ELM_LAYOUT, Elm_Entry_Data, Elm_Part_Data)
ELM_PART_CONTENT_DEFAULT_SET(elm_entry, "icon")
#include "elm_entry_internal_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT()
ELM_LAYOUT_TEXT_ALIASES_IMPLEMENT()

#define ELM_ENTRY_EXTRA_OPS \
   ELM_PART_CONTENT_DEFAULT_OPS(elm_entry), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_entry), \
   ELM_LAYOUT_CONTENT_ALIASES_OPS(), \
   ELM_LAYOUT_TEXT_ALIASES_OPS()

#include "elm_entry.eo.c"
