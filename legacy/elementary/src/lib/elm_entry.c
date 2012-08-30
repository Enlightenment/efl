#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "elm_widget_entry.h"

EAPI const char ELM_ENTRY_SMART_NAME[] = "elm_entry";

/* Maximum chunk size to be inserted to the entry at once
 * FIXME: This size is arbitrary, should probably choose a better size.
 * Possibly also find a way to set it to a low value for weak computers,
 * and to a big value for better computers. */
#define _CHUNK_SIZE 10000

static const char SIG_ABORTED[] = "aborted";
static const char SIG_ACTIVATED[] = "activated";
static const char SIG_ANCHOR_CLICKED[] = "anchor,clicked";
static const char SIG_ANCHOR_DOWN[] = "anchor,down";
static const char SIG_ANCHOR_HOVER_OPENED[] = "anchor,hover,opened";
static const char SIG_ANCHOR_IN[] = "anchor,in";
static const char SIG_ANCHOR_OUT[] = "anchor,out";
static const char SIG_ANCHOR_UP[] = "anchor,up";
static const char SIG_CHANGED[] = "changed";
static const char SIG_CHANGED_USER[] = "changed,user";
static const char SIG_CLICKED[] = "clicked";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_CLICKED_TRIPLE[] = "clicked,triple";
static const char SIG_CURSOR_CHANGED[] = "cursor,changed";
static const char SIG_CURSOR_CHANGED_MANUAL[] = "cursor,changed,manual";
static const char SIG_FOCUSED[] = "focused";
static const char SIG_LANG_CHANGED[] = "language,changed";
static const char SIG_LONGPRESSED[] = "longpressed";
static const char SIG_MAX_LENGHT[] = "maxlength,reached";
static const char SIG_PREEDIT_CHANGED[] = "preedit,changed";
static const char SIG_PRESS[] = "press";
static const char SIG_REDO_REQUEST[] = "redo,request";
static const char SIG_SELECTION_CHANGED[] = "selection,changed";
static const char SIG_SELECTION_CLEARED[] = "selection,cleared";
static const char SIG_SELECTION_COPY[] = "selection,copy";
static const char SIG_SELECTION_CUT[] = "selection,cut";
static const char SIG_SELECTION_PASTE[] = "selection,paste";
static const char SIG_SELECTION_START[] = "selection,start";
static const char SIG_THEME_CHANGED[] = "theme,changed";
static const char SIG_UNDO_REQUEST[] = "undo,request";
static const char SIG_UNFOCUSED[] = "unfocused";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_ABORTED, ""},
   {SIG_ACTIVATED, ""},
   {SIG_ANCHOR_CLICKED, ""},
   {SIG_ANCHOR_DOWN, ""},
   {SIG_ANCHOR_HOVER_OPENED, ""},
   {SIG_ANCHOR_IN, ""},
   {SIG_ANCHOR_OUT, ""},
   {SIG_ANCHOR_UP, ""},
   {SIG_CHANGED, ""},
   {SIG_CHANGED_USER, ""},
   {SIG_CLICKED, ""},
   {SIG_CLICKED_DOUBLE, ""},
   {SIG_CLICKED_TRIPLE, ""},
   {SIG_CURSOR_CHANGED, ""},
   {SIG_CURSOR_CHANGED_MANUAL, ""},
   {SIG_FOCUSED, ""},
   {SIG_LANG_CHANGED, ""},
   {SIG_LONGPRESSED, ""},
   {SIG_MAX_LENGHT, ""},
   {SIG_PREEDIT_CHANGED, ""},
   {SIG_PRESS, ""},
   {SIG_REDO_REQUEST, ""},
   {SIG_SELECTION_CHANGED, ""},
   {SIG_SELECTION_CLEARED, ""},
   {SIG_SELECTION_COPY, ""},
   {SIG_SELECTION_CUT, ""},
   {SIG_SELECTION_PASTE, ""},
   {SIG_SELECTION_START, ""},
   {SIG_THEME_CHANGED, ""},
   {SIG_UNDO_REQUEST, ""},
   {SIG_UNFOCUSED, ""},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"icon", "elm.swallow.icon"},
   {"end", "elm.swallow.end"},
   {NULL, NULL}
};

static const Evas_Smart_Interface *_smart_interfaces[] =
{
   (Evas_Smart_Interface *)&ELM_SCROLLABLE_IFACE, NULL
};

EVAS_SMART_SUBCLASS_IFACE_NEW
  (ELM_ENTRY_SMART_NAME, _elm_entry, Elm_Entry_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks,
  _smart_interfaces);

static Eina_List *entries = NULL;

struct _Mod_Api
{
   void (*obj_hook)(Evas_Object *obj);
   void (*obj_unhook)(Evas_Object *obj);
   void (*obj_longpress)(Evas_Object *obj);
};

static Mod_Api *
_module_find(Evas_Object *obj __UNUSED__)
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
_buf_append(char *buf,
            const char *str,
            int *len,
            int *alloc)
{
   int len2 = strlen(str);

   if ((*len + len2) >= *alloc)
     {
        char *buf2 = realloc(buf, *alloc + len2 + 512);
        if (!buf2) return NULL;
        buf = buf2;
        *alloc += (512 + len2);
     }

   strcpy(buf + *len, str);
   *len += len2;

   return buf;
}

static char *
_file_load(const char *file)
{
   FILE *f;
   size_t size;
   int alloc = 0, len = 0;
   char *text = NULL, buf[16384 + 1];

   f = fopen(file, "rb");
   if (!f) return NULL;
   while ((size = fread(buf, 1, sizeof(buf) - 1, f)))
     {
        char *tmp_text;

        buf[size] = 0;
        tmp_text = _buf_append(text, buf, &len, &alloc);
        if (!tmp_text) break;
        text = tmp_text;
     }
   fclose(f);

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

   if ((!text) || (!text[0]))
     {
        ecore_file_unlink(file);

        return;
     }

   f = fopen(file, "wb");
   if (!f)
     {
        // FIXME: report a write error
        return;
     }

   fputs(text, f); // FIXME: catch error
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

static Elm_Entry_Markup_Filter *
_filter_new(Elm_Entry_Filter_Cb func,
            void *data)
{
   Elm_Entry_Markup_Filter *tf = ELM_NEW(Elm_Entry_Markup_Filter);

   if (!tf) return NULL;

   tf->func = func;
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

        if (lim) free(lim);
     }
   else if (tf->func == elm_entry_filter_accept_set)
     {
        Elm_Entry_Filter_Accept_Set *as = tf->data;

        if (as)
          {
             if (as->accepted) eina_stringshare_del(as->accepted);
             if (as->rejected) eina_stringshare_del(as->rejected);

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
     elm_widget_mirrored_set(sd->anchor_hover.hover, rtl);
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

/* we can't reuse layout's here, because it's on entry_edje only */
static Eina_Bool
_elm_entry_smart_disable(Evas_Object *obj)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   if (elm_object_disabled_get(obj))
     edje_object_signal_emit(sd->entry_edje, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(sd->entry_edje, "elm,state,enabled", "elm");

   return EINA_TRUE;
}

/* we can't issue the layout's theming code here, cause it assumes an
 * unique edje object, always */
static Eina_Bool
_elm_entry_smart_theme(Evas_Object *obj)
{
   const char *t;
   Elm_Widget_Smart_Class *parent_parent =
     (Elm_Widget_Smart_Class *)((Evas_Smart_Class *)
                                _elm_entry_parent_sc)->parent;

   ELM_ENTRY_DATA_GET(obj, sd);

   if (!parent_parent->theme(obj))
     return EINA_FALSE;

   evas_event_freeze(evas_object_evas_get(obj));

   edje_object_mirrored_set
     (ELM_WIDGET_DATA(sd)->resize_obj, elm_widget_mirrored_get(obj));

   edje_object_scale_set
     (ELM_WIDGET_DATA(sd)->resize_obj,
     elm_widget_scale_get(obj) * elm_config_scale_get());

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   t = eina_stringshare_add(elm_object_text_get(obj));

   elm_widget_theme_object_set
     (obj, sd->entry_edje, "entry", _elm_entry_theme_group_get(obj),
     elm_widget_style_get(obj));

   if (_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set
       (sd->entry_edje, "elm.text", EINA_TRUE);

   elm_object_text_set(obj, t);
   eina_stringshare_del(t);

   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(sd->entry_edje, "elm,state,disabled", "elm");

   edje_object_part_text_input_panel_layout_set
     (sd->entry_edje, "elm.text", sd->input_panel_layout);
   edje_object_part_text_autocapital_type_set
     (sd->entry_edje, "elm.text", sd->autocapital_type);
   edje_object_part_text_prediction_allow_set
     (sd->entry_edje, "elm.text", sd->prediction_allow);
   edje_object_part_text_input_panel_enabled_set
     (sd->entry_edje, "elm.text", sd->input_panel_enable);
   edje_object_part_text_input_panel_imdata_set
     (sd->entry_edje, "elm.text", sd->input_panel_imdata,
     sd->input_panel_imdata_len);
   edje_object_part_text_input_panel_return_key_type_set
     (sd->entry_edje, "elm.text", sd->input_panel_return_key_type);
   edje_object_part_text_input_panel_return_key_disabled_set
     (sd->entry_edje, "elm.text", sd->input_panel_return_key_disabled);

   if (sd->cursor_pos != 0)
     elm_entry_cursor_pos_set(obj, sd->cursor_pos);

   if (elm_widget_focus_get(obj))
     edje_object_signal_emit(sd->entry_edje, "elm,action,focus", "elm");

   edje_object_message_signal_process(sd->entry_edje);

   if (sd->scroll)
     {
        const char *str;

        sd->s_iface->mirrored_set(obj, elm_widget_mirrored_get(obj));

        elm_widget_theme_object_set
          (obj, sd->scr_edje, "scroller", "entry", elm_widget_style_get(obj));

        str = edje_object_data_get(sd->scr_edje, "focus_highlight");
        if ((str) && (!strcmp(str, "on")))
          elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
        else
          elm_widget_highlight_in_theme_set(obj, EINA_FALSE);
     }

   elm_layout_sizing_eval(obj);

   sd->has_text = !sd->has_text;
   _elm_entry_guide_update(obj, !sd->has_text);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   evas_object_smart_callback_call(obj, SIG_THEME_CHANGED, NULL);

   return EINA_TRUE;
}

static void
_cursor_geometry_recalc(Evas_Object *obj)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   evas_object_smart_callback_call(obj, SIG_CURSOR_CHANGED, NULL);

   if (!sd->deferred_recalc_job)
     {
        Evas_Coord cx, cy, cw, ch;

        edje_object_part_text_cursor_geometry_get
          (sd->entry_edje, "elm.text", &cx, &cy, &cw, &ch);
        if (sd->cur_changed)
          {
             elm_widget_show_region_set(obj, cx, cy, cw, ch, EINA_FALSE);
             sd->cur_changed = EINA_FALSE;
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

        evas_object_size_hint_min_get(data, &ominw, NULL);
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
             elm_widget_show_region_set(data, cx, cy, cw, ch, EINA_FALSE);
             sd->cur_changed = EINA_FALSE;
          }
     }
}

static void
_elm_entry_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord resw, resh;

   ELM_ENTRY_DATA_GET(obj, sd);

   evas_object_geometry_get(obj, NULL, NULL, &resw, &resh);

   if (sd->line_wrap)
     {
        if ((resw == sd->last_w) && (!sd->changed))
          {
             if (sd->scroll)
               {
                  Evas_Coord vw = 0, vh = 0, w = 0, h = 0;

                  sd->s_iface->content_viewport_size_get(obj, &vw, &vh);

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
             sd->s_iface->content_viewport_size_get(obj, &vw, &vh);
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

                  evas_object_size_hint_min_get(sd->entry_edje, &ominw, NULL);
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
             if (sd->deferred_recalc_job)
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

             sd->s_iface->content_viewport_size_get(obj, &vw, &vh);

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

static Eina_Bool
_elm_entry_smart_on_focus(Evas_Object *obj)
{
   Evas_Object *top;

   ELM_ENTRY_DATA_GET(obj, sd);

   top = elm_widget_top_get(obj);

   if (!sd->editable) return EINA_FALSE;
   if (elm_widget_focus_get(obj))
     {
        evas_object_focus_set(sd->entry_edje, EINA_TRUE);
        edje_object_signal_emit(sd->entry_edje, "elm,action,focus", "elm");
        if (top && sd->input_panel_enable)
          elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_ON);
        evas_object_smart_callback_call(obj, SIG_FOCUSED, NULL);
        _return_key_enabled_check(obj);
     }
   else
     {
        edje_object_signal_emit(sd->entry_edje, "elm,action,unfocus", "elm");
        evas_object_focus_set(sd->entry_edje, EINA_FALSE);
        if (top && sd->input_panel_enable)
          elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_OFF);
        evas_object_smart_callback_call(obj, SIG_UNFOCUSED, NULL);
     }

   return EINA_TRUE;
}

static Eina_Bool
_elm_entry_smart_translate(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, SIG_LANG_CHANGED, NULL);

   return EINA_TRUE;
}

static void
_elm_entry_smart_on_focus_region(const Evas_Object *obj,
                                 Evas_Coord *x,
                                 Evas_Coord *y,
                                 Evas_Coord *w,
                                 Evas_Coord *h)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_cursor_geometry_get
     (sd->entry_edje, "elm.text", x, y, w, h);
}

static void
_show_region_hook(void *data,
                  Evas_Object *obj)
{
   Evas_Coord x, y, w, h;

   ELM_ENTRY_DATA_GET(data, sd);

   elm_widget_show_region_get(obj, &x, &y, &w, &h);

   if (sd->scroll)
     sd->s_iface->content_region_show(obj, x, y, w, h);
}

static Eina_Bool
_elm_entry_smart_sub_object_del(Evas_Object *obj,
                                Evas_Object *sobj)
{
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

   if (!ELM_WIDGET_CLASS(_elm_entry_parent_sc)->sub_object_del(obj, sobj))
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
_hoversel_position(Evas_Object *obj)
{
   Evas_Coord cx, cy, cw, ch, x, y, mw, mh;

   ELM_ENTRY_DATA_GET(obj, sd);

   cx = cy = 0;
   cw = ch = 1;
   evas_object_geometry_get(sd->entry_edje, &x, &y, NULL, NULL);
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
   if (cw < mw)
     {
        cx += (cw - mw) / 2;
        cw = mw;
     }
   if (ch < mh)
     {
        cy += (ch - mh) / 2;
        ch = mh;
     }
   evas_object_move(sd->hoversel, x + cx, y + cy);
   evas_object_resize(sd->hoversel, cw, ch);
}

static void
_hover_del_job(void *data)
{
   ELM_ENTRY_DATA_GET(data, sd);

   if (sd->hoversel)
     {
        evas_object_del(sd->hoversel);
        sd->hoversel = NULL;
     }
   sd->hov_deljob = NULL;
}

static void
_hover_dismissed_cb(void *data,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
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
   if (sd->hov_deljob) ecore_job_del(sd->hov_deljob);
   sd->hov_deljob = ecore_job_add(_hover_del_job, data);
}

static void
_hover_selected_cb(void *data,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   ELM_ENTRY_DATA_GET(data, sd);

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
     return NULL;

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

   edje_object_part_text_user_insert(sd->entry_edje, "elm.text", str);

   if (str != entry) free(str);
}

static void
_paste_cb(void *data,
          Evas_Object *obj __UNUSED__,
          void *event_info __UNUSED__)
{
   ELM_ENTRY_DATA_GET(data, sd);

   evas_object_smart_callback_call(data, SIG_SELECTION_PASTE, NULL);
   if (sd->sel_notify_handler)
     {
#ifdef HAVE_ELEMENTARY_X
        Elm_Sel_Format formats = ELM_SEL_FORMAT_MARKUP;

        sd->selection_asked = EINA_TRUE;

        if (sd->cnp_mode == ELM_CNP_MODE_PLAINTEXT)
          formats = ELM_SEL_FORMAT_TEXT;
        else if (sd->cnp_mode != ELM_CNP_MODE_NO_IMAGE)
          formats |= ELM_SEL_FORMAT_IMAGE;

        elm_cnp_selection_get
          (data, ELM_SEL_TYPE_CLIPBOARD, formats, NULL, NULL);
#endif
     }
   else
     {
#ifdef HAVE_ELEMENTARY_WAYLAND
        Elm_Sel_Format formats = ELM_SEL_FORMAT_MARKUP;
        wd->selection_asked = EINA_TRUE;
        if (wd->cnp_mode == ELM_CNP_MODE_PLAINTEXT)
           formats = ELM_SEL_FORMAT_TEXT;
        else if (wd->cnp_mode != ELM_CNP_MODE_NO_IMAGE)
           formats |= ELM_SEL_FORMAT_IMAGE;
        elm_cnp_selection_get(data, ELM_SEL_TYPE_CLIPBOARD, formats, NULL, NULL);
#endif
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
   if (seltype == ELM_SEL_TYPE_CLIPBOARD)
     eina_stringshare_replace(&sd->cut_sel, sel);
}

static void
_cut_cb(void *data,
        Evas_Object *obj __UNUSED__,
        void *event_info __UNUSED__)
{
   ELM_ENTRY_DATA_GET(data, sd);

   /* Store it */
   sd->sel_mode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set
       (sd->entry_edje, "elm.text", EINA_FALSE);
   edje_object_signal_emit(sd->entry_edje, "elm,state,select,off", "elm");

   if (!_elm_config->desktop_entry)
     elm_widget_scroll_hold_pop(data);

   _selection_store(ELM_SEL_TYPE_CLIPBOARD, data);
   edje_object_part_text_user_insert(sd->entry_edje, "elm.text", "");
   elm_layout_sizing_eval(data);
}

static void
_copy_cb(void *data,
         Evas_Object *obj __UNUSED__,
         void *event_info __UNUSED__)
{
   ELM_ENTRY_DATA_GET(data, sd);

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
                 Evas_Object *obj __UNUSED__,
                 void *event_info __UNUSED__)
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
                       Evas_Object *obj __UNUSED__,
                       void *event_info __UNUSED__)
{
   Elm_Entry_Context_Menu_Item *it = data;
   Evas_Object *obj2 = it->obj;

   if (it->func) it->func(it->data, obj2, NULL);
}

static void
_menu_call(Evas_Object *obj)
{
   Evas_Object *top;
   const Eina_List *l;
   const Elm_Entry_Context_Menu_Item *it;

   ELM_ENTRY_DATA_GET(obj, sd);

   if ((sd->api) && (sd->api->obj_longpress))
     {
        sd->api->obj_longpress(obj);
     }
   else if (sd->context_menu)
     {
        const char *context_menu_orientation;

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

        evas_object_smart_callback_add
          (sd->hoversel, "dismissed", _hover_dismissed_cb, obj);
        if (sd->have_selection)
          {
             if (!sd->password)
               {
                  if (sd->have_selection)
                    {
                       elm_hoversel_item_add
                         (sd->hoversel, E_("Copy"), NULL, ELM_ICON_NONE,
                         _copy_cb, obj);
                       if (sd->editable)
                         elm_hoversel_item_add
                           (sd->hoversel, E_("Cut"), NULL, ELM_ICON_NONE,
                           _cut_cb, obj);
                    }
                  elm_hoversel_item_add
                    (sd->hoversel, E_("Cancel"), NULL, ELM_ICON_NONE,
                    _hover_cancel_cb, obj);
               }
          }
        else
          {
             if (!sd->sel_mode)
               {
                  if (!_elm_config->desktop_entry)
                    {
                       if (!sd->password)
                         elm_hoversel_item_add
                           (sd->hoversel, E_("Select"), NULL, ELM_ICON_NONE,
                           _hover_selected_cb, obj);
                    }
                  if (elm_selection_selection_has_owner(obj))
                    {
                       if (sd->editable)
                         elm_hoversel_item_add
                           (sd->hoversel, E_("Paste"), NULL, ELM_ICON_NONE,
                           _paste_cb, obj);
                    }
               }
          }

        EINA_LIST_FOREACH (sd->items, l, it)
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

static Eina_Bool
_long_press_cb(void *data)
{
   ELM_ENTRY_DATA_GET(data, sd);

   if (!_elm_config->desktop_entry)
     _menu_call(data);

   sd->longpress_timer = NULL;
   evas_object_smart_callback_call(data, SIG_LONGPRESSED, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down_cb(void *data,
               Evas *evas __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;

   ELM_ENTRY_DATA_GET(data, sd);

   if (sd->disabled) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   sd->downx = ev->canvas.x;
   sd->downy = ev->canvas.y;
   if (ev->button == 1)
     {
        if (sd->longpress_timer) ecore_timer_del(sd->longpress_timer);
        sd->longpress_timer = ecore_timer_add
            (_elm_config->longpress_timeout, _long_press_cb, data);
     }
   else if (ev->button == 3)
     {
        if (_elm_config->desktop_entry)
          _menu_call(data);
     }
}

static void
_mouse_up_cb(void *data,
             Evas *evas __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;

   ELM_ENTRY_DATA_GET(data, sd);

   if (sd->disabled) return;
   if (ev->button == 1)
     {
        if (sd->longpress_timer)
          {
             ecore_timer_del(sd->longpress_timer);
             sd->longpress_timer = NULL;
          }
     }
   else if ((ev->button == 3) && (!_elm_config->desktop_entry))
     {
        sd->use_down = 1;
        _menu_call(data);
     }
}

static void
_mouse_move_cb(void *data,
               Evas *evas __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;

   ELM_ENTRY_DATA_GET(data, sd);

   if (sd->disabled) return;
   if (!sd->sel_mode)
     {
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
          {
             if (sd->longpress_timer)
               {
                  ecore_timer_del(sd->longpress_timer);
                  sd->longpress_timer = NULL;
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
                  ecore_timer_del(sd->longpress_timer);
                  sd->longpress_timer = NULL;
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
             ecore_timer_del(sd->longpress_timer);
             sd->longpress_timer = NULL;
          }
     }
}

static void
_entry_changed_handle(void *data,
                      const char *event)
{
   Evas_Coord minh;
   const char *text;

   ELM_ENTRY_DATA_GET(data, sd);

   evas_event_freeze(evas_object_evas_get(data));
   sd->changed = EINA_TRUE;
   /* Reset the size hints which are no more relevant. Keep the
    * height, this is a hack, but doesn't really matter cause we'll
    * re-eval in a moment. */
   evas_object_size_hint_min_get(data, NULL, &minh);
   evas_object_size_hint_min_set(data, -1, minh);

   elm_layout_sizing_eval(data);
   if (sd->text) eina_stringshare_del(sd->text);
   sd->text = NULL;
   if (sd->delay_write)
     {
        ecore_timer_del(sd->delay_write);
        sd->delay_write = NULL;
     }
   evas_event_thaw(evas_object_evas_get(data));
   evas_event_thaw_eval(evas_object_evas_get(data));
   if ((sd->auto_save) && (sd->file))
     sd->delay_write = ecore_timer_add(2.0, _delay_write, data);
   /* callback - this could call callbacks that delete the
    * entry... thus... any access to sd after this could be
    * invalid */
   evas_object_smart_callback_call(data, event, NULL);
   _return_key_enabled_check(data);
   text = edje_object_part_text_get(sd->entry_edje, "elm.text");
   if (text)
     {
        if (text[0])
          _elm_entry_guide_update(data, EINA_TRUE);
        else
          _elm_entry_guide_update(data, EINA_FALSE);
     }
}

static void
_entry_changed_signal_cb(void *data,
                         Evas_Object *obj __UNUSED__,
                         const char *emission __UNUSED__,
                         const char *source __UNUSED__)
{
   _entry_changed_handle(data, SIG_CHANGED);
}

static void
_entry_changed_user_signal_cb(void *data,
                              Evas_Object *obj __UNUSED__,
                              const char *emission __UNUSED__,
                              const char *source __UNUSED__)
{
   Elm_Entry_Change_Info info;
   Edje_Entry_Change_Info *edje_info = (Edje_Entry_Change_Info *)
     edje_object_signal_callback_extra_data_get();

   if (edje_info)
     {
        memcpy(&info, edje_info, sizeof(info));
        evas_object_smart_callback_call(data, SIG_CHANGED_USER, &info);
     }
   else
     {
        evas_object_smart_callback_call(data, SIG_CHANGED_USER, NULL);
     }
}

static void
_entry_preedit_changed_signal_cb(void *data,
                                 Evas_Object *obj __UNUSED__,
                                 const char *emission __UNUSED__,
                                 const char *source __UNUSED__)
{
   _entry_changed_handle(data, SIG_PREEDIT_CHANGED);
}

static void
_entry_undo_request_signal_cb(void *data,
                              Evas_Object *obj __UNUSED__,
                              const char *emission __UNUSED__,
                              const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_UNDO_REQUEST, NULL);
}

static void
_entry_redo_request_signal_cb(void *data,
                              Evas_Object *obj __UNUSED__,
                              const char *emission __UNUSED__,
                              const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_REDO_REQUEST, NULL);
}

static void
_entry_selection_start_signal_cb(void *data,
                                 Evas_Object *obj __UNUSED__,
                                 const char *emission __UNUSED__,
                                 const char *source __UNUSED__)
{
   const Eina_List *l;
   Evas_Object *entry;

   ELM_ENTRY_DATA_GET(data, sd);

   if (!elm_object_focus_get(data)) elm_object_focus_set(data, EINA_TRUE);
   EINA_LIST_FOREACH (entries, l, entry)
     {
        if (entry != data) elm_entry_select_none(entry);
     }
   sd->have_selection = EINA_TRUE;
   evas_object_smart_callback_call(data, SIG_SELECTION_START, NULL);
#ifdef HAVE_ELEMENTARY_X
   if (sd->sel_notify_handler)
     {
        const char *txt = elm_entry_selection_get(data);
        Evas_Object *top;

        top = elm_widget_top_get(data);
        if (txt && top && (elm_win_xwindow_get(top)))
          elm_cnp_selection_set(data, ELM_SEL_TYPE_PRIMARY,
                                ELM_SEL_FORMAT_MARKUP, txt, strlen(txt));
     }
#endif
}

static void
_entry_selection_all_signal_cb(void *data,
                               Evas_Object *obj __UNUSED__,
                               const char *emission __UNUSED__,
                               const char *source __UNUSED__)
{
   elm_entry_select_all(data);
}

static void
_entry_selection_none_signal_cb(void *data,
                                Evas_Object *obj __UNUSED__,
                                const char *emission __UNUSED__,
                                const char *source __UNUSED__)
{
   elm_entry_select_none(data);
}

static void
_entry_selection_changed_signal_cb(void *data,
                                   Evas_Object *obj __UNUSED__,
                                   const char *emission __UNUSED__,
                                   const char *source __UNUSED__)
{
   ELM_ENTRY_DATA_GET(data, sd);

   sd->have_selection = EINA_TRUE;
   evas_object_smart_callback_call(data, SIG_SELECTION_CHANGED, NULL);
   _selection_store(ELM_SEL_TYPE_PRIMARY, data);
}

static void
_entry_selection_cleared_signal_cb(void *data,
                                   Evas_Object *obj __UNUSED__,
                                   const char *emission __UNUSED__,
                                   const char *source __UNUSED__)
{
   ELM_ENTRY_DATA_GET(data, sd);

   if (!sd->have_selection) return;

   sd->have_selection = EINA_FALSE;
   evas_object_smart_callback_call(data, SIG_SELECTION_CLEARED, NULL);
   if (sd->sel_notify_handler)
     {
        if (sd->cut_sel)
          {
#ifdef HAVE_ELEMENTARY_X
             Evas_Object *top;

             top = elm_widget_top_get(data);
             if ((top) && (elm_win_xwindow_get(top)))
               elm_cnp_selection_set
                 (data, ELM_SEL_TYPE_PRIMARY, ELM_SEL_FORMAT_MARKUP,
                 sd->cut_sel, strlen(sd->cut_sel));
#endif
             eina_stringshare_del(sd->cut_sel);
             sd->cut_sel = NULL;
          }
        else
          {
#ifdef HAVE_ELEMENTARY_X
             Evas_Object *top;

             top = elm_widget_top_get(data);
             if ((top) && (elm_win_xwindow_get(top)))
               elm_object_cnp_selection_clear(data, ELM_SEL_TYPE_PRIMARY);
#endif
          }
     }
}

static void
_entry_paste_request_signal_cb(void *data,
                               Evas_Object *obj __UNUSED__,
                               const char *emission,
                               const char *source __UNUSED__)
{
   ELM_ENTRY_DATA_GET(data, sd);

#ifdef HAVE_ELEMENTARY_X
   Elm_Sel_Type type = (emission[sizeof("ntry,paste,request,")] == '1') ?
     ELM_SEL_TYPE_PRIMARY : ELM_SEL_TYPE_CLIPBOARD;
#endif

   if (!sd->editable) return;
   evas_object_smart_callback_call(data, SIG_SELECTION_PASTE, NULL);
   if (sd->sel_notify_handler)
     {
#ifdef HAVE_ELEMENTARY_X
        Evas_Object *top;

        top = elm_widget_top_get(data);
        if ((top) && (elm_win_xwindow_get(top)))
          {
             Elm_Sel_Format formats = ELM_SEL_FORMAT_MARKUP;

             sd->selection_asked = EINA_TRUE;

             if (sd->cnp_mode == ELM_CNP_MODE_PLAINTEXT)
               formats = ELM_SEL_FORMAT_TEXT;
             else if (sd->cnp_mode != ELM_CNP_MODE_NO_IMAGE)
               formats |= ELM_SEL_FORMAT_IMAGE;

             elm_cnp_selection_get(data, type, formats, NULL, NULL);
          }
#endif
     }
}

static void
_entry_copy_notify_signal_cb(void *data,
                             Evas_Object *obj __UNUSED__,
                             const char *emission __UNUSED__,
                             const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_COPY, NULL);
   _copy_cb(data, NULL, NULL);
}

static void
_entry_cut_notify_signal_cb(void *data,
                            Evas_Object *obj __UNUSED__,
                            const char *emission __UNUSED__,
                            const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_CUT, NULL);
   _cut_cb(data, NULL, NULL);
}

static void
_entry_cursor_changed_signal_cb(void *data,
                                Evas_Object *obj __UNUSED__,
                                const char *emission __UNUSED__,
                                const char *source __UNUSED__)
{
   ELM_ENTRY_DATA_GET(data, sd);
   sd->cursor_pos = edje_object_part_text_cursor_pos_get
       (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
   sd->cur_changed = EINA_TRUE;
   _cursor_geometry_recalc(data);
}

static void
_entry_cursor_changed_manual_signal_cb(void *data,
                                       Evas_Object *obj __UNUSED__,
                                       const char *emission __UNUSED__,
                                       const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CURSOR_CHANGED_MANUAL, NULL);
}

static void
_signal_anchor_geoms_do_things_with_lol(Elm_Entry_Smart_Data *sd,
                                        Elm_Entry_Anchor_Info *ei)
{
   Evas_Textblock_Rectangle *r;
   const Eina_List *geoms, *l;
   Evas_Coord px, py, x, y;

   geoms = edje_object_part_text_anchor_geometry_get
       (sd->entry_edje, "elm.text", ei->name);

   if (!geoms) return;

   evas_object_geometry_get(sd->entry_edje, &x, &y, NULL, NULL);
   evas_pointer_canvas_xy_get
     (evas_object_evas_get(sd->entry_edje), &px, &py);

   EINA_LIST_FOREACH (geoms, l, r)
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
                             Evas_Object *obj __UNUSED__,
                             const char *emission __UNUSED__,
                             const char *source __UNUSED__)
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
     evas_object_smart_callback_call(data, SIG_ANCHOR_DOWN, &ei);
}

static void
_entry_anchor_up_signal_cb(void *data,
                           Evas_Object *obj __UNUSED__,
                           const char *emission __UNUSED__,
                           const char *source __UNUSED__)
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
     evas_object_smart_callback_call(data, SIG_ANCHOR_UP, &ei);
}

static void
_anchor_hover_del_cb(void *data,
                     Evas *e __UNUSED__,
                     Evas_Object *obj __UNUSED__,
                     void *event_info __UNUSED__)
{
   ELM_ENTRY_DATA_GET(data, sd);

   if (sd->anchor_hover.pop) evas_object_del(sd->anchor_hover.pop);
   sd->anchor_hover.pop = NULL;
   evas_object_event_callback_del_full
     (sd->anchor_hover.hover, EVAS_CALLBACK_DEL, _anchor_hover_del_cb, obj);
}

static void
_anchor_hover_clicked_cb(void *data,
                         Evas_Object *obj __UNUSED__,
                         void *event_info __UNUSED__)
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

   ei.anchor_info = info;

   sd->anchor_hover.pop = elm_icon_add(obj);
   evas_object_move(sd->anchor_hover.pop, info->x, info->y);
   evas_object_resize(sd->anchor_hover.pop, info->w, info->h);

   sd->anchor_hover.hover = elm_hover_add(obj);
   evas_object_event_callback_add
     (sd->anchor_hover.hover, EVAS_CALLBACK_DEL, _anchor_hover_del_cb, obj);
   elm_widget_mirrored_set
     (sd->anchor_hover.hover, elm_widget_mirrored_get(obj));
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
   if (elm_widget_mirrored_get(sd->anchor_hover.hover))
     {
        Eina_Bool tmp = ei.hover_left;

        ei.hover_left = ei.hover_right;
        ei.hover_right = tmp;
     }

   evas_object_smart_callback_call(obj, SIG_ANCHOR_HOVER_OPENED, &ei);
   evas_object_smart_callback_add
     (sd->anchor_hover.hover, "clicked", _anchor_hover_clicked_cb, obj);

   /* FIXME: Should just check if there's any callback registered to
    * the smart events instead.  This is used to determine if anyone
    * cares about the hover or not. */
   if (!elm_layout_content_get(sd->anchor_hover.hover, "middle") &&
       !elm_layout_content_get(sd->anchor_hover.hover, "left") &&
       !elm_layout_content_get(sd->anchor_hover.hover, "right") &&
       !elm_layout_content_get(sd->anchor_hover.hover, "top") &&
       !elm_layout_content_get(sd->anchor_hover.hover, "bottom"))
     {
        evas_object_del(sd->anchor_hover.hover);
        sd->anchor_hover.hover = NULL;
     }
   else
     evas_object_show(sd->anchor_hover.hover);
}

static void
_entry_anchor_clicked_signal_cb(void *data,
                                Evas_Object *obj __UNUSED__,
                                const char *emission,
                                const char *source __UNUSED__)
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
        evas_object_smart_callback_call(data, SIG_ANCHOR_CLICKED, &ei);

        _entry_hover_anchor_clicked_do(data, &ei);
     }
}

static void
_entry_anchor_move_signal_cb(void *data __UNUSED__,
                             Evas_Object *obj __UNUSED__,
                             const char *emission __UNUSED__,
                             const char *source __UNUSED__)
{
}

static void
_entry_anchor_in_signal_cb(void *data,
                           Evas_Object *obj __UNUSED__,
                           const char *emission __UNUSED__,
                           const char *source __UNUSED__)
{
   Elm_Entry_Anchor_Info ei;

   ELM_ENTRY_DATA_GET(data, sd);

   ei.name = emission + sizeof("nchor,mouse,in,");
   ei.button = 0;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with_lol(sd, &ei);

   if (!sd->disabled)
     evas_object_smart_callback_call(data, SIG_ANCHOR_IN, &ei);
}

static void
_entry_anchor_out_signal_cb(void *data,
                            Evas_Object *obj __UNUSED__,
                            const char *emission __UNUSED__,
                            const char *source __UNUSED__)
{
   Elm_Entry_Anchor_Info ei;

   ELM_ENTRY_DATA_GET(data, sd);

   ei.name = emission + sizeof("nchor,mouse,out,");
   ei.button = 0;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with_lol(sd, &ei);

   if (!sd->disabled)
     evas_object_smart_callback_call(data, SIG_ANCHOR_OUT, &ei);
}

static void
_entry_key_enter_signal_cb(void *data,
                           Evas_Object *obj __UNUSED__,
                           const char *emission __UNUSED__,
                           const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_ACTIVATED, NULL);
}

static void
_entry_key_escape_signal_cb(void *data,
                            Evas_Object *obj __UNUSED__,
                            const char *emission __UNUSED__,
                            const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_ABORTED, NULL);
}

static void
_entry_mouse_down_signal_cb(void *data,
                            Evas_Object *obj __UNUSED__,
                            const char *emission __UNUSED__,
                            const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_PRESS, NULL);
}

static void
_entry_mouse_clicked_signal_cb(void *data,
                               Evas_Object *obj __UNUSED__,
                               const char *emission __UNUSED__,
                               const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_entry_mouse_double_signal_cb(void *data,
                              Evas_Object *obj __UNUSED__,
                              const char *emission __UNUSED__,
                              const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CLICKED_DOUBLE, NULL);
}

static void
_entry_mouse_triple_signal_cb(void *data,
                              Evas_Object *obj __UNUSED__,
                              const char *emission __UNUSED__,
                              const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CLICKED_TRIPLE, NULL);
}

#ifdef HAVE_ELEMENTARY_X
static Eina_Bool
_event_selection_notify(void *data,
                        int type __UNUSED__,
                        void *event)
{
   Ecore_X_Event_Selection_Notify *ev = event;

   ELM_ENTRY_DATA_GET(data, sd);

   if ((!sd->selection_asked) && (!sd->drag_selection_asked))
     return ECORE_CALLBACK_PASS_ON;

   if ((ev->selection == ECORE_X_SELECTION_CLIPBOARD) ||
       (ev->selection == ECORE_X_SELECTION_PRIMARY))
     {
        Ecore_X_Selection_Data_Text *text_data;

        text_data = ev->data;
        if (text_data->data.content == ECORE_X_SELECTION_CONTENT_TEXT)
          {
             if (text_data->text)
               {
                  char *txt = _elm_util_text_to_mkup(text_data->text);

                  if (txt)
                    {
                       elm_entry_entry_insert(data, txt);
                       free(txt);
                    }
               }
          }
        sd->selection_asked = EINA_FALSE;
     }
   else if (ev->selection == ECORE_X_SELECTION_XDND)
     {
        Ecore_X_Selection_Data_Text *text_data;

        text_data = ev->data;
        if (text_data->data.content == ECORE_X_SELECTION_CONTENT_TEXT)
          {
             if (text_data->text)
               {
                  char *txt = _elm_util_text_to_mkup(text_data->text);

                  if (txt)
                    {
                       /* Massive FIXME: this should be at the drag point */
                       elm_entry_entry_insert(data, txt);
                       free(txt);
                    }
               }
          }
        sd->drag_selection_asked = EINA_FALSE;

        ecore_x_dnd_send_finished();
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_event_selection_clear(void *data __UNUSED__,
                       int type __UNUSED__,
                       void *event __UNUSED__)
{
   Ecore_X_Event_Selection_Clear *ev = event;

   ELM_ENTRY_DATA_GET(data, sd);

   if (!sd->have_selection) return ECORE_CALLBACK_PASS_ON;
   if ((ev->selection == ECORE_X_SELECTION_CLIPBOARD) ||
       (ev->selection == ECORE_X_SELECTION_PRIMARY))
     {
        elm_entry_select_none(data);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_drag_drop_cb(void *data __UNUSED__,
              Evas_Object *obj,
              Elm_Selection_Data *drop)
{
   Eina_Bool rv;

   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_cursor_copy
     (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN, /*->*/ EDJE_CURSOR_USER);
   rv = edje_object_part_text_cursor_coord_set
       (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN, drop->x, drop->y);

   if (!rv) WRN("Warning: Failed to position cursor: paste anyway");

   elm_entry_entry_insert(obj, drop->data);
   edje_object_part_text_cursor_copy
     (sd->entry_edje, "elm.text", EDJE_CURSOR_USER, /*->*/ EDJE_CURSOR_MAIN);

   return EINA_TRUE;
}

#endif

static Evas_Object *
_item_get(void *data,
          Evas_Object *edje __UNUSED__,
          const char *part __UNUSED__,
          const char *item)
{
   Eina_List *l;
   Evas_Object *o;
   Elm_Entry_Item_Provider *ip;

   ELM_ENTRY_DATA_GET(data, sd);

   EINA_LIST_FOREACH (sd->item_providers, l, ip)
     {
        o = ip->func(ip->data, data, item);
        if (o) return o;
     }
   if (!strncmp(item, "file://", 7))
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
               (data, o, "entry/emoticon", "wtf",
               elm_widget_style_get(data));
          }
        return o;
     }

   o = edje_object_add(evas_object_evas_get(data));
   if (!elm_widget_theme_object_set
         (data, o, "entry", item, elm_widget_style_get(data)))
     elm_widget_theme_object_set
       (data, o, "entry/emoticon", "wtf", elm_widget_style_get(data));
   return o;
}

static void
_text_filter_cb(void *data,
                Evas_Object *edje __UNUSED__,
                const char *part __UNUSED__,
                Edje_Text_Filter_Type type,
                char **text)
{
   Eina_List *l;
   Elm_Entry_Markup_Filter *tf;

   ELM_ENTRY_DATA_GET(data, sd);

   if (type == EDJE_TEXT_FILTER_FORMAT)
     return;

   EINA_LIST_FOREACH (sd->text_filters, l, tf)
     {
        tf->func(tf->data, data, text);
        if (!*text)
          break;
     }
}

static void
_markup_filter_cb(void *data,
                  Evas_Object *edje __UNUSED__,
                  const char *part __UNUSED__,
                  char **text)
{
   Eina_List *l;
   Elm_Entry_Markup_Filter *tf;

   ELM_ENTRY_DATA_GET(data, sd);

   EINA_LIST_FOREACH (sd->markup_filters, l, tf)
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
   if (sd->text) eina_stringshare_del(sd->text);
   sd->text = NULL;
   sd->changed = EINA_TRUE;

   start = sd->append_text_position;
   if ((start + _CHUNK_SIZE) < sd->append_text_len)
     {
        int pos = start;
        int tag_start, esc_start;

        tag_start = esc_start = -1;
        /* Find proper markup cut place */
        while (pos - start < _CHUNK_SIZE)
          {
             int prev_pos = pos;
             Eina_Unicode tmp =
               eina_unicode_utf8_get_next(sd->append_text_left, &pos);

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
        free(sd->append_text_left);
        sd->append_text_left = NULL;
        sd->append_text_idler = NULL;
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
                  free(utfstr);
                  utfstr = NULL;
               }
             free(markup);
             markup = NULL;
          }
        if (can_add < unit_size)
          {
             if (!i)
               {
                  evas_object_smart_callback_call(obj, SIG_MAX_LENGHT, NULL);
                  free(*text);
                  *text = NULL;
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

   evas_object_smart_callback_call(obj, SIG_MAX_LENGHT, NULL);
}

static void
_elm_entry_smart_signal(Evas_Object *obj,
                        const char *emission,
                        const char *source)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   /* always pass to both edje objs */
   edje_object_signal_emit(sd->entry_edje, emission, source);

   if (sd->scr_edje)
     edje_object_signal_emit(sd->scr_edje, emission, source);
}

static void
_elm_entry_smart_callback_add(Evas_Object *obj,
                              const char *emission,
                              const char *source,
                              Edje_Signal_Cb func_cb,
                              void *data)
{
   Evas_Object *ro;

   ELM_ENTRY_DATA_GET(obj, sd);

   ro = ELM_WIDGET_DATA(sd)->resize_obj;

   ELM_WIDGET_DATA(sd)->resize_obj = sd->entry_edje;

   ELM_LAYOUT_CLASS(_elm_entry_parent_sc)->callback_add
     (obj, emission, source, func_cb, data);

   if (sd->scr_edje)
     {
        ELM_WIDGET_DATA(sd)->resize_obj = sd->scr_edje;

        ELM_LAYOUT_CLASS(_elm_entry_parent_sc)->callback_add
          (obj, emission, source, func_cb, data);
     }

   ELM_WIDGET_DATA(sd)->resize_obj = ro;
}

static void *
_elm_entry_smart_callback_del(Evas_Object *obj,
                              const char *emission,
                              const char *source,
                              Edje_Signal_Cb func_cb)
{
   Evas_Object *ro;
   void *data;

   ELM_ENTRY_DATA_GET(obj, sd);

   ro = ELM_WIDGET_DATA(sd)->resize_obj;

   ELM_WIDGET_DATA(sd)->resize_obj = sd->entry_edje;

   data = ELM_LAYOUT_CLASS(_elm_entry_parent_sc)->callback_del
       (obj, emission, source, func_cb);

   if (sd->scr_edje)
     {
        ELM_WIDGET_DATA(sd)->resize_obj = sd->scr_edje;

        ELM_LAYOUT_CLASS(_elm_entry_parent_sc)->callback_del
          (obj, emission, source, func_cb);
     }

   ELM_WIDGET_DATA(sd)->resize_obj = ro;

   return data;
}

static Eina_Bool
_elm_entry_smart_content_set(Evas_Object *obj,
                             const char *part,
                             Evas_Object *content)
{
   if (!ELM_CONTAINER_CLASS(_elm_entry_parent_sc)->content_set
         (obj, part, content))
     return EINA_FALSE;

   /* too bad entry does not follow the pattern
    * "elm,state,{icon,end},visible", we have to repeat ourselves */
   if (!part || !strcmp(part, "icon") || !strcmp(part, "elm.swallow.icon"))
     elm_entry_icon_visible_set(obj, EINA_TRUE);

   if (!part || !strcmp(part, "end") || !strcmp(part, "elm.swallow.end"))
     elm_entry_end_visible_set(obj, EINA_TRUE);

   return EINA_TRUE;
}

static Evas_Object *
_elm_entry_smart_content_unset(Evas_Object *obj,
                               const char *part)
{
   Evas_Object *ret;

   ret = ELM_CONTAINER_CLASS(_elm_entry_parent_sc)->content_unset(obj, part);
   if (!ret) return NULL;

   /* too bad entry does not follow the pattern
    * "elm,state,{icon,end},hidden", we have to repeat ourselves */
   if (!part || !strcmp(part, "icon") || !strcmp(part, "elm.swallow.icon"))
     elm_entry_icon_visible_set(obj, EINA_FALSE);

   if (!part || !strcmp(part, "end") || !strcmp(part, "elm.swallow.end"))
     elm_entry_end_visible_set(obj, EINA_FALSE);

   return ret;
}

static Eina_Bool
_elm_entry_smart_text_set(Evas_Object *obj,
                          const char *item,
                          const char *entry)
{
   int len = 0;

   ELM_ENTRY_DATA_GET(obj, sd);

   if (!entry) entry = "";
   if (item)
     {
        if (!strcmp(item, "guide"))
          edje_object_part_text_set(sd->entry_edje, "elm.guide", entry);
        else
          edje_object_part_text_set(sd->entry_edje, item, entry);

        return EINA_TRUE;
     }

   evas_event_freeze(evas_object_evas_get(obj));
   if (sd->text) eina_stringshare_del(sd->text);
   sd->text = NULL;
   sd->changed = EINA_TRUE;

   /* Clear currently pending job if there is one */
   if (sd->append_text_idler)
     {
        ecore_idler_del(sd->append_text_idler);
        free(sd->append_text_left);
        sd->append_text_left = NULL;
        sd->append_text_idler = NULL;
     }

   len = strlen(entry);
   /* Split to ~_CHUNK_SIZE chunks */
   if (len > _CHUNK_SIZE)
     {
        sd->append_text_left = (char *)malloc(len + 1);
     }

   /* If we decided to use the idler */
   if (sd->append_text_left)
     {
        /* Need to clear the entry first */
        edje_object_part_text_set(sd->entry_edje, "elm.text", "");
        memcpy(sd->append_text_left, entry, len + 1);
        sd->append_text_position = 0;
        sd->append_text_len = len;
        sd->append_text_idler = ecore_idler_add(_text_append_idler, obj);
     }
   else
     {
        edje_object_part_text_set(sd->entry_edje, "elm.text", entry);
     }

   if ((entry) && (entry[0]))
     _elm_entry_guide_update(obj, EINA_TRUE);
   else
     _elm_entry_guide_update(obj, EINA_FALSE);

   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   return EINA_TRUE;
}

static const char *
_elm_entry_smart_text_get(const Evas_Object *obj,
                          const char *item)
{
   const char *text;

   ELM_ENTRY_DATA_GET(obj, sd);

   if (item)
     {
        if (!strcmp(item, "default")) goto proceed;
        else if (!strcmp(item, "guide"))
          return edje_object_part_text_get(sd->entry_edje, "elm.guide");
        else
          return edje_object_part_text_get(sd->entry_edje, item);
     }

proceed:

   text = edje_object_part_text_get(sd->entry_edje, "elm.text");
   if (!text)
     {
        ERR("text=NULL for edje %p, part 'elm.text'", sd->entry_edje);

        return NULL;
     }

   if (sd->append_text_len > 0)
     {
        char *tmpbuf;
        size_t tlen;

        tlen = strlen(text);
        tmpbuf = malloc(sd->append_text_len + 1);
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
        tmpbuf[sd->append_text_len] = '\0';
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
_access_info_cb(void *data __UNUSED__,
                Evas_Object *obj,
                Elm_Widget_Item *item __UNUSED__)
{
   const char *txt;

   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->password) return NULL;

   txt = elm_widget_access_info_get(obj);

   if (!txt) txt = elm_entry_entry_get(obj);
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data __UNUSED__,
                 Evas_Object *obj,
                 Elm_Widget_Item *item __UNUSED__)
{
   Eina_Strbuf *buf;
   char *txt;

   ELM_ENTRY_DATA_GET(obj, sd);

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

   txt = strdup(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   if (txt) return txt;

   return NULL;
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
_resize_cb(void *data,
           Evas *e __UNUSED__,
           Evas_Object *obj __UNUSED__,
           void *event_info __UNUSED__)
{
   ELM_ENTRY_DATA_GET(data, sd);

   if (sd->line_wrap)
     {
        elm_layout_sizing_eval(data);
     }
   else if (sd->scroll)
     {
        Evas_Coord vw = 0, vh = 0;

        sd->s_iface->content_viewport_size_get(data, &vw, &vh);
        if (vw < sd->ent_mw) vw = sd->ent_mw;
        if (vh < sd->ent_mh) vh = sd->ent_mh;
        evas_object_resize(sd->entry_edje, vw, vh);
     }

   if (sd->hoversel) _hoversel_position(data);
}

static void
_elm_entry_smart_add(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_X
   Evas_Object *top;
#endif

   EVAS_SMART_DATA_ALLOC(obj, Elm_Entry_Smart_Data);

   ELM_WIDGET_CLASS(_elm_entry_parent_sc)->base.add(obj);

   priv->entry_edje = ELM_WIDGET_DATA(priv)->resize_obj;

   priv->cnp_mode = ELM_CNP_MODE_MARKUP;
   priv->line_wrap = ELM_WRAP_WORD;
   priv->context_menu = EINA_TRUE;
   priv->disabled = EINA_FALSE;
   priv->auto_save = EINA_TRUE;
   priv->editable = EINA_TRUE;
   priv->scroll = EINA_FALSE;

   priv->input_panel_imdata = NULL;

   elm_layout_theme_set(obj, "entry", "base", elm_widget_style_get(obj));

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_data_set(priv->hit_rect, "_elm_leaveme", obj);
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   priv->s_iface = evas_object_smart_interface_get
       (obj, ELM_SCROLLABLE_IFACE_NAME);

   priv->s_iface->objects_set(obj, priv->entry_edje, priv->hit_rect);

   edje_object_item_provider_set(priv->entry_edje, _item_get, obj);

   edje_object_text_insert_filter_callback_add
     (priv->entry_edje, "elm.text", _text_filter_cb, obj);
   edje_object_text_markup_filter_callback_add
     (priv->entry_edje, "elm.text", _markup_filter_cb, obj);

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
     (ELM_WIDGET_DATA(priv)->resize_obj, obj, ELM_CURSOR_XTERM);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   if (_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set
       (priv->entry_edje, "elm.text", EINA_TRUE);

   elm_layout_sizing_eval(obj);

   elm_entry_input_panel_layout_set(obj, ELM_INPUT_PANEL_LAYOUT_NORMAL);
   elm_entry_input_panel_enabled_set(obj, EINA_TRUE);
   elm_entry_prediction_allow_set(obj, EINA_TRUE);

   priv->autocapital_type = edje_object_part_text_autocapital_type_get
       (priv->entry_edje, "elm.text");

#ifdef HAVE_ELEMENTARY_X
   top = elm_widget_top_get(obj);
   if ((top) && (elm_win_xwindow_get(top)))
     {
        priv->sel_notify_handler =
          ecore_event_handler_add
            (ECORE_X_EVENT_SELECTION_NOTIFY, _event_selection_notify, obj);
        priv->sel_clear_handler =
          ecore_event_handler_add
            (ECORE_X_EVENT_SELECTION_CLEAR, _event_selection_clear, obj);
     }

   elm_drop_target_add
     (obj, ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE,
     _drag_drop_cb, NULL);
#endif

   entries = eina_list_prepend(entries, obj);

   // module - find module for entry
   priv->api = _module_find(obj);
   // if found - hook in
   if ((priv->api) && (priv->api->obj_hook)) priv->api->obj_hook(obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   // access
   _elm_access_object_register(obj, priv->entry_edje);
   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_TYPE, E_("Entry"));
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_STATE, _access_state_cb, NULL);
}

static void
_elm_entry_smart_del(Evas_Object *obj)
{
   Elm_Entry_Context_Menu_Item *it;
   Elm_Entry_Item_Provider *ip;
   Elm_Entry_Markup_Filter *tf;

   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->delay_write)
     {
        ecore_timer_del(sd->delay_write);
        sd->delay_write = NULL;
        if (sd->auto_save) _save_do(obj);
     }

   elm_entry_anchor_hover_end(obj);
   elm_entry_anchor_hover_parent_set(obj, NULL);

   evas_event_freeze(evas_object_evas_get(obj));

   if (sd->file) eina_stringshare_del(sd->file);

   if (sd->hov_deljob) ecore_job_del(sd->hov_deljob);
   if ((sd->api) && (sd->api->obj_unhook))
     sd->api->obj_unhook(obj);  // module - unhook

   entries = eina_list_remove(entries, obj);
#ifdef HAVE_ELEMENTARY_X
   if (sd->sel_notify_handler)
     ecore_event_handler_del(sd->sel_notify_handler);
   if (sd->sel_clear_handler)
     ecore_event_handler_del(sd->sel_clear_handler);
#endif
   if (sd->cut_sel) eina_stringshare_del(sd->cut_sel);
   if (sd->text) eina_stringshare_del(sd->text);
   if (sd->deferred_recalc_job)
     ecore_job_del(sd->deferred_recalc_job);
   if (sd->append_text_idler)
     {
        ecore_idler_del(sd->append_text_idler);
        free(sd->append_text_left);
        sd->append_text_left = NULL;
        sd->append_text_idler = NULL;
     }
   if (sd->longpress_timer) ecore_timer_del(sd->longpress_timer);
   EINA_LIST_FREE (sd->items, it)
     {
        eina_stringshare_del(it->label);
        eina_stringshare_del(it->icon_file);
        eina_stringshare_del(it->icon_group);
        free(it);
     }
   EINA_LIST_FREE (sd->item_providers, ip)
     {
        free(ip);
     }
   EINA_LIST_FREE (sd->text_filters, tf)
     {
        _filter_free(tf);
     }
   EINA_LIST_FREE (sd->markup_filters, tf)
     {
        _filter_free(tf);
     }
   if (sd->delay_write) ecore_timer_del(sd->delay_write);
   if (sd->input_panel_imdata) free(sd->input_panel_imdata);

   if (sd->anchor_hover.hover_style)
     eina_stringshare_del(sd->anchor_hover.hover_style);

   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   ELM_WIDGET_CLASS(_elm_entry_parent_sc)->base.del(obj);
}

static void
_elm_entry_smart_move(Evas_Object *obj,
                      Evas_Coord x,
                      Evas_Coord y)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_entry_parent_sc)->base.move(obj, x, y);

   evas_object_move(sd->hit_rect, x, y);

   if (sd->hoversel) _hoversel_position(obj);
}

static void
_elm_entry_smart_resize(Evas_Object *obj,
                        Evas_Coord w,
                        Evas_Coord h)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_entry_parent_sc)->base.resize(obj, w, h);

   evas_object_resize(sd->hit_rect, w, h);
}

static void
_elm_entry_smart_member_add(Evas_Object *obj,
                            Evas_Object *member)
{
   ELM_ENTRY_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_entry_parent_sc)->base.member_add(obj, member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

static void
_elm_entry_smart_set_user(Elm_Entry_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_entry_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_entry_smart_del;
   ELM_WIDGET_CLASS(sc)->base.move = _elm_entry_smart_move;
   ELM_WIDGET_CLASS(sc)->base.resize = _elm_entry_smart_resize;
   ELM_WIDGET_CLASS(sc)->base.member_add = _elm_entry_smart_member_add;

   ELM_WIDGET_CLASS(sc)->on_focus_region = _elm_entry_smart_on_focus_region;
   ELM_WIDGET_CLASS(sc)->sub_object_del = _elm_entry_smart_sub_object_del;
   ELM_WIDGET_CLASS(sc)->on_focus = _elm_entry_smart_on_focus;
   ELM_WIDGET_CLASS(sc)->theme = _elm_entry_smart_theme;
   ELM_WIDGET_CLASS(sc)->disable = _elm_entry_smart_disable;
   ELM_WIDGET_CLASS(sc)->translate = _elm_entry_smart_translate;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_entry_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_unset = _elm_entry_smart_content_unset;

   ELM_LAYOUT_CLASS(sc)->signal = _elm_entry_smart_signal;
   ELM_LAYOUT_CLASS(sc)->callback_add = _elm_entry_smart_callback_add;
   ELM_LAYOUT_CLASS(sc)->callback_del = _elm_entry_smart_callback_del;
   ELM_LAYOUT_CLASS(sc)->text_set = _elm_entry_smart_text_set;
   ELM_LAYOUT_CLASS(sc)->text_get = _elm_entry_smart_text_get;
   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_entry_smart_sizing_eval;
   ELM_LAYOUT_CLASS(sc)->content_aliases = _content_aliases;
}

EAPI const Elm_Entry_Smart_Class *
elm_entry_smart_class_get(void)
{
   static Elm_Entry_Smart_Class _sc =
     ELM_ENTRY_SMART_CLASS_INIT_NAME_VERSION(ELM_ENTRY_SMART_NAME);
   static const Elm_Entry_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_entry_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_entry_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_entry_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_entry_text_style_user_push(Evas_Object *obj,
                               const char *style)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_style_user_push(sd->entry_edje, "elm.text", style);
   _elm_entry_smart_theme(obj);
}

EAPI void
elm_entry_text_style_user_pop(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_style_user_pop(sd->entry_edje, "elm.text");

   _elm_entry_smart_theme(obj);
}

EAPI const char *
elm_entry_text_style_user_peek(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) NULL;
   ELM_ENTRY_DATA_GET(obj, sd);

   return edje_object_part_text_style_user_peek(sd->entry_edje, "elm.text");
}

EAPI void
elm_entry_single_line_set(Evas_Object *obj,
                          Eina_Bool single_line)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->single_line == single_line) return;

   sd->single_line = single_line;
   sd->line_wrap = ELM_WRAP_NONE;
   elm_entry_cnp_mode_set(obj, ELM_CNP_MODE_NO_IMAGE);
   _elm_entry_smart_theme(obj);

   if (sd->scroll)
     {
        if (sd->single_line)
          sd->s_iface->policy_set
            (obj, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
        else
          {
             sd->s_iface->policy_set(obj, sd->policy_h, sd->policy_v);
          }
        elm_layout_sizing_eval(obj);
     }
}

EAPI Eina_Bool
elm_entry_single_line_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->single_line;
}

EAPI void
elm_entry_password_set(Evas_Object *obj,
                       Eina_Bool password)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   password = !!password;

   if (sd->password == password) return;
   sd->password = password;

   if (password)
     {
        sd->single_line = EINA_TRUE;
        sd->line_wrap = ELM_WRAP_NONE;
#ifdef HAVE_ELEMENTARY_X
        elm_drop_target_del(obj);
#endif
        _entry_selection_callbacks_unregister(obj);
     }
   else
     {
#ifdef HAVE_ELEMENTARY_X
        elm_drop_target_add(obj, ELM_SEL_FORMAT_MARKUP, _drag_drop_cb, NULL);
#endif
        _entry_selection_callbacks_register(obj);
     }

   _elm_entry_smart_theme(obj);
}

EAPI Eina_Bool
elm_entry_password_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->password;
}

EAPI void
elm_entry_entry_set(Evas_Object *obj,
                    const char *entry)
{
   _elm_entry_smart_text_set(obj, NULL, entry);
}

EAPI const char *
elm_entry_entry_get(const Evas_Object *obj)
{
   return _elm_entry_smart_text_get(obj, NULL);
}

EAPI void
elm_entry_entry_append(Evas_Object *obj,
                       const char *entry)
{
   int len = 0;

   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if (!entry) entry = "";

   sd->changed = EINA_TRUE;

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
        /* FIXME: Add chunked appending here (like in entry_set) */
        edje_object_part_text_append(sd->entry_edje, "elm.text", entry);
     }
}

EAPI Eina_Bool
elm_entry_is_empty(const Evas_Object *obj)
{
   /* FIXME: until there's support for that in textblock, we just
    * check to see if the there is text or not. */
   const Evas_Object *tb;
   Evas_Textblock_Cursor *cur;
   Eina_Bool ret;

   ELM_ENTRY_CHECK(obj) EINA_TRUE;
   ELM_ENTRY_DATA_GET(obj, sd);

   /* It's a hack until we get the support suggested above.  We just
    * create a cursor, point it to the begining, and then try to
    * advance it, if it can advance, the tb is not empty, otherwise it
    * is. */
   tb = edje_object_part_object_get(sd->entry_edje, "elm.text");

   /* This is actually, ok for the time being, these hackish stuff
      will be removed once evas 1.0 is out */
   cur = evas_object_textblock_cursor_new((Evas_Object *)tb);
   evas_textblock_cursor_pos_set(cur, 0);
   ret = evas_textblock_cursor_char_next(cur);
   evas_textblock_cursor_free(cur);

   return !ret;
}

EAPI Evas_Object *
elm_entry_textblock_get(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) NULL;
   ELM_ENTRY_DATA_GET(obj, sd);

   return (Evas_Object *)edje_object_part_object_get
            (sd->entry_edje, "elm.text");
}

EAPI void
elm_entry_calc_force(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_calc_force(sd->entry_edje);
   sd->changed = EINA_TRUE;
   elm_layout_sizing_eval(obj);
}

EAPI const char *
elm_entry_selection_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) NULL;
   ELM_ENTRY_DATA_GET(obj, sd);

   if ((sd->password)) return NULL;
   return edje_object_part_text_selection_get(sd->entry_edje, "elm.text");
}

EAPI void
elm_entry_entry_insert(Evas_Object *obj,
                       const char *entry)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_insert(sd->entry_edje, "elm.text", entry);
   sd->changed = EINA_TRUE;
   elm_layout_sizing_eval(obj);
}

EAPI void
elm_entry_line_wrap_set(Evas_Object *obj,
                        Elm_Wrap_Type wrap)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->line_wrap == wrap) return;
   sd->last_w = -1;
   sd->line_wrap = wrap;
   _elm_entry_smart_theme(obj);
}

EAPI Elm_Wrap_Type
elm_entry_line_wrap_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->line_wrap;
}

EAPI void
elm_entry_editable_set(Evas_Object *obj,
                       Eina_Bool editable)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->editable == editable) return;
   sd->editable = editable;
   _elm_entry_smart_theme(obj);

#ifdef HAVE_ELEMENTARY_X
   if (editable)
     elm_drop_target_add(obj, ELM_SEL_FORMAT_MARKUP, _drag_drop_cb, NULL);
   else
     elm_drop_target_del(obj);
#endif
}

EAPI Eina_Bool
elm_entry_editable_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->editable;
}

EAPI void
elm_entry_select_none(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if ((sd->password)) return;
   if (sd->sel_mode)
     {
        sd->sel_mode = EINA_FALSE;
        if (!_elm_config->desktop_entry)
          edje_object_part_text_select_allow_set
            (sd->entry_edje, "elm.text", EINA_FALSE);
        edje_object_signal_emit(sd->entry_edje, "elm,state,select,off", "elm");
     }
   sd->have_selection = EINA_FALSE;
   edje_object_part_text_select_none(sd->entry_edje, "elm.text");
}

EAPI void
elm_entry_select_all(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if ((sd->password)) return;
   if (sd->sel_mode)
     {
        sd->sel_mode = EINA_FALSE;
        if (!_elm_config->desktop_entry)
          edje_object_part_text_select_allow_set
            (sd->entry_edje, "elm.text", EINA_FALSE);
        edje_object_signal_emit(sd->entry_edje, "elm,state,select,off", "elm");
     }
   sd->have_selection = EINA_TRUE;
   edje_object_part_text_select_all(sd->entry_edje, "elm.text");
}

EAPI Eina_Bool
elm_entry_cursor_geometry_get(const Evas_Object *obj,
                              Evas_Coord *x,
                              Evas_Coord *y,
                              Evas_Coord *w,
                              Evas_Coord *h)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_cursor_geometry_get
     (sd->entry_edje, "elm.text", x, y, w, h);
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_entry_cursor_next(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return edje_object_part_text_cursor_next
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI Eina_Bool
elm_entry_cursor_prev(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return edje_object_part_text_cursor_prev
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI Eina_Bool
elm_entry_cursor_up(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return edje_object_part_text_cursor_up
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI Eina_Bool
elm_entry_cursor_down(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return edje_object_part_text_cursor_down
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_cursor_begin_set(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_cursor_begin_set
     (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_cursor_end_set(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_cursor_end_set
     (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_cursor_line_begin_set(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_cursor_line_begin_set
     (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_cursor_line_end_set(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_cursor_line_end_set
     (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_cursor_selection_begin(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_select_begin(sd->entry_edje, "elm.text");
}

EAPI void
elm_entry_cursor_selection_end(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_select_extend(sd->entry_edje, "elm.text");
}

EAPI Eina_Bool
elm_entry_cursor_is_format_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return edje_object_part_text_cursor_is_format_get
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI Eina_Bool
elm_entry_cursor_is_visible_format_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return edje_object_part_text_cursor_is_visible_format_get
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI char *
elm_entry_cursor_content_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) NULL;
   ELM_ENTRY_DATA_GET(obj, sd);

   return edje_object_part_text_cursor_content_get
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_cursor_pos_set(Evas_Object *obj,
                         int pos)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_cursor_pos_set
     (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN, pos);
   edje_object_message_signal_process(sd->entry_edje);
}

EAPI int
elm_entry_cursor_pos_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) 0;
   ELM_ENTRY_DATA_GET(obj, sd);

   return edje_object_part_text_cursor_pos_get
            (sd->entry_edje, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_selection_cut(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if ((sd->password)) return;
   _cut_cb(obj, NULL, NULL);
}

EAPI void
elm_entry_selection_copy(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if ((sd->password)) return;
   _copy_cb(obj, NULL, NULL);
}

EAPI void
elm_entry_selection_paste(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if ((sd->password)) return;
   _paste_cb(obj, NULL, NULL);
}

EAPI void
elm_entry_context_menu_clear(Evas_Object *obj)
{
   Elm_Entry_Context_Menu_Item *it;

   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   EINA_LIST_FREE (sd->items, it)
     {
        eina_stringshare_del(it->label);
        eina_stringshare_del(it->icon_file);
        eina_stringshare_del(it->icon_group);
        free(it);
     }
}

EAPI void
elm_entry_context_menu_item_add(Evas_Object *obj,
                                const char *label,
                                const char *icon_file,
                                Elm_Icon_Type icon_type,
                                Evas_Smart_Cb func,
                                const void *data)
{
   Elm_Entry_Context_Menu_Item *it;

   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

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

EAPI void
elm_entry_context_menu_disabled_set(Evas_Object *obj,
                                    Eina_Bool disabled)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->context_menu == !disabled) return;
   sd->context_menu = !disabled;
}

EAPI Eina_Bool
elm_entry_context_menu_disabled_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return !sd->context_menu;
}

EAPI void
elm_entry_item_provider_append(Evas_Object *obj,
                               Elm_Entry_Item_Provider_Cb func,
                               void *data)
{
   Elm_Entry_Item_Provider *ip;

   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(func);

   ip = calloc(1, sizeof(Elm_Entry_Item_Provider));
   if (!ip) return;

   ip->func = func;
   ip->data = data;
   sd->item_providers = eina_list_append(sd->item_providers, ip);
}

EAPI void
elm_entry_item_provider_prepend(Evas_Object *obj,
                                Elm_Entry_Item_Provider_Cb func,
                                void *data)
{
   Elm_Entry_Item_Provider *ip;

   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(func);

   ip = calloc(1, sizeof(Elm_Entry_Item_Provider));
   if (!ip) return;

   ip->func = func;
   ip->data = data;
   sd->item_providers = eina_list_prepend(sd->item_providers, ip);
}

EAPI void
elm_entry_item_provider_remove(Evas_Object *obj,
                               Elm_Entry_Item_Provider_Cb func,
                               void *data)
{
   Eina_List *l;
   Elm_Entry_Item_Provider *ip;

   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(func);

   EINA_LIST_FOREACH (sd->item_providers, l, ip)
     {
        if ((ip->func == func) && ((!data) || (ip->data == data)))
          {
             sd->item_providers = eina_list_remove_list(sd->item_providers, l);
             free(ip);
             return;
          }
     }
}

EAPI void
elm_entry_markup_filter_append(Evas_Object *obj,
                               Elm_Entry_Filter_Cb func,
                               void *data)
{
   Elm_Entry_Markup_Filter *tf;

   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(func);

   tf = _filter_new(func, data);
   if (!tf) return;

   sd->markup_filters = eina_list_append(sd->markup_filters, tf);
}

EAPI void
elm_entry_markup_filter_prepend(Evas_Object *obj,
                                Elm_Entry_Filter_Cb func,
                                void *data)
{
   Elm_Entry_Markup_Filter *tf;

   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(func);

   tf = _filter_new(func, data);
   if (!tf) return;

   sd->markup_filters = eina_list_prepend(sd->markup_filters, tf);
}

EAPI void
elm_entry_markup_filter_remove(Evas_Object *obj,
                               Elm_Entry_Filter_Cb func,
                               void *data)
{
   Eina_List *l;
   Elm_Entry_Markup_Filter *tf;

   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(func);

   EINA_LIST_FOREACH (sd->markup_filters, l, tf)
     {
        if ((tf->func == func) && ((!data) || (tf->data == data)))
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
             evas_object_smart_callback_call(entry, SIG_MAX_LENGHT, NULL);
             free(*text);
             *text = NULL;
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
             evas_object_smart_callback_call(entry, SIG_MAX_LENGHT, NULL);
             free(*text);
             *text = NULL;
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
                            Evas_Object *entry __UNUSED__,
                            char **text)
{
   int read_idx, last_read_idx = 0, read_char;
   Elm_Entry_Filter_Accept_Set *as = data;
   Eina_Bool goes_in;
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
        if (in_set == goes_in)
          {
             int size = read_idx - last_read_idx;
             const char *src = (*text) + last_read_idx;
             if (src != insert)
               memcpy(insert, *text + last_read_idx, size);
             insert += size;
          }
        last_read_idx = read_idx;
        read_idx = evas_string_char_next_get(*text, read_idx, &read_char);
     }
   *insert = 0;
}

EAPI Eina_Bool
elm_entry_file_set(Evas_Object *obj,
                   const char *file,
                   Elm_Text_Format format)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->delay_write)
     {
        ecore_timer_del(sd->delay_write);
        sd->delay_write = NULL;
     }

   if (sd->auto_save) _save_do(obj);
   eina_stringshare_replace(&sd->file, file);
   sd->format = format;
   return _load_do(obj);
}

EAPI void
elm_entry_file_get(const Evas_Object *obj,
                   const char **file,
                   Elm_Text_Format *format)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if (file) *file = sd->file;
   if (format) *format = sd->format;
}

EAPI void
elm_entry_file_save(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->delay_write)
     {
        ecore_timer_del(sd->delay_write);
        sd->delay_write = NULL;
     }
   _save_do(obj);
   sd->delay_write = ecore_timer_add(2.0, _delay_write, obj);
}

EAPI void
elm_entry_autosave_set(Evas_Object *obj,
                       Eina_Bool auto_save)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   sd->auto_save = !!auto_save;
}

EAPI Eina_Bool
elm_entry_autosave_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

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

EAPI void
elm_entry_cnp_mode_set(Evas_Object *obj,
                       Elm_Cnp_Mode cnp_mode)
{
   Elm_Sel_Format format = ELM_SEL_FORMAT_MARKUP;

   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->cnp_mode == cnp_mode) return;
   sd->cnp_mode = cnp_mode;
   if (sd->cnp_mode == ELM_CNP_MODE_PLAINTEXT)
     format = ELM_SEL_FORMAT_TEXT;
   else if (cnp_mode == ELM_CNP_MODE_MARKUP)
     format |= ELM_SEL_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   elm_drop_target_add(obj, format, _drag_drop_cb, NULL);
#endif
}

EAPI Elm_Cnp_Mode
elm_entry_cnp_mode_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) ELM_CNP_MODE_MARKUP;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->cnp_mode;
}

EAPI void
elm_entry_scrollable_set(Evas_Object *obj,
                         Eina_Bool scroll)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   scroll = !!scroll;
   if (sd->scroll == scroll) return;
   sd->scroll = scroll;

   if (sd->scroll)
     {
        /* we now must re-theme ourselves to a scroller decoration
         * and move the entry looking object to be the content of the
         * scrollable view */
        elm_widget_resize_object_set(obj, NULL);
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

        elm_widget_resize_object_set(obj, sd->scr_edje);

        sd->s_iface->objects_set(obj, sd->scr_edje, sd->hit_rect);

        sd->s_iface->bounce_allow_set(obj, sd->h_bounce, sd->v_bounce);
        if (sd->single_line)
          sd->s_iface->policy_set
            (obj, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
        else
          sd->s_iface->policy_set(obj, sd->policy_h, sd->policy_v);
        sd->s_iface->content_set(obj, sd->entry_edje);
        elm_widget_on_show_region_hook_set(obj, _show_region_hook, obj);
     }
   else
     {
        if (sd->scr_edje)
          {
             sd->s_iface->content_set(obj, NULL);
             evas_object_hide(sd->scr_edje);
          }
        elm_widget_resize_object_set(obj, sd->entry_edje);

        if (sd->scr_edje)
          elm_widget_sub_object_add(obj, sd->scr_edje);

        sd->s_iface->objects_set(obj, sd->entry_edje, sd->hit_rect);

        elm_widget_on_show_region_hook_set(obj, NULL, NULL);
     }
   sd->last_w = -1;
   _elm_entry_smart_theme(obj);
}

EAPI Eina_Bool
elm_entry_scrollable_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->scroll;
}

EAPI void
elm_entry_icon_visible_set(Evas_Object *obj,
                           Eina_Bool setting)
{
   ELM_ENTRY_CHECK(obj);

   if (!elm_layout_content_get(obj, "elm.swallow.icon")) return;

   if (setting)
     elm_layout_signal_emit(obj, "elm,action,show,icon", "elm");
   else
     elm_layout_signal_emit(obj, "elm,action,hide,icon", "elm");

   elm_layout_sizing_eval(obj);
}

EAPI void
elm_entry_end_visible_set(Evas_Object *obj,
                          Eina_Bool setting)
{
   ELM_ENTRY_CHECK(obj);

   if (!elm_layout_content_get(obj, "elm.swallow.icon")) return;

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
   ELM_ENTRY_DATA_GET(obj, sd);

   sd->policy_h = h;
   sd->policy_v = v;
   sd->s_iface->policy_set(obj, sd->policy_h, sd->policy_v);
}

EAPI void
elm_entry_bounce_set(Evas_Object *obj,
                     Eina_Bool h_bounce,
                     Eina_Bool v_bounce)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   sd->h_bounce = h_bounce;
   sd->v_bounce = v_bounce;
   sd->s_iface->bounce_allow_set(obj, h_bounce, v_bounce);
}

EAPI void
elm_entry_bounce_get(const Evas_Object *obj,
                     Eina_Bool *h_bounce,
                     Eina_Bool *v_bounce)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   sd->s_iface->bounce_allow_get(obj, h_bounce, v_bounce);
}

EAPI void
elm_entry_input_panel_layout_set(Evas_Object *obj,
                                 Elm_Input_Panel_Layout layout)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   sd->input_panel_layout = layout;

   edje_object_part_text_input_panel_layout_set
     (sd->entry_edje, "elm.text", layout);
}

EAPI Elm_Input_Panel_Layout
elm_entry_input_panel_layout_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) ELM_INPUT_PANEL_LAYOUT_INVALID;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->input_panel_layout;
}

EAPI void
elm_entry_autocapital_type_set(Evas_Object *obj,
                               Elm_Autocapital_Type autocapital_type)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   sd->autocapital_type = autocapital_type;
   edje_object_part_text_autocapital_type_set
     (sd->entry_edje, "elm.text", autocapital_type);
}

EAPI Elm_Autocapital_Type
elm_entry_autocapital_type_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) ELM_AUTOCAPITAL_TYPE_NONE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->autocapital_type;
}

EAPI void
elm_entry_prediction_allow_set(Evas_Object *obj,
                               Eina_Bool prediction)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   sd->prediction_allow = prediction;
   edje_object_part_text_prediction_allow_set
     (sd->entry_edje, "elm.text", prediction);
}

EAPI Eina_Bool
elm_entry_prediction_allow_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_TRUE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->prediction_allow;
}

EAPI void
elm_entry_imf_context_reset(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_imf_context_reset(sd->entry_edje, "elm.text");
}

EAPI void
elm_entry_input_panel_enabled_set(Evas_Object *obj,
                                  Eina_Bool enabled)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   sd->input_panel_enable = enabled;
   edje_object_part_text_input_panel_enabled_set
     (sd->entry_edje, "elm.text", enabled);
}

EAPI Eina_Bool
elm_entry_input_panel_enabled_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_TRUE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->input_panel_enable;
}

EAPI void
elm_entry_input_panel_show(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_input_panel_show(sd->entry_edje, "elm.text");
}

EAPI void
elm_entry_input_panel_hide(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_input_panel_hide(sd->entry_edje, "elm.text");
}

EAPI void
elm_entry_input_panel_language_set(Evas_Object *obj,
                                   Elm_Input_Panel_Lang lang)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   sd->input_panel_lang = lang;
   edje_object_part_text_input_panel_language_set
     (sd->entry_edje, "elm.text", lang);
}

EAPI Elm_Input_Panel_Lang
elm_entry_input_panel_language_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) ELM_INPUT_PANEL_LANG_AUTOMATIC;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->input_panel_lang;
}

EAPI void
elm_entry_input_panel_imdata_set(Evas_Object *obj,
                                 const void *data,
                                 int len)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->input_panel_imdata)
     free(sd->input_panel_imdata);

   sd->input_panel_imdata = calloc(1, len);
   sd->input_panel_imdata_len = len;
   memcpy(sd->input_panel_imdata, data, len);

   edje_object_part_text_input_panel_imdata_set
     (sd->entry_edje, "elm.text", sd->input_panel_imdata,
     sd->input_panel_imdata_len);
}

EAPI void
elm_entry_input_panel_imdata_get(const Evas_Object *obj,
                                 void *data,
                                 int *len)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   edje_object_part_text_input_panel_imdata_get
     (sd->entry_edje, "elm.text", data, len);
}

EAPI void
elm_entry_input_panel_return_key_type_set(Evas_Object *obj,
                                          Elm_Input_Panel_Return_Key_Type
                                          return_key_type)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   sd->input_panel_return_key_type = return_key_type;

   edje_object_part_text_input_panel_return_key_type_set
     (sd->entry_edje, "elm.text", return_key_type);
}

EAPI Elm_Input_Panel_Return_Key_Type
elm_entry_input_panel_return_key_type_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) ELM_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->input_panel_return_key_type;
}

EAPI void
elm_entry_input_panel_return_key_disabled_set(Evas_Object *obj,
                                              Eina_Bool disabled)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   sd->input_panel_return_key_disabled = disabled;

   edje_object_part_text_input_panel_return_key_disabled_set
     (sd->entry_edje, "elm.text", disabled);
}

EAPI Eina_Bool
elm_entry_input_panel_return_key_disabled_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->input_panel_return_key_disabled;
}

EAPI void
elm_entry_input_panel_return_key_autoenabled_set(Evas_Object *obj,
                                                 Eina_Bool enabled)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   sd->auto_return_key = enabled;
   _return_key_enabled_check(obj);
}

EAPI void *
elm_entry_imf_context_get(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) NULL;
   ELM_ENTRY_DATA_GET(obj, sd);
   if (!sd) return NULL;

   return edje_object_part_text_imf_context_get(sd->entry_edje, "elm.text");
}

/* START - ANCHOR HOVER */
static void
_anchor_parent_del_cb(void *data,
                      Evas *e __UNUSED__,
                      Evas_Object *obj __UNUSED__,
                      void *event_info __UNUSED__)
{
   ELM_ENTRY_DATA_GET(data, sd);

   sd->anchor_hover.hover_parent = NULL;
}

EAPI void
elm_entry_anchor_hover_parent_set(Evas_Object *obj,
                                  Evas_Object *parent)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

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

EAPI Evas_Object *
elm_entry_anchor_hover_parent_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) NULL;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->anchor_hover.hover_parent;
}

EAPI void
elm_entry_anchor_hover_style_set(Evas_Object *obj,
                                 const char *style)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   eina_stringshare_replace(&sd->anchor_hover.hover_style, style);
}

EAPI const char *
elm_entry_anchor_hover_style_get(const Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj) NULL;
   ELM_ENTRY_DATA_GET(obj, sd);

   return sd->anchor_hover.hover_style;
}

EAPI void
elm_entry_anchor_hover_end(Evas_Object *obj)
{
   ELM_ENTRY_CHECK(obj);
   ELM_ENTRY_DATA_GET(obj, sd);

   if (sd->anchor_hover.hover) evas_object_del(sd->anchor_hover.hover);
   if (sd->anchor_hover.pop) evas_object_del(sd->anchor_hover.pop);
   sd->anchor_hover.hover = NULL;
   sd->anchor_hover.pop = NULL;
}

/* END - ANCHOR HOVER */
