#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "els_scroller.h"


/* Maximum chunk size to be inserted to the entry at once
 * FIXME: This size is arbitrary, should probably choose a better size.
 * Possibly also find a way to set it to a low value for weak computers,
 * and to a big value for better computers. */
#define _CHUNK_SIZE 10000

typedef struct _Mod_Api Mod_Api;

typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Entry_Context_Menu_Item Elm_Entry_Context_Menu_Item;
typedef struct _Elm_Entry_Item_Provider Elm_Entry_Item_Provider;
typedef struct _Elm_Entry_Markup_Filter Elm_Entry_Markup_Filter;

struct _Widget_Data
{
   Evas_Object *ent, *scroller;
   Evas_Object *hoversel;
   Ecore_Job *deferred_recalc_job;
   Ecore_Event_Handler *sel_notify_handler;
   Ecore_Event_Handler *sel_clear_handler;
   Ecore_Timer *longpress_timer;
   Ecore_Timer *delay_write;
   /* for deferred appending */
   Ecore_Idler *append_text_idler;
   char *append_text_left;
   int append_text_position;
   int append_text_len;
   /* Only for clipboard */
   const char *cut_sel;
   const char *text;
   const char *file;
   Elm_Text_Format format;
   Evas_Coord lastw, entmw, entmh;
   Evas_Coord downx, downy;
   Eina_List *items;
   Eina_List *item_providers;
   Eina_List *text_filters;
   Eina_List *markup_filters;
   Ecore_Job *hovdeljob;
   Mod_Api *api; // module api if supplied
   int cursor_pos;
   Elm_Scroller_Policy policy_h, policy_v;
   Elm_Wrap_Type linewrap;
   Elm_Input_Panel_Layout input_panel_layout;
   Elm_Autocapital_Type autocapital_type;
   Elm_Input_Panel_Lang input_panel_lang;
   Elm_Input_Panel_Return_Key_Type input_panel_return_key_type;
   void *input_panel_imdata;
   int input_panel_imdata_len;
   struct
     {
        Evas_Object *hover_parent;
        Evas_Object *pop, *hover;
        const char *hover_style;
     } anchor_hover;
   Eina_Bool changed : 1;
   Eina_Bool single_line : 1;
   Eina_Bool password : 1;
   Eina_Bool editable : 1;
   Eina_Bool selection_asked : 1;
   Eina_Bool have_selection : 1;
   Eina_Bool selmode : 1;
   Eina_Bool deferred_cur : 1;
   Eina_Bool cur_changed : 1;
   Eina_Bool disabled : 1;
   Eina_Bool context_menu : 1;
   Eina_Bool drag_selection_asked : 1;
   Eina_Bool can_write : 1;
   Eina_Bool autosave : 1;
   Eina_Bool usedown : 1;
   Eina_Bool scroll : 1;
   Eina_Bool h_bounce : 1;
   Eina_Bool v_bounce : 1;
   Eina_Bool input_panel_enable : 1;
   Eina_Bool prediction_allow : 1;
   Eina_Bool input_panel_return_key_disabled : 1;
   Eina_Bool autoreturnkey : 1;
   Eina_Bool havetext : 1;
   Elm_Cnp_Mode cnp_mode : 2;
};

struct _Elm_Entry_Context_Menu_Item
{
   Evas_Object *obj;
   const char *label;
   const char *icon_file;
   const char *icon_group;
   Elm_Icon_Type icon_type;
   Evas_Smart_Cb func;
   void *data;
};

struct _Elm_Entry_Item_Provider
{
   Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item);
   void *data;
};

struct _Elm_Entry_Markup_Filter
{
   Elm_Entry_Filter_Cb func;
   void *data;
};

typedef enum _Length_Unit
{
   LENGTH_UNIT_CHAR,
   LENGTH_UNIT_BYTE,
   LENGTH_UNIT_LAST
} Length_Unit;

static const char *widtype = NULL;

#ifdef HAVE_ELEMENTARY_X
static Eina_Bool _drag_drop_cb(void *data, Evas_Object *obj, Elm_Selection_Data *);
#endif
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _content_set_hook(Evas_Object *obj, const char *part, Evas_Object *content);
static Evas_Object *_content_unset_hook(Evas_Object *obj, const char *part);
static Evas_Object *_content_get_hook(const Evas_Object *obj, const char *part);
static void _resize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static const char *_getbase(Evas_Object *obj);
static void _signal_entry_changed(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_selection_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_selection_changed(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_selection_cleared(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_entry_paste_request(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_entry_copy_notify(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_entry_cut_notify(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_cursor_changed(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _add_chars_till_limit(Evas_Object *obj, char **text, int can_add, Length_Unit unit);
static void _entry_hover_anchor_clicked(void *data, Evas_Object *obj, void *event_info);

static const char SIG_CHANGED[] = "changed";
static const char SIG_CHANGED_USER[] = "changed,user";
static const char SIG_ACTIVATED[] = "activated";
static const char SIG_ABORTED[] = "aborted";
static const char SIG_PRESS[] = "press";
static const char SIG_LONGPRESSED[] = "longpressed";
static const char SIG_CLICKED[] = "clicked";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_CLICKED_TRIPLE[] = "clicked,triple";
static const char SIG_FOCUSED[] = "focused";
static const char SIG_UNFOCUSED[] = "unfocused";
static const char SIG_SELECTION_PASTE[] = "selection,paste";
static const char SIG_SELECTION_COPY[] = "selection,copy";
static const char SIG_SELECTION_CUT[] = "selection,cut";
static const char SIG_SELECTION_START[] = "selection,start";
static const char SIG_SELECTION_CHANGED[] = "selection,changed";
static const char SIG_SELECTION_CLEARED[] = "selection,cleared";
static const char SIG_CURSOR_CHANGED[] = "cursor,changed";
static const char SIG_CURSOR_CHANGED_MANUAL[] = "cursor,changed,manual";
static const char SIG_ANCHOR_CLICKED[] = "anchor,clicked";
static const char SIG_ANCHOR_HOVER_OPENED[] = "anchor,hover,opened";
static const char SIG_ANCHOR_DOWN[] = "anchor,down";
static const char SIG_ANCHOR_UP[] = "anchor,up";
static const char SIG_ANCHOR_IN[] = "anchor,in";
static const char SIG_ANCHOR_OUT[] = "anchor,out";
static const char SIG_PREEDIT_CHANGED[] = "preedit,changed";
static const char SIG_UNDO_REQUEST[] = "undo,request";
static const char SIG_REDO_REQUEST[] = "redo,request";
static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_CHANGED, ""},
       {SIG_ACTIVATED, ""},
       {SIG_ABORTED, ""},
       {SIG_PRESS, ""},
       {SIG_LONGPRESSED, ""},
       {SIG_CLICKED, ""},
       {SIG_CLICKED_DOUBLE, ""},
       {SIG_CLICKED_TRIPLE, ""},
       {SIG_FOCUSED, ""},
       {SIG_UNFOCUSED, ""},
       {SIG_SELECTION_PASTE, ""},
       {SIG_SELECTION_COPY, ""},
       {SIG_SELECTION_CUT, ""},
       {SIG_SELECTION_START, ""},
       {SIG_SELECTION_CHANGED, ""},
       {SIG_SELECTION_CLEARED, ""},
       {SIG_CURSOR_CHANGED, ""},
       {SIG_CURSOR_CHANGED_MANUAL, ""},
       {SIG_ANCHOR_CLICKED, ""},
       {SIG_ANCHOR_HOVER_OPENED, ""},
       {SIG_ANCHOR_DOWN, ""},
       {SIG_ANCHOR_UP, ""},
       {SIG_ANCHOR_IN, ""},
       {SIG_ANCHOR_OUT, ""},
       {SIG_PREEDIT_CHANGED, ""},
       {SIG_CHANGED_USER, ""},
       {SIG_UNDO_REQUEST, ""},
       {SIG_REDO_REQUEST, ""},
       {NULL, NULL}
};

static Eina_List *entries = NULL;

struct _Mod_Api
{
   void (*obj_hook) (Evas_Object *obj);
   void (*obj_unhook) (Evas_Object *obj);
   void (*obj_longpress) (Evas_Object *obj);
};

static Mod_Api *
_module(Evas_Object *obj __UNUSED__)
{
   static Elm_Module *m = NULL;
   if (m) goto ok; // already found - just use
   if (!(m = _elm_module_find_as("entry/api"))) return NULL;
   // get module api
   m->api = malloc(sizeof(Mod_Api));
   if (!m->api) return NULL;
   ((Mod_Api *)(m->api)      )->obj_hook = // called on creation
      _elm_module_symbol_get(m, "obj_hook");
   ((Mod_Api *)(m->api)      )->obj_unhook = // called on deletion
      _elm_module_symbol_get(m, "obj_unhook");
   ((Mod_Api *)(m->api)      )->obj_longpress = // called on long press menu
      _elm_module_symbol_get(m, "obj_longpress");
ok: // ok - return api
   return m->api;
}

static char *
_buf_append(char *buf, const char *str, int *len, int *alloc)
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
_load_file(const char *file)
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
_load_plain(const char *file)
{
   char *text;

   text = _load_file(file);
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
_load(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char *text;
   if (!wd) return EINA_FALSE;
   if (!wd->file)
     {
        elm_object_text_set(obj, "");
        return EINA_TRUE;
     }
   switch (wd->format)
     {
      case ELM_TEXT_FORMAT_PLAIN_UTF8:
         text = _load_plain(wd->file);
         break;
      case ELM_TEXT_FORMAT_MARKUP_UTF8:
         text = _load_file(wd->file);
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
_save_markup_utf8(const char *file, const char *text)
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
_save_plain_utf8(const char *file, const char *text)
{
   char *text2;

   text2 = elm_entry_markup_to_utf8(text);
   if (!text2)
     return;
   _save_markup_utf8(file, text2);
   free(text2);
}

static void
_save(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!wd->file) return;
   switch (wd->format)
     {
      case ELM_TEXT_FORMAT_PLAIN_UTF8:
         _save_plain_utf8(wd->file, elm_object_text_get(obj));
         break;
      case ELM_TEXT_FORMAT_MARKUP_UTF8:
         _save_markup_utf8(wd->file, elm_object_text_get(obj));
         break;
      default:
         break;
     }
}

static Eina_Bool
_delay_write(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return ECORE_CALLBACK_CANCEL;
   _save(data);
   wd->delay_write = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_elm_entry_update_guide(Evas_Object *obj, Eina_Bool havetext)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((havetext) && (!wd->havetext))
     edje_object_signal_emit(wd->ent, "elm,guide,disabled", "elm");
   else if ((!havetext) && (wd->havetext))
     edje_object_signal_emit(wd->ent, "elm,guide,enabled", "elm");
   wd->havetext = havetext;
}

static Elm_Entry_Markup_Filter *
_filter_new(Elm_Entry_Filter_Cb func, void *data)
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
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->delay_write)
     {
        ecore_timer_del(wd->delay_write);
        wd->delay_write = NULL;
        if (wd->autosave) _save(obj);
     }
   elm_entry_anchor_hover_end(obj);
   elm_entry_anchor_hover_parent_set(obj, NULL);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Entry_Context_Menu_Item *it;
   Elm_Entry_Item_Provider *ip;
   Elm_Entry_Markup_Filter *tf;

   evas_event_freeze(evas_object_evas_get(obj));

   if (wd->file) eina_stringshare_del(wd->file);

   if (wd->hovdeljob) ecore_job_del(wd->hovdeljob);
   if ((wd->api) && (wd->api->obj_unhook)) wd->api->obj_unhook(obj); // module - unhook

   entries = eina_list_remove(entries, obj);
#ifdef HAVE_ELEMENTARY_X
   if (wd->sel_notify_handler)
     ecore_event_handler_del(wd->sel_notify_handler);
   if (wd->sel_clear_handler)
     ecore_event_handler_del(wd->sel_clear_handler);
#endif
   if (wd->cut_sel) eina_stringshare_del(wd->cut_sel);
   if (wd->text) eina_stringshare_del(wd->text);
   if (wd->deferred_recalc_job) ecore_job_del(wd->deferred_recalc_job);
   if (wd->append_text_idler)
     {
        ecore_idler_del(wd->append_text_idler);
        free(wd->append_text_left);
        wd->append_text_left = NULL;
        wd->append_text_idler = NULL;
     }
   if (wd->longpress_timer) ecore_timer_del(wd->longpress_timer);
   EINA_LIST_FREE(wd->items, it)
     {
        eina_stringshare_del(it->label);
        eina_stringshare_del(it->icon_file);
        eina_stringshare_del(it->icon_group);
        free(it);
     }
   EINA_LIST_FREE(wd->item_providers, ip)
     {
        free(ip);
     }
   EINA_LIST_FREE(wd->text_filters, tf)
     {
        _filter_free(tf);
     }
   EINA_LIST_FREE(wd->markup_filters, tf)
     {
        _filter_free(tf);
     }
   if (wd->delay_write) ecore_timer_del(wd->delay_write);
   if (wd->input_panel_imdata) free(wd->input_panel_imdata);

   if (wd->anchor_hover.hover_style) eina_stringshare_del(wd->anchor_hover.hover_style);
   evas_object_smart_callback_del_full(obj, "sub-object-del", _sub_del, wd);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_mirrored_set(wd->ent, rtl);
   if (wd->anchor_hover.hover)
      elm_widget_mirrored_set(wd->anchor_hover.hover, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *t;

   evas_event_freeze(evas_object_evas_get(obj));
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   t = eina_stringshare_add(elm_object_text_get(obj));
   _elm_theme_object_set(obj, wd->ent, "entry", _getbase(obj), elm_widget_style_get(obj));
   if (_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set(wd->ent, "elm.text", EINA_TRUE);
   elm_object_text_set(obj, t);
   eina_stringshare_del(t);
   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->ent, "elm,state,disabled", "elm");
   edje_object_part_text_input_panel_layout_set(wd->ent, "elm.text", wd->input_panel_layout);
   edje_object_part_text_autocapital_type_set(wd->ent, "elm.text", wd->autocapital_type);
   edje_object_part_text_prediction_allow_set(wd->ent, "elm.text", wd->prediction_allow);
   edje_object_part_text_input_panel_enabled_set(wd->ent, "elm.text", wd->input_panel_enable);
   edje_object_part_text_input_panel_imdata_set(wd->ent, "elm.text", wd->input_panel_imdata, wd->input_panel_imdata_len);
   edje_object_part_text_input_panel_return_key_type_set(wd->ent, "elm.text", wd->input_panel_return_key_type);
   edje_object_part_text_input_panel_return_key_disabled_set(wd->ent, "elm.text", wd->input_panel_return_key_disabled);

   if (wd->cursor_pos != 0)
     elm_entry_cursor_pos_set(obj, wd->cursor_pos);
   if (elm_widget_focus_get(obj))
     edje_object_signal_emit(wd->ent, "elm,action,focus", "elm");
   edje_object_message_signal_process(wd->ent);
   edje_object_scale_set(wd->ent, elm_widget_scale_get(obj) * _elm_config->scale);
   if (wd->scroll)
     {
        const char *str;
        Evas_Object *edj;

        elm_smart_scroller_mirrored_set(wd->scroller, elm_widget_mirrored_get(obj));
        elm_smart_scroller_object_theme_set(obj, wd->scroller, "scroller", "entry",
                                       elm_widget_style_get(obj));
        edj = elm_smart_scroller_edje_object_get(wd->scroller);
        str = edje_object_data_get(edj, "focus_highlight");
        if ((str) && (!strcmp(str, "on")))
          elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
        else
          elm_widget_highlight_in_theme_set(obj, EINA_FALSE);
     }
   _sizing_eval(obj);
   wd->havetext = !wd->havetext;
   _elm_entry_update_guide(obj, !wd->havetext);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (elm_widget_disabled_get(obj))
     {
        edje_object_signal_emit(wd->ent, "elm,state,disabled", "elm");
        wd->disabled = EINA_TRUE;
     }
   else
     {
        edje_object_signal_emit(wd->ent, "elm,state,enabled", "elm");
        wd->disabled = EINA_FALSE;
     }
}

static void
_recalc_cursor_geometry(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_smart_callback_call(obj, SIG_CURSOR_CHANGED, NULL);
   if (!wd->deferred_recalc_job)
     {
        Evas_Coord cx, cy, cw, ch;
        edje_object_part_text_cursor_geometry_get(wd->ent, "elm.text",
                                                  &cx, &cy, &cw, &ch);
        if (wd->cur_changed)
          {
             elm_widget_show_region_set(obj, cx, cy, cw, ch, EINA_FALSE);
             wd->cur_changed = EINA_FALSE;
          }
     }
   else
     wd->deferred_cur = EINA_TRUE;
}

static void
_elm_deferred_recalc_job(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord minh = -1, resw = -1, minw = -1, fw = 0, fh = 0;
   if (!wd) return;
   wd->deferred_recalc_job = NULL;

   evas_object_geometry_get(wd->ent, NULL, NULL, &resw, NULL);
   edje_object_size_min_restricted_calc(wd->ent, &minw, &minh, resw, 0);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   /* This is a hack to workaround the way min size hints are treated.
    * If the minimum width is smaller than the restricted width, it means
    * the minimum doesn't matter. */
   if (minw <= resw)
     {
        Evas_Coord ominw = -1;
        evas_object_size_hint_min_get(data, &ominw, NULL);
        minw = ominw;
     }

   wd->entmw = minw;
   wd->entmh = minh;

   elm_coords_finger_size_adjust(1, &fw, 1, &fh);
   if (wd->scroll)
     {
        Evas_Coord vmw = 0, vmh = 0;

        edje_object_size_min_calc
           (elm_smart_scroller_edje_object_get(wd->scroller),
               &vmw, &vmh);
        if (wd->single_line)
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
        if (wd->single_line)
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

   if (wd->deferred_cur)
     {
        Evas_Coord cx, cy, cw, ch;
        edje_object_part_text_cursor_geometry_get(wd->ent, "elm.text",
                                                  &cx, &cy, &cw, &ch);
        if (wd->cur_changed)
          {
             elm_widget_show_region_set(data, cx, cy, cw, ch, EINA_FALSE);
             wd->cur_changed = EINA_FALSE;
          }
     }
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord resw, resh;
   if (!wd) return;

   evas_object_geometry_get(obj, NULL, NULL, &resw, &resh);
   if (wd->linewrap)
     {
        if ((resw == wd->lastw) && (!wd->changed)) return;
        evas_event_freeze(evas_object_evas_get(obj));
        wd->changed = EINA_FALSE;
        wd->lastw = resw;
        if (wd->scroll)
          {
             Evas_Coord vw = 0, vh = 0, vmw = 0, vmh = 0, w = -1, h = -1;

             evas_object_resize(wd->scroller, resw, resh);
             edje_object_size_min_calc
                (elm_smart_scroller_edje_object_get(wd->scroller),
                 &vmw, &vmh);
             elm_smart_scroller_child_viewport_size_get(wd->scroller, &vw, &vh);
             edje_object_size_min_restricted_calc(wd->ent, &minw, &minh, vw, 0);
             elm_coords_finger_size_adjust(1, &minw, 1, &minh);
             /* This is a hack to workaround the way min size hints are treated.
              * If the minimum width is smaller than the restricted width, it means
              * the minimum doesn't matter. */
             if (minw <= vw)
               {
                  Evas_Coord ominw = -1;
                  evas_object_size_hint_min_get(wd->ent, &ominw, NULL);
                  minw = ominw;
               }
             wd->entmw = minw;
             wd->entmh = minh;

             if ((minw > 0) && (vw < minw)) vw = minw;
             if (minh > vh) vh = minh;

             if (wd->single_line) h = vmh + minh;
             else h = vmh;
             evas_object_resize(wd->ent, vw, vh);
             evas_object_size_hint_min_set(obj, w, h);
             if (wd->single_line)
               evas_object_size_hint_max_set(obj, -1, h);
             else
               evas_object_size_hint_max_set(obj, -1, -1);
          }
        else
          {
             if (wd->deferred_recalc_job) ecore_job_del(wd->deferred_recalc_job);
             wd->deferred_recalc_job = ecore_job_add(_elm_deferred_recalc_job, obj);
          }
        evas_event_thaw(evas_object_evas_get(obj));
        evas_event_thaw_eval(evas_object_evas_get(obj));
     }
   else
     {
        if (!wd->changed) return;
        evas_event_freeze(evas_object_evas_get(obj));
        wd->changed = EINA_FALSE;
        wd->lastw = resw;
        if (wd->scroll)
          {
             Evas_Coord vw = 0, vh = 0, vmw = 0, vmh = 0, w = -1, h = -1;

             edje_object_size_min_calc(wd->ent, &minw, &minh);
             wd->entmw = minw;
             wd->entmh = minh;
             elm_coords_finger_size_adjust(1, &minw, 1, &minh);

             elm_smart_scroller_child_viewport_size_get(wd->scroller, &vw, &vh);

             if (minw > vw) vw = minw;
             if (minh > vh) vh = minh;

             evas_object_resize(wd->ent, vw, vh);
             edje_object_size_min_calc
                (elm_smart_scroller_edje_object_get(wd->scroller),
                 &vmw, &vmh);
             if (wd->single_line) h = vmh + minh;
             else h = vmh;
             evas_object_size_hint_min_set(obj, w, h);
             if (wd->single_line)
               evas_object_size_hint_max_set(obj, -1, h);
             else
               evas_object_size_hint_max_set(obj, -1, -1);
          }
        else
          {
             edje_object_size_min_calc(wd->ent, &minw, &minh);
             wd->entmw = minw;
             wd->entmh = minh;
             elm_coords_finger_size_adjust(1, &minw, 1, &minh);
             evas_object_size_hint_min_set(obj, minw, minh);
             if (wd->single_line)
               evas_object_size_hint_max_set(obj, -1, minh);
             else
               evas_object_size_hint_max_set(obj, -1, -1);
          }
        evas_event_thaw(evas_object_evas_get(obj));
        evas_event_thaw_eval(evas_object_evas_get(obj));
     }

   _recalc_cursor_geometry(obj);
}

static void
_check_enable_return_key(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Bool return_key_disabled = EINA_FALSE;
   if (!wd) return;

   if (!wd->autoreturnkey) return;

   if (elm_entry_is_empty(obj) == EINA_TRUE)
     return_key_disabled = EINA_TRUE;

   elm_entry_input_panel_return_key_disabled_set(obj, return_key_disabled);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *top = elm_widget_top_get(obj);
   if (!wd) return;
   if (!wd->editable) return;
   if (elm_widget_focus_get(obj))
     {
        evas_object_focus_set(wd->ent, EINA_TRUE);
        edje_object_signal_emit(wd->ent, "elm,action,focus", "elm");
        if (top && wd->input_panel_enable)
          elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_ON);
        evas_object_smart_callback_call(obj, SIG_FOCUSED, NULL);
        _check_enable_return_key(obj);
     }
   else
     {
        edje_object_signal_emit(wd->ent, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->ent, EINA_FALSE);
        if (top && wd->input_panel_enable)
          elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_OFF);
        evas_object_smart_callback_call(obj, SIG_UNFOCUSED, NULL);
     }
}

static void
_content_set_hook(Evas_Object *obj, const char *part, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *edje;
   Evas_Object *prev_content;
   if ((!wd) || (!content)) return;

   if (wd->scroll)
      edje = elm_smart_scroller_edje_object_get(wd->scroller);
   else
      edje = wd->ent;

   if (!part || !strcmp(part, "icon"))
     {
        prev_content = edje_object_part_swallow_get(edje, "elm.swallow.icon");
        edje_object_signal_emit(edje, "elm,action,show,icon", "elm");
     }
   else if (!strcmp(part, "end"))
     {
        prev_content = edje_object_part_swallow_get(edje, "elm.swallow.end");
        edje_object_signal_emit(edje, "elm,action,show,end", "elm");
     }
   else
     prev_content = edje_object_part_swallow_get(edje, part);

   if (prev_content) evas_object_del(prev_content);

   evas_event_freeze(evas_object_evas_get(obj));
   elm_widget_sub_object_add(obj, content);

   if (!part || !strcmp(part, "icon"))
     edje_object_part_swallow(edje, "elm.swallow.icon", content);
   else if (!strcmp(part, "end"))
     edje_object_part_swallow(edje, "elm.swallow.end", content);
   else
     edje_object_part_swallow(edje, part, content);

   _sizing_eval(obj);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static Evas_Object *
_content_unset_hook(Evas_Object *obj, const char *part)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *content, *edje;
   if (!wd) return NULL;

   if (wd->scroll)
      edje = elm_smart_scroller_edje_object_get(wd->scroller);
   else
      edje = wd->ent;

   if (!part || !strcmp(part, "icon"))
     {
        edje_object_signal_emit(edje, "elm,action,hide,icon", "elm");
        content = edje_object_part_swallow_get(edje, "elm.swallow.icon");
     }
   else if (!strcmp(part, "end"))
     {
        edje_object_signal_emit(edje, "elm,action,hide,end", "elm");
        content = edje_object_part_swallow_get(edje, "elm.swallow.end");
     }
   else
     content = edje_object_part_swallow_get(edje, part);

   edje_object_part_swallow(edje, part, NULL);
   if (!content) return NULL;
   evas_event_freeze(evas_object_evas_get(obj));
   elm_widget_sub_object_del(obj, content);
   edje_object_part_unswallow(wd->ent, content);
   _sizing_eval(obj);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   return content;
}

static Evas_Object *
_content_get_hook(const Evas_Object *obj, const char *part)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *content = NULL, *edje;
   if (!wd) return NULL;

   if (wd->scroll)
      edje = elm_smart_scroller_edje_object_get(wd->scroller);
   else
      edje = wd->ent;

   if (!edje) return NULL;

   if (!part || !strcmp(part, "icon"))
     content = edje_object_part_swallow_get(edje, "elm.swallow.icon");
   else if (!strcmp(part, "end"))
     content = edje_object_part_swallow_get(edje, "elm.swallow.end");
   else
     content = edje_object_part_swallow_get(edje, part);

   return content;
}

static void
_translate_hook(Evas_Object *obj)
{
   evas_object_smart_callback_call(obj, "language,changed", NULL);
}

static void
_signal_emit_hook(Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_emit(wd->ent, emission, source);
   if (wd->scroller)
     edje_object_signal_emit(elm_smart_scroller_edje_object_get(wd->scroller),
                             emission, source);
}

static void
_signal_callback_add_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_callback_add(wd->ent, emission, source, func_cb, data);
   if (wd->scroller)
     edje_object_signal_callback_add(elm_smart_scroller_edje_object_get(wd->scroller),
                                     emission, source, func_cb, data);
}

static void
_signal_callback_del_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_signal_callback_del_full(wd->ent, emission, source, func_cb,
                                        data);
   if (wd->scroller)
     edje_object_signal_callback_del_full(elm_smart_scroller_edje_object_get(wd->scroller),
                                          emission, source, func_cb, data);
}

static void
_on_focus_region_hook(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_part_text_cursor_geometry_get(wd->ent, "elm.text", x, y, w, h);
}

static void
_focus_region_hook(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->scroll)
     elm_smart_scroller_child_region_show(wd->scroller, x, y, w, h);
}

static void
_show_region_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord x, y, w, h;
   if (!wd) return;
   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   if (wd->scroll)
     elm_smart_scroller_child_region_show(wd->scroller, x, y, w, h);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = data;
   Evas_Object *sub = event_info;
   Evas_Object *edje;

   if (wd->scroll)
      edje = elm_smart_scroller_edje_object_get(wd->scroller);
   else
      edje = wd->ent;

   if (sub == edje_object_part_swallow_get(edje, "elm.swallow.icon"))
     {
        edje_object_part_unswallow(edje, sub);
        if (edje)
          edje_object_signal_emit(edje, "elm,action,hide,icon", "elm");
     }
   else if (sub == edje_object_part_swallow_get(edje, "elm.swallow.end"))
     {
        edje_object_part_unswallow(edje, sub);
        if (edje)
          edje_object_signal_emit(edje, "elm,action,hide,end", "elm");
     }
   _sizing_eval(obj);
}

static void
_hoversel_position(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord cx, cy, cw, ch, x, y, mw, mh;
   if (!wd) return;

   cx = cy = 0;
   cw = ch = 1;
   evas_object_geometry_get(wd->ent, &x, &y, NULL, NULL);
   if (wd->usedown)
     {
        cx = wd->downx - x;
        cy = wd->downy - y;
        cw = 1;
        ch = 1;
     }
   else
     edje_object_part_text_cursor_geometry_get(wd->ent, "elm.text",
                                               &cx, &cy, &cw, &ch);
   evas_object_size_hint_min_get(wd->hoversel, &mw, &mh);
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
   evas_object_move(wd->hoversel, x + cx, y + cy);
   evas_object_resize(wd->hoversel, cw, ch);
}

static void
_move(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (wd->hoversel) _hoversel_position(data);
}

static void
_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   if (wd->linewrap)
     {
        _sizing_eval(data);
     }
   else if (wd->scroll)
     {
        Evas_Coord vw = 0, vh = 0;

        elm_smart_scroller_child_viewport_size_get(wd->scroller, &vw, &vh);
        if (vw < wd->entmw) vw = wd->entmw;
        if (vh < wd->entmh) vh = wd->entmh;
        evas_object_resize(wd->ent, vw, vh);
     }
   if (wd->hoversel) _hoversel_position(data);
}

static void
_hover_del(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   if (wd->hoversel)
     {
        evas_object_del(wd->hoversel);
        wd->hoversel = NULL;
     }
   wd->hovdeljob = NULL;
}

static void
_dismissed(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->usedown = 0;
   if (wd->hoversel) evas_object_hide(wd->hoversel);
   if (wd->selmode)
     {
        if (!_elm_config->desktop_entry)
          {
             if (!wd->password)
               edje_object_part_text_select_allow_set(wd->ent, "elm.text", EINA_TRUE);
          }
     }
   elm_widget_scroll_freeze_pop(data);
   if (wd->hovdeljob) ecore_job_del(wd->hovdeljob);
   wd->hovdeljob = ecore_job_add(_hover_del, data);
}

static void
_select(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->selmode = EINA_TRUE;
   edje_object_part_text_select_none(wd->ent, "elm.text");
   if (!_elm_config->desktop_entry)
     {
        if (!wd->password)
          edje_object_part_text_select_allow_set(wd->ent, "elm.text", EINA_TRUE);
     }
   edje_object_signal_emit(wd->ent, "elm,state,select,on", "elm");
   if (!_elm_config->desktop_entry)
     elm_widget_scroll_hold_push(data);
}

static char *
_remove_item_tags(const char *str)
{
   char *ret;
   if (!str)
     return NULL;

   Eina_Strbuf *buf = eina_strbuf_new();
   if (!buf)
     return NULL;

   if (!eina_strbuf_append(buf, str))
     return NULL;

   while (EINA_TRUE)
     {
        const char *temp = eina_strbuf_string_get(buf);

        char *startTag = NULL;
        char *endTag = NULL;

        startTag = strstr(temp, "<item");
        if (!startTag)
          startTag = strstr(temp, "</item");
        if (startTag)
          endTag = strstr(startTag, ">");
        else
          break;
        if (!endTag || startTag > endTag)
          break;

        size_t sindex = startTag - temp;
        size_t eindex = endTag - temp + 1;
        if (!eina_strbuf_remove(buf, sindex, eindex))
          break;
     }
   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

void
_elm_entry_entry_paste(Evas_Object *obj, const char *entry)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char *str = NULL;

   if (wd->cnp_mode == ELM_CNP_MODE_NO_IMAGE)
     {
        str = _remove_item_tags(entry);
        if (!str) str = strdup(entry);
     }
   else
     str = strdup(entry);
   if (!str) str = (char *)entry;

   edje_object_part_text_user_insert(wd->ent, "elm.text", str);
   if (str != entry) free(str);
}

static void
_paste(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_smart_callback_call(data, SIG_SELECTION_PASTE, NULL);
   if (wd->sel_notify_handler)
     {
#ifdef HAVE_ELEMENTARY_X
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
_store_selection(Elm_Sel_Type seltype, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *sel;

   if (!wd) return;
   sel = edje_object_part_text_selection_get(wd->ent, "elm.text");
   if ((!sel) || (!sel[0])) return; /* avoid deleting our own selection */
   elm_cnp_selection_set(obj, seltype, ELM_SEL_FORMAT_MARKUP, sel, strlen(sel));
   if (seltype == ELM_SEL_TYPE_CLIPBOARD)
     eina_stringshare_replace(&wd->cut_sel, sel);
}

static void
_cut(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   /* Store it */
   wd->selmode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set(wd->ent, "elm.text", EINA_FALSE);
   edje_object_signal_emit(wd->ent, "elm,state,select,off", "elm");
   if (!_elm_config->desktop_entry)
     elm_widget_scroll_hold_pop(data);
   _store_selection(ELM_SEL_TYPE_CLIPBOARD, data);
   edje_object_part_text_user_insert(wd->ent, "elm.text", "");
   _sizing_eval(data);
}

static void
_copy(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->selmode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     {
        edje_object_part_text_select_allow_set(wd->ent, "elm.text", EINA_FALSE);
        edje_object_signal_emit(wd->ent, "elm,state,select,off", "elm");
        elm_widget_scroll_hold_pop(data);
     }
   _store_selection(ELM_SEL_TYPE_CLIPBOARD, data);
   //   edje_object_part_text_select_none(wd->ent, "elm.text");
}

static void
_cancel(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->selmode = EINA_FALSE;
   if (!_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set(wd->ent, "elm.text", EINA_FALSE);
   edje_object_signal_emit(wd->ent, "elm,state,select,off", "elm");
   if (!_elm_config->desktop_entry)
     elm_widget_scroll_hold_pop(data);
   edje_object_part_text_select_none(wd->ent, "elm.text");
}

static void
_item_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Entry_Context_Menu_Item *it = data;
   Evas_Object *obj2 = it->obj;
   if (it->func) it->func(it->data, obj2, NULL);
}

static void
_menu_press(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *top;
   const Eina_List *l;
   const Elm_Entry_Context_Menu_Item *it;
   if (!wd) return;
   if ((wd->api) && (wd->api->obj_longpress))
     {
        wd->api->obj_longpress(obj);
     }
   else if (wd->context_menu)
     {
        const char *context_menu_orientation;

        if (wd->hoversel) evas_object_del(wd->hoversel);
        else elm_widget_scroll_freeze_push(obj);
        wd->hoversel = elm_hoversel_add(obj);
        context_menu_orientation = edje_object_data_get
           (wd->ent, "context_menu_orientation");
        if ((context_menu_orientation) &&
            (!strcmp(context_menu_orientation, "horizontal")))
          elm_hoversel_horizontal_set(wd->hoversel, EINA_TRUE);
        elm_object_style_set(wd->hoversel, "entry");
        elm_widget_sub_object_add(obj, wd->hoversel);
        elm_object_text_set(wd->hoversel, "Text");
        top = elm_widget_top_get(obj);
        if (top) elm_hoversel_hover_parent_set(wd->hoversel, top);
        evas_object_smart_callback_add(wd->hoversel, "dismissed", _dismissed, obj);
        if (wd->have_selection)
          {
             if (!wd->password)
               {
                  if (wd->have_selection)
                    {
                       elm_hoversel_item_add(wd->hoversel, E_("Copy"), NULL, ELM_ICON_NONE,
                                             _copy, obj);
                       if (wd->editable)
                         elm_hoversel_item_add(wd->hoversel, E_("Cut"), NULL, ELM_ICON_NONE,
                                               _cut, obj);
                    }
                  elm_hoversel_item_add(wd->hoversel, E_("Cancel"), NULL, ELM_ICON_NONE,
                                        _cancel, obj);
               }
          }
        else
          {
             if (!wd->selmode)
               {
                  if (!_elm_config->desktop_entry)
                    {
                       if (!wd->password)
                         elm_hoversel_item_add(wd->hoversel, E_("Select"), NULL, ELM_ICON_NONE,
                                               _select, obj);
                    }
                  if (elm_selection_selection_has_owner())
                    {
                       if (wd->editable)
                         elm_hoversel_item_add(wd->hoversel, E_("Paste"), NULL, ELM_ICON_NONE,
                                               _paste, obj);
                    }
               }
          }
        EINA_LIST_FOREACH(wd->items, l, it)
          {
             elm_hoversel_item_add(wd->hoversel, it->label, it->icon_file,
                                   it->icon_type, _item_clicked, it);
          }
        if (wd->hoversel)
          {
             _hoversel_position(obj);
             evas_object_show(wd->hoversel);
             elm_hoversel_hover_begin(wd->hoversel);
          }
        if (!_elm_config->desktop_entry)
          {
             edje_object_part_text_select_allow_set(wd->ent, "elm.text", EINA_FALSE);
             edje_object_part_text_select_abort(wd->ent, "elm.text");
          }
     }
}

static Eina_Bool
_long_press(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return ECORE_CALLBACK_CANCEL;
   if (!_elm_config->desktop_entry)
     _menu_press(data);
   wd->longpress_timer = NULL;
   evas_object_smart_callback_call(data, SIG_LONGPRESSED, NULL);
   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Down *ev = event_info;
   if (!wd) return;
   if (wd->disabled) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   wd->downx = ev->canvas.x;
   wd->downy = ev->canvas.y;
   if (ev->button == 1)
     {
        if (wd->longpress_timer) ecore_timer_del(wd->longpress_timer);
        wd->longpress_timer = ecore_timer_add(_elm_config->longpress_timeout, _long_press, data);
     }
   else if (ev->button == 3)
     {
        if (_elm_config->desktop_entry)
          _menu_press(data);
     }
}

static void
_mouse_up(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Up *ev = event_info;
   if (!wd) return;
   if (wd->disabled) return;
   if (ev->button == 1)
     {
        if (wd->longpress_timer)
          {
             ecore_timer_del(wd->longpress_timer);
             wd->longpress_timer = NULL;
          }
     }
   else if ((ev->button == 3) && (!_elm_config->desktop_entry))
     {
        wd->usedown = 1;
        _menu_press(data);
     }
}

static void
_mouse_move(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Move *ev = event_info;
   if (!wd) return;
   if (wd->disabled) return;
   if (!wd->selmode)
     {
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
          {
             if (wd->longpress_timer)
               {
                  ecore_timer_del(wd->longpress_timer);
                  wd->longpress_timer = NULL;
               }
          }
        else if (wd->longpress_timer)
          {
             Evas_Coord dx, dy;

             dx = wd->downx - ev->cur.canvas.x;
             dx *= dx;
             dy = wd->downy - ev->cur.canvas.y;
             dy *= dy;
             if ((dx + dy) >
                 ((_elm_config->finger_size / 2) *
                  (_elm_config->finger_size / 2)))
               {
                  ecore_timer_del(wd->longpress_timer);
                  wd->longpress_timer = NULL;
               }
          }
     }
   else if (wd->longpress_timer)
     {
        Evas_Coord dx, dy;

        dx = wd->downx - ev->cur.canvas.x;
        dx *= dx;
        dy = wd->downy - ev->cur.canvas.y;
        dy *= dy;
        if ((dx + dy) >
            ((_elm_config->finger_size / 2) *
             (_elm_config->finger_size / 2)))
          {
             ecore_timer_del(wd->longpress_timer);
             wd->longpress_timer = NULL;
          }
     }
}

static const char *
_getbase(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return "base";
   if (wd->editable)
     {
        if (wd->password) return "base-password";
        else
          {
             if (wd->single_line) return "base-single";
             else
               {
                  switch (wd->linewrap)
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
        if (wd->password) return "base-password";
        else
          {
             if (wd->single_line) return "base-single-noedit";
             else
               {
                  switch (wd->linewrap)
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
_entry_changed_common_handling(void *data, const char *event)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord minh;
   const char *text;
   if (!wd) return;
   evas_event_freeze(evas_object_evas_get(data));
   wd->changed = EINA_TRUE;
   /* Reset the size hints which are no more relevant.
    * Keep the height, this is a hack, but doesn't really matter
    * cause we'll re-eval in a moment. */
   evas_object_size_hint_min_get(data, NULL, &minh);
   evas_object_size_hint_min_set(data, -1, minh);
   _sizing_eval(data);
   if (wd->text) eina_stringshare_del(wd->text);
   wd->text = NULL;
   if (wd->delay_write)
     {
        ecore_timer_del(wd->delay_write);
        wd->delay_write = NULL;
     }
   evas_event_thaw(evas_object_evas_get(data));
   evas_event_thaw_eval(evas_object_evas_get(data));
   if ((wd->autosave) && (wd->file))
     wd->delay_write = ecore_timer_add(2.0, _delay_write, data);
   /* callback - this could call callbacks that delete the entry... thus...
    * any access to wd after this could be invalid */
   evas_object_smart_callback_call(data, event, NULL);
   _check_enable_return_key(data);
   text = edje_object_part_text_get(wd->ent, "elm.text");
   if (text)
     {
        if (text[0])
          _elm_entry_update_guide(data, EINA_TRUE);
        else
          _elm_entry_update_guide(data, EINA_FALSE);
     }
}

static void
_signal_entry_changed(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _entry_changed_common_handling(data, SIG_CHANGED);
}

static void
_signal_entry_changed_user(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
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
_signal_preedit_changed(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _entry_changed_common_handling(data, SIG_PREEDIT_CHANGED);
}

static void
_signal_undo_request(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_UNDO_REQUEST, NULL);
}

static void
_signal_redo_request(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_REDO_REQUEST, NULL);
}

static void
_signal_selection_start(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   const Eina_List *l;
   Evas_Object *entry;
   if (!wd) return;
   if (!elm_object_focus_get(data)) elm_object_focus_set(data, EINA_TRUE);
   EINA_LIST_FOREACH(entries, l, entry)
     {
        if (entry != data) elm_entry_select_none(entry);
     }
   wd->have_selection = EINA_TRUE;
   evas_object_smart_callback_call(data, SIG_SELECTION_START, NULL);
#ifdef HAVE_ELEMENTARY_X
   if (wd->sel_notify_handler)
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
_signal_selection_all(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   elm_entry_select_all(data);
}

static void
_signal_selection_none(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   elm_entry_select_none(data);
}

static void
_signal_selection_changed(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->have_selection = EINA_TRUE;
   evas_object_smart_callback_call(data, SIG_SELECTION_CHANGED, NULL);
   _store_selection(ELM_SEL_TYPE_PRIMARY, data);
}

static void
_signal_selection_cleared(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (!wd->have_selection) return;
   wd->have_selection = EINA_FALSE;
   evas_object_smart_callback_call(data, SIG_SELECTION_CLEARED, NULL);
   if (wd->sel_notify_handler)
     {
        if (wd->cut_sel)
          {
#ifdef HAVE_ELEMENTARY_X
             Evas_Object *top;

             top = elm_widget_top_get(data);
             if ((top) && (elm_win_xwindow_get(top)))
               elm_cnp_selection_set(data, ELM_SEL_TYPE_PRIMARY,
                                     ELM_SEL_FORMAT_MARKUP, wd->cut_sel,
                                     strlen(wd->cut_sel));
#endif
             eina_stringshare_del(wd->cut_sel);
             wd->cut_sel = NULL;
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
_signal_entry_paste_request(void *data, Evas_Object *obj __UNUSED__, const char *emission, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
#ifdef HAVE_ELEMENTARY_X
   Elm_Sel_Type type = (emission[sizeof("ntry,paste,request,")] == '1') ?
     ELM_SEL_TYPE_PRIMARY : ELM_SEL_TYPE_CLIPBOARD;
#endif

   if (!wd) return;
   if (!wd->editable) return;
   evas_object_smart_callback_call(data, SIG_SELECTION_PASTE, NULL);
   if (wd->sel_notify_handler)
     {
#ifdef HAVE_ELEMENTARY_X
        Evas_Object *top;

        top = elm_widget_top_get(data);
        if ((top) && (elm_win_xwindow_get(top)))
          {
             wd->selection_asked = EINA_TRUE;
             Elm_Sel_Format formats = ELM_SEL_FORMAT_MARKUP;
             if (wd->cnp_mode == ELM_CNP_MODE_PLAINTEXT)
               formats = ELM_SEL_FORMAT_TEXT;
             else if (wd->cnp_mode != ELM_CNP_MODE_NO_IMAGE)
               formats |= ELM_SEL_FORMAT_IMAGE;
             elm_cnp_selection_get(data, type, formats,
                                   NULL, NULL);
          }
#endif
     }
}

static void
_signal_entry_copy_notify(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_COPY, NULL);
   _copy(data, NULL, NULL);
}

static void
_signal_entry_cut_notify(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SELECTION_CUT, NULL);
   _cut(data, NULL, NULL);
}

static void
_signal_cursor_changed(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->cursor_pos = edje_object_part_text_cursor_pos_get(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
   wd->cur_changed = EINA_TRUE;
   _recalc_cursor_geometry(data);
}

static void
_signal_cursor_changed_manual(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CURSOR_CHANGED_MANUAL, NULL);
}


static void
_signal_anchor_geoms_do_things_with(Widget_Data *wd, Elm_Entry_Anchor_Info *ei)
{
   const Eina_List *geoms, *l;
   Evas_Textblock_Rectangle *r;
   Evas_Coord px, py, x, y;

   geoms = edje_object_part_text_anchor_geometry_get(wd->ent, "elm.text", ei->name);
   if (!geoms) return;


   evas_object_geometry_get(wd->ent, &x, &y, NULL, NULL);
   evas_pointer_canvas_xy_get(evas_object_evas_get(wd->ent), &px, &py);
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
_signal_anchor_down(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Entry_Anchor_Info ei;
   const char *p;
   char *p2;
   if (!wd) return;
   p = emission + sizeof("nchor,mouse,down,");
   ei.button = strtol(p, &p2, 10);
   ei.name = p2 + 1;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with(wd, &ei);

   if (!wd->disabled)
     evas_object_smart_callback_call(data, SIG_ANCHOR_DOWN, &ei);
}

static void
_signal_anchor_up(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Entry_Anchor_Info ei;
   const char *p;
   char *p2;
   if (!wd) return;
   p = emission + sizeof("nchor,mouse,up,");
   ei.button = strtol(p, &p2, 10);
   ei.name = p2 + 1;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with(wd, &ei);

   if (!wd->disabled)
     evas_object_smart_callback_call(data, SIG_ANCHOR_UP, &ei);
}

static void
_signal_anchor_clicked(void *data, Evas_Object *obj __UNUSED__, const char *emission, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Entry_Anchor_Info ei;
   const char *p;
   char *p2;
   if (!wd) return;
   p = emission + sizeof("nchor,mouse,clicked,");
   ei.button = strtol(p, &p2, 10);
   ei.name = p2 + 1;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with(wd, &ei);

   if (!wd->disabled)
     {
        evas_object_smart_callback_call(data, SIG_ANCHOR_CLICKED, &ei);
        _entry_hover_anchor_clicked(data, data, &ei);
     }
}

static void
_signal_anchor_move(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
}

static void
_signal_anchor_in(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Entry_Anchor_Info ei;
   if (!wd) return;
   ei.name = emission + sizeof("nchor,mouse,in,");
   ei.button = 0;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with(wd, &ei);

   if (!wd->disabled)
     evas_object_smart_callback_call(data, SIG_ANCHOR_IN, &ei);
}

static void
_signal_anchor_out(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Entry_Anchor_Info ei;
   if (!wd) return;
   ei.name = emission + sizeof("nchor,mouse,out,");
   ei.button = 0;
   ei.x = ei.y = ei.w = ei.h = 0;

   _signal_anchor_geoms_do_things_with(wd, &ei);

   if (!wd->disabled)
     evas_object_smart_callback_call(data, SIG_ANCHOR_OUT, &ei);
}

static void
_signal_key_enter(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_smart_callback_call(data, SIG_ACTIVATED, NULL);
}

static void
_signal_key_escape(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_smart_callback_call(data, SIG_ABORTED, NULL);
}

static void
_signal_mouse_down(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_smart_callback_call(data, SIG_PRESS, NULL);
}

static void
_signal_mouse_clicked(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_signal_mouse_double(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_smart_callback_call(data, SIG_CLICKED_DOUBLE, NULL);
}

static void
_signal_mouse_triple(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_smart_callback_call(data, SIG_CLICKED_TRIPLE, NULL);
}

#ifdef HAVE_ELEMENTARY_X
static Eina_Bool
_event_selection_notify(void *data, int type __UNUSED__, void *event)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Ecore_X_Event_Selection_Notify *ev = event;
   if (!wd) return ECORE_CALLBACK_PASS_ON;
   if ((!wd->selection_asked) && (!wd->drag_selection_asked))
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
        wd->selection_asked = EINA_FALSE;
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
        wd->drag_selection_asked = EINA_FALSE;

        ecore_x_dnd_send_finished();

     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_event_selection_clear(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Ecore_X_Event_Selection_Clear *ev = event;
   if (!wd) return ECORE_CALLBACK_PASS_ON;
   if (!wd->have_selection) return ECORE_CALLBACK_PASS_ON;
   if ((ev->selection == ECORE_X_SELECTION_CLIPBOARD) ||
       (ev->selection == ECORE_X_SELECTION_PRIMARY))
     {
        elm_entry_select_none(data);
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_drag_drop_cb(void *data __UNUSED__, Evas_Object *obj, Elm_Selection_Data *drop)
{
   Widget_Data *wd;
   Eina_Bool rv;

   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   printf("Inserting at (%d,%d) %s\n",drop->x,drop->y,(char*)drop->data);

   edje_object_part_text_cursor_copy(wd->ent, "elm.text",
                                     EDJE_CURSOR_MAIN,/*->*/EDJE_CURSOR_USER);
   rv = edje_object_part_text_cursor_coord_set(wd->ent,"elm.text",
                                               EDJE_CURSOR_MAIN,drop->x,drop->y);
   if (!rv) printf("Warning: Failed to position cursor: paste anyway\n");
   elm_entry_entry_insert(obj, drop->data);
   edje_object_part_text_cursor_copy(wd->ent, "elm.text",
                                     EDJE_CURSOR_USER,/*->*/EDJE_CURSOR_MAIN);

   return EINA_TRUE;
}
#endif

static Evas_Object *
_get_item(void *data, Evas_Object *edje __UNUSED__, const char *part __UNUSED__, const char *item)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Object *o;
   Eina_List *l;
   Elm_Entry_Item_Provider *ip;

   EINA_LIST_FOREACH(wd->item_providers, l, ip)
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
             _elm_theme_object_set(data, o, "entry/emoticon", "wtf", elm_widget_style_get(data));
          }
        return o;
     }
   o = edje_object_add(evas_object_evas_get(data));
   if (!_elm_theme_object_set(data, o, "entry", item, elm_widget_style_get(data)))
     _elm_theme_object_set(data, o, "entry/emoticon", "wtf", elm_widget_style_get(data));
   return o;
}

static void
_text_filter(void *data, Evas_Object *edje __UNUSED__, const char *part __UNUSED__, Edje_Text_Filter_Type type, char **text)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Eina_List *l;
   Elm_Entry_Markup_Filter *tf;

   if (type == EDJE_TEXT_FILTER_FORMAT)
     return;

   EINA_LIST_FOREACH(wd->text_filters, l, tf)
     {
        tf->func(tf->data, data, text);
        if (!*text)
          break;
     }
}

static void
_markup_filter(void *data, Evas_Object *edje __UNUSED__, const char *part __UNUSED__, char **text)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Eina_List *l;
   Elm_Entry_Markup_Filter *tf;

   EINA_LIST_FOREACH(wd->markup_filters, l, tf)
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
   Evas_Object *obj = (Evas_Object *) data;
   Widget_Data *wd = elm_widget_data_get(obj);
   evas_event_freeze(evas_object_evas_get(obj));
   if (wd->text) eina_stringshare_del(wd->text);
   wd->text = NULL;
   wd->changed = EINA_TRUE;

   start = wd->append_text_position;
   if ((start + _CHUNK_SIZE) < wd->append_text_len)
     {
        int pos = start;
        int tag_start, esc_start;

        tag_start = esc_start = -1;
        /* Find proper markup cut place */
        while (pos - start < _CHUNK_SIZE)
          {
             int prev_pos = pos;
             Eina_Unicode tmp =
                eina_unicode_utf8_get_next(wd->append_text_left, &pos);
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
             wd->append_text_position = tag_start;
          }
        else if (esc_start >= 0)
          {
             wd->append_text_position = esc_start;
          }
        else
          {
             wd->append_text_position = pos;
          }
     }
   else
     {
        wd->append_text_position = wd->append_text_len;
     }

   backup = wd->append_text_left[wd->append_text_position];
   wd->append_text_left[wd->append_text_position] = '\0';

   edje_object_part_text_append(wd->ent, "elm.text",
                                wd->append_text_left + start);

   wd->append_text_left[wd->append_text_position] = backup;

   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   _elm_entry_update_guide(obj, EINA_TRUE);
   
   /* If there's still more to go, renew the idler, else, cleanup */
   if (wd->append_text_position < wd->append_text_len)
     {
        return ECORE_CALLBACK_RENEW;
     }
   else
     {
        free(wd->append_text_left);
        wd->append_text_left = NULL;
        wd->append_text_idler = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
}

static void
_add_chars_till_limit(Evas_Object *obj, char **text, int can_add, Length_Unit unit)
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
                  evas_object_smart_callback_call(obj, "maxlength,reached", NULL);
                  free(*text);
                  *text = NULL;
                  return;
               }
             can_add = 0;
             strncpy(new_text, new_text + idx, current_len - ((new_text + idx) - *text));
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
   evas_object_smart_callback_call(obj, "maxlength,reached", NULL);
}

static void
_elm_entry_text_set(Evas_Object *obj, const char *item, const char *entry)
{
   int len = 0;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!entry) entry = "";
   if (item)
     {
        if (!strcmp(item, "guide"))
          edje_object_part_text_set(wd->ent, "elm.guide", entry);
        else
          edje_object_part_text_set(wd->ent, item, entry);
        return;
     }

   evas_event_freeze(evas_object_evas_get(obj));
   if (wd->text) eina_stringshare_del(wd->text);
   wd->text = NULL;
   wd->changed = EINA_TRUE;

   /* Clear currently pending job if there is one */
   if (wd->append_text_idler)
     {
        ecore_idler_del(wd->append_text_idler);
        free(wd->append_text_left);
        wd->append_text_left = NULL;
        wd->append_text_idler = NULL;
     }

   len = strlen(entry);
   /* Split to ~_CHUNK_SIZE chunks */
   if (len > _CHUNK_SIZE)
     {
        wd->append_text_left = (char *) malloc(len + 1);
     }

   /* If we decided to use the idler */
   if (wd->append_text_left)
     {
        /* Need to clear the entry first */
        edje_object_part_text_set(wd->ent, "elm.text", "");
        memcpy(wd->append_text_left, entry, len + 1);
        wd->append_text_position = 0;
        wd->append_text_len = len;
        wd->append_text_idler = ecore_idler_add(_text_append_idler, obj);
     }
   else
     {
        edje_object_part_text_set(wd->ent, "elm.text", entry);
     }
   if ((entry) && (entry[0]))
     _elm_entry_update_guide(obj, EINA_TRUE);
   else
     _elm_entry_update_guide(obj, EINA_FALSE);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static const char *
_elm_entry_text_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && strcmp(item, "default")) return NULL;
   const char *text;
   if (!wd) return NULL;
   text = edje_object_part_text_get(wd->ent, "elm.text");
   if (!text)
     {
        ERR("text=NULL for edje %p, part 'elm.text'", wd->ent);
        return NULL;
     }

   if (wd->append_text_len > 0)
     {
        char *tmpbuf;
        size_t tlen;
        tlen = strlen(text);
        tmpbuf = malloc(wd->append_text_len + 1);
        if (!tmpbuf)
          {
             ERR("Failed to allocate memory for entry's text %p", obj);
             return NULL;
          }
        memcpy(tmpbuf, text, tlen);
        if (wd->append_text_left)
          memcpy(tmpbuf + tlen, wd->append_text_left + wd->append_text_position, wd->append_text_len - wd->append_text_position);
        tmpbuf[wd->append_text_len] = '\0';
        eina_stringshare_replace(&wd->text, tmpbuf);
        free(tmpbuf);
     }
   else
     {
        eina_stringshare_replace(&wd->text, text);
     }
   return wd->text;
}

static char *
_access_info_cb(void *data __UNUSED__,
                Evas_Object *obj,
                Elm_Widget_Item *item __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd || wd->password) return NULL;

   const char *txt = elm_widget_access_info_get(obj);

   if (!txt) txt = elm_entry_entry_get(obj);
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data __UNUSED__,
                 Evas_Object *obj,
                 Elm_Widget_Item *item __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   Eina_Strbuf *buf;
   buf = eina_strbuf_new();

   if (elm_widget_disabled_get(obj))
     eina_strbuf_append(buf, "State: Disabled");

   if (!wd->editable)
     {
        if (!eina_strbuf_length_get(buf))
          eina_strbuf_append(buf, "State: Not Editable");
        else eina_strbuf_append(buf, ", Not Editable");
     }

   if (wd->password)
     {
        if (!eina_strbuf_length_get(buf))
          eina_strbuf_append(buf, "State: Password");
        else eina_strbuf_append(buf, ", Password");
     }

   char *txt = strdup(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   if (txt) return txt;

   return NULL;
}

EAPI Evas_Object *
elm_entry_add(Evas_Object *parent)
{
#ifdef HAVE_ELEMENTARY_X
   Evas_Object *top;
#endif
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "entry");
   elm_widget_type_set(obj, "entry");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_focus_region_hook_set(obj, _focus_region_hook);
   elm_widget_on_focus_region_hook_set(obj, _on_focus_region_hook);
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_text_set_hook_set(obj, _elm_entry_text_set);
   elm_widget_text_get_hook_set(obj, _elm_entry_text_get);
   elm_widget_content_set_hook_set(obj, _content_set_hook);
   elm_widget_content_unset_hook_set(obj, _content_unset_hook);
   elm_widget_content_get_hook_set(obj, _content_get_hook);
   elm_widget_translate_hook_set(obj, _translate_hook);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, wd);

   wd->linewrap     = ELM_WRAP_WORD;
   wd->editable     = EINA_TRUE;
   wd->disabled     = EINA_FALSE;
   wd->context_menu = EINA_TRUE;
   wd->autosave     = EINA_TRUE;
   wd->cnp_mode     = ELM_CNP_MODE_MARKUP;
   wd->scroll       = EINA_FALSE;
   wd->input_panel_imdata = NULL;

   wd->ent = edje_object_add(e);
   elm_object_sub_cursor_set(wd->ent, obj, ELM_CURSOR_XTERM);
   edje_object_item_provider_set(wd->ent, _get_item, obj);
   edje_object_text_insert_filter_callback_add(wd->ent,"elm.text", _text_filter, obj);
   edje_object_text_markup_filter_callback_add(wd->ent,"elm.text", _markup_filter, obj);
   evas_object_event_callback_add(wd->ent, EVAS_CALLBACK_MOVE, _move, obj);
   evas_object_event_callback_add(wd->ent, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down, obj);
   evas_object_event_callback_add(wd->ent, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, obj);
   evas_object_event_callback_add(wd->ent, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, obj);

   _elm_theme_object_set(obj, wd->ent, "entry", "base", "default");
   edje_object_signal_callback_add(wd->ent, "entry,changed", "elm.text",
                                   _signal_entry_changed, obj);
   edje_object_signal_callback_add(wd->ent, "entry,changed,user", "elm.text",
                                   _signal_entry_changed_user, obj);
   edje_object_signal_callback_add(wd->ent, "preedit,changed", "elm.text",
                                   _signal_preedit_changed, obj);
   edje_object_signal_callback_add(wd->ent, "selection,start", "elm.text",
                                   _signal_selection_start, obj);
   edje_object_signal_callback_add(wd->ent, "selection,changed", "elm.text",
                                   _signal_selection_changed, obj);
   edje_object_signal_callback_add(wd->ent, "entry,selection,all,request", "elm.text",
                                   _signal_selection_all, obj);
   edje_object_signal_callback_add(wd->ent, "entry,selection,none,request", "elm.text",
                                   _signal_selection_none, obj);
   edje_object_signal_callback_add(wd->ent, "selection,cleared", "elm.text",
                                   _signal_selection_cleared, obj);
   edje_object_signal_callback_add(wd->ent, "entry,paste,request,*", "elm.text",
                                   _signal_entry_paste_request, obj);
   edje_object_signal_callback_add(wd->ent, "entry,copy,notify", "elm.text",
                                   _signal_entry_copy_notify, obj);
   edje_object_signal_callback_add(wd->ent, "entry,cut,notify", "elm.text",
                                   _signal_entry_cut_notify, obj);
   edje_object_signal_callback_add(wd->ent, "cursor,changed", "elm.text",
                                   _signal_cursor_changed, obj);
   edje_object_signal_callback_add(wd->ent, "cursor,changed,manual", "elm.text",
                                   _signal_cursor_changed_manual, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,down,*", "elm.text",
                                   _signal_anchor_down, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,up,*", "elm.text",
                                   _signal_anchor_up, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,clicked,*", "elm.text",
                                   _signal_anchor_clicked, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,move,*", "elm.text",
                                   _signal_anchor_move, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,in,*", "elm.text",
                                   _signal_anchor_in, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,out,*", "elm.text",
                                   _signal_anchor_out, obj);
   edje_object_signal_callback_add(wd->ent, "entry,key,enter", "elm.text",
                                   _signal_key_enter, obj);
   edje_object_signal_callback_add(wd->ent, "entry,key,escape", "elm.text",
                                   _signal_key_escape, obj);
   edje_object_signal_callback_add(wd->ent, "mouse,down,1", "elm.text",
                                   _signal_mouse_down, obj);
   edje_object_signal_callback_add(wd->ent, "mouse,clicked,1", "elm.text",
                                   _signal_mouse_clicked, obj);
   edje_object_signal_callback_add(wd->ent, "mouse,down,1,double", "elm.text",
                                   _signal_mouse_double, obj);
   edje_object_signal_callback_add(wd->ent, "mouse,down,1,triple", "elm.text",
                                   _signal_mouse_triple, obj);
   edje_object_signal_callback_add(wd->ent, "entry,undo,request", "elm.text",
                                   _signal_undo_request, obj);
   edje_object_signal_callback_add(wd->ent, "entry,redo,request", "elm.text",
                                   _signal_redo_request, obj);
   edje_object_part_text_set(wd->ent, "elm.text", "");
   if (_elm_config->desktop_entry)
     edje_object_part_text_select_allow_set(wd->ent, "elm.text", EINA_TRUE);
   elm_widget_resize_object_set(obj, wd->ent);
   _sizing_eval(obj);

   elm_entry_input_panel_layout_set(obj, ELM_INPUT_PANEL_LAYOUT_NORMAL);
   elm_entry_input_panel_enabled_set(obj, EINA_TRUE);
   elm_entry_prediction_allow_set(obj, EINA_TRUE);

   wd->autocapital_type = edje_object_part_text_autocapital_type_get(wd->ent, "elm.text");

#ifdef HAVE_ELEMENTARY_X
   top = elm_widget_top_get(obj);
   if ((top) && (elm_win_xwindow_get(top)))
     {
        wd->sel_notify_handler =
           ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY,
                                   _event_selection_notify, obj);
        wd->sel_clear_handler =
           ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR,
                                   _event_selection_clear, obj);
     }

   elm_drop_target_add(obj, ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE,
                       _drag_drop_cb, NULL);
#endif

   entries = eina_list_prepend(entries, obj);

   // module - find module for entry
   wd->api = _module(obj);
   // if found - hook in
   if ((wd->api) && (wd->api->obj_hook)) wd->api->obj_hook(obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   // access
   _elm_access_object_register(obj, wd->ent);
   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_TYPE, E_("Entry"));
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_STATE, _access_state_cb, NULL);

   return obj;
}

EAPI void
elm_entry_text_style_user_push(Evas_Object *obj, const char *style)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_part_text_style_user_push(wd->ent, "elm.text", style);
   _theme_hook(obj);
}

EAPI void
elm_entry_text_style_user_pop(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_part_text_style_user_pop(wd->ent, "elm.text");
   _theme_hook(obj);
}

EAPI const char*
elm_entry_text_style_user_peek(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return edje_object_part_text_style_user_peek(wd->ent, "elm.text");
}

EAPI void
elm_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->single_line == single_line) return;
   wd->single_line = single_line;
   wd->linewrap = ELM_WRAP_NONE;
   elm_entry_cnp_mode_set(obj, ELM_CNP_MODE_NO_IMAGE);
   _theme_hook(obj);
   if (wd->scroller)
     {
        if (wd->single_line)
          elm_smart_scroller_policy_set(wd->scroller,
                                        ELM_SMART_SCROLLER_POLICY_OFF,
                                        ELM_SMART_SCROLLER_POLICY_OFF);
        else
          {
             const Elm_Scroller_Policy map[3] =
               {
                  ELM_SMART_SCROLLER_POLICY_AUTO,
                  ELM_SMART_SCROLLER_POLICY_ON,
                  ELM_SMART_SCROLLER_POLICY_OFF
               };
             elm_smart_scroller_policy_set(wd->scroller,
                                           map[wd->policy_h],
                                           map[wd->policy_v]);
          }
        _sizing_eval(obj);
     }
}

EAPI Eina_Bool
elm_entry_single_line_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->single_line;
}

EAPI void
elm_entry_password_set(Evas_Object *obj, Eina_Bool password)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->password == password) return;
   wd->password = password;
   if (password)
     {
        wd->single_line = EINA_TRUE;
        wd->linewrap = ELM_WRAP_NONE;
#ifdef HAVE_ELEMENTARY_X
        elm_drop_target_del(obj);
#endif
        edje_object_signal_callback_del_full(wd->ent, "selection,start", "elm.text",
                                        _signal_selection_start, obj);
        edje_object_signal_callback_del_full(wd->ent, "selection,changed", "elm.text",
                                        _signal_selection_changed, obj);
        edje_object_signal_callback_del_full(wd->ent, "entry,selection,all,request", "elm.text",
                                        _signal_selection_all, obj);
        edje_object_signal_callback_del_full(wd->ent, "entry,selection,none,request", "elm.text",
                                        _signal_selection_none, obj);
        edje_object_signal_callback_del_full(wd->ent, "selection,cleared", "elm.text",
                                        _signal_selection_cleared, obj);
        edje_object_signal_callback_del_full(wd->ent, "entry,paste,request,*", "elm.text",
                                        _signal_entry_paste_request, obj);
        edje_object_signal_callback_del_full(wd->ent, "entry,copy,notify", "elm.text",
                                        _signal_entry_copy_notify, obj);
        edje_object_signal_callback_del_full(wd->ent, "entry,cut,notify", "elm.text",
                                        _signal_entry_cut_notify, obj);
     }
   else
     {
#ifdef HAVE_ELEMENTARY_X
        elm_drop_target_add(obj, ELM_SEL_FORMAT_MARKUP, _drag_drop_cb, NULL);
#endif
        edje_object_signal_callback_add(wd->ent, "selection,start", "elm.text",
                                        _signal_selection_start, obj);
        edje_object_signal_callback_add(wd->ent, "selection,changed", "elm.text",
                                        _signal_selection_changed, obj);
        edje_object_signal_callback_add(wd->ent, "entry,selection,all,request", "elm.text",
                                        _signal_selection_all, obj);
        edje_object_signal_callback_add(wd->ent, "entry,selection,none,request", "elm.text",
                                        _signal_selection_none, obj);
        edje_object_signal_callback_add(wd->ent, "selection,cleared", "elm.text",
                                        _signal_selection_cleared, obj);
        edje_object_signal_callback_add(wd->ent, "entry,paste,request,*", "elm.text",
                                        _signal_entry_paste_request, obj);
        edje_object_signal_callback_add(wd->ent, "entry,copy,notify", "elm.text",
                                        _signal_entry_copy_notify, obj);
        edje_object_signal_callback_add(wd->ent, "entry,cut,notify", "elm.text",
                                        _signal_entry_cut_notify, obj);
     }
   _theme_hook(obj);
}

EAPI Eina_Bool
elm_entry_password_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->password;
}

EAPI void
elm_entry_entry_set(Evas_Object *obj, const char *entry)
{
   _elm_entry_text_set(obj, NULL, entry);
}

EAPI const char *
elm_entry_entry_get(const Evas_Object *obj)
{
   return _elm_entry_text_get(obj, NULL);
}

EAPI void
elm_entry_entry_append(Evas_Object *obj, const char *entry)
{
   int len = 0;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!entry) entry = "";
   wd->changed = EINA_TRUE;

   len = strlen(entry);
   if (wd->append_text_left)
     {
        char *tmpbuf;
        tmpbuf = realloc(wd->append_text_left, wd->append_text_len + len + 1);
        if (!tmpbuf)
          {
             /* Do something */
             return;
          }
        wd->append_text_left = tmpbuf;
        memcpy(wd->append_text_left + wd->append_text_len, entry, len + 1);
        wd->append_text_len += len;
     }
   else
     {
        /* FIXME: Add chunked appending here (like in entry_set) */
        edje_object_part_text_append(wd->ent, "elm.text", entry);
     }
}

EAPI Eina_Bool
elm_entry_is_empty(const Evas_Object *obj)
{
   /* FIXME: until there's support for that in textblock, we just check
    * to see if the there is text or not. */
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_TRUE;
   Widget_Data *wd = elm_widget_data_get(obj);
   const Evas_Object *tb;
   Evas_Textblock_Cursor *cur;
   Eina_Bool ret;
   if (!wd) return EINA_TRUE;
   /* It's a hack until we get the support suggested above.
    * We just create a cursor, point it to the begining, and then
    * try to advance it, if it can advance, the tb is not empty,
    * otherwise it is. */
   tb = edje_object_part_object_get(wd->ent, "elm.text");
   cur = evas_object_textblock_cursor_new((Evas_Object *) tb); /* This is
                                                                  actually, ok for the time being, these hackish stuff will be removed
                                                                  once evas 1.0 is out*/
   evas_textblock_cursor_pos_set(cur, 0);
   ret = evas_textblock_cursor_char_next(cur);
   evas_textblock_cursor_free(cur);

   return !ret;
}

EAPI Evas_Object *
elm_entry_textblock_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   return (Evas_Object *) edje_object_part_object_get(wd->ent, "elm.text");
}

EAPI void
elm_entry_calc_force(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   edje_object_calc_force(wd->ent);
   wd->changed = EINA_TRUE;
   _sizing_eval(obj);
}


EAPI const char *
elm_entry_selection_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (wd->password)) return NULL;
   return edje_object_part_text_selection_get(wd->ent, "elm.text");
}

EAPI void
elm_entry_entry_insert(Evas_Object *obj, const char *entry)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_part_text_insert(wd->ent, "elm.text", entry);
   wd->changed = EINA_TRUE;
   _sizing_eval(obj);
}

EAPI void
elm_entry_line_wrap_set(Evas_Object *obj, Elm_Wrap_Type wrap)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->linewrap == wrap) return;
   wd->lastw = -1;
   wd->linewrap = wrap;
   _theme_hook(obj);
}

EAPI Elm_Wrap_Type
elm_entry_line_wrap_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->linewrap;
}

EAPI void
elm_entry_editable_set(Evas_Object *obj, Eina_Bool editable)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->editable == editable) return;
   wd->editable = editable;
   _theme_hook(obj);

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
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->editable;
}

EAPI void
elm_entry_select_none(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (wd->password)) return;
   if (wd->selmode)
     {
        wd->selmode = EINA_FALSE;
        if (!_elm_config->desktop_entry)
          edje_object_part_text_select_allow_set(wd->ent, "elm.text", EINA_FALSE);
        edje_object_signal_emit(wd->ent, "elm,state,select,off", "elm");
     }
   wd->have_selection = EINA_FALSE;
   edje_object_part_text_select_none(wd->ent, "elm.text");
}

EAPI void
elm_entry_select_all(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (wd->password)) return;
   if (wd->selmode)
     {
        wd->selmode = EINA_FALSE;
        if (!_elm_config->desktop_entry)
          edje_object_part_text_select_allow_set(wd->ent, "elm.text", EINA_FALSE);
        edje_object_signal_emit(wd->ent, "elm,state,select,off", "elm");
     }
   wd->have_selection = EINA_TRUE;
   edje_object_part_text_select_all(wd->ent, "elm.text");
}

EAPI Eina_Bool
elm_entry_cursor_geometry_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   edje_object_part_text_cursor_geometry_get(wd->ent, "elm.text", x, y, w, h);
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_entry_cursor_next(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return edje_object_part_text_cursor_next(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI Eina_Bool
elm_entry_cursor_prev(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return edje_object_part_text_cursor_prev(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI Eina_Bool
elm_entry_cursor_up(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return edje_object_part_text_cursor_up(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI Eina_Bool
elm_entry_cursor_down(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return edje_object_part_text_cursor_down(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_cursor_begin_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_part_text_cursor_begin_set(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_cursor_end_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_part_text_cursor_end_set(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_cursor_line_begin_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_part_text_cursor_line_begin_set(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_cursor_line_end_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_part_text_cursor_line_end_set(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_cursor_selection_begin(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_part_text_select_begin(wd->ent, "elm.text");
}

EAPI void
elm_entry_cursor_selection_end(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_part_text_select_extend(wd->ent, "elm.text");
}

EAPI Eina_Bool
elm_entry_cursor_is_format_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return edje_object_part_text_cursor_is_format_get(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI Eina_Bool
elm_entry_cursor_is_visible_format_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return edje_object_part_text_cursor_is_visible_format_get(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI char *
elm_entry_cursor_content_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return edje_object_part_text_cursor_content_get(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_cursor_pos_set(Evas_Object *obj, int pos)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_part_text_cursor_pos_set(wd->ent, "elm.text", EDJE_CURSOR_MAIN, pos);
   edje_object_message_signal_process(wd->ent);
}

EAPI int
elm_entry_cursor_pos_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return edje_object_part_text_cursor_pos_get(wd->ent, "elm.text", EDJE_CURSOR_MAIN);
}

EAPI void
elm_entry_selection_cut(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (wd->password)) return;
   _cut(obj, NULL, NULL);
}

EAPI void
elm_entry_selection_copy(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (wd->password)) return;
   _copy(obj, NULL, NULL);
}

EAPI void
elm_entry_selection_paste(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (wd->password)) return;
   _paste(obj, NULL, NULL);
}

EAPI void
elm_entry_context_menu_clear(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Entry_Context_Menu_Item *it;
   if (!wd) return;
   EINA_LIST_FREE(wd->items, it)
     {
        eina_stringshare_del(it->label);
        eina_stringshare_del(it->icon_file);
        eina_stringshare_del(it->icon_group);
        free(it);
     }
}

EAPI void
elm_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Entry_Context_Menu_Item *it;
   if (!wd) return;
   it = calloc(1, sizeof(Elm_Entry_Context_Menu_Item));
   if (!it) return;
   wd->items = eina_list_append(wd->items, it);
   it->obj = obj;
   it->label = eina_stringshare_add(label);
   it->icon_file = eina_stringshare_add(icon_file);
   it->icon_type = icon_type;
   it->func = func;
   it->data = (void *)data;
}

EAPI void
elm_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->context_menu == !disabled) return;
   wd->context_menu = !disabled;
}

EAPI Eina_Bool
elm_entry_context_menu_disabled_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return !wd->context_menu;
}

EAPI void
elm_entry_item_provider_append(Evas_Object *obj, Elm_Entry_Item_Provider_Cb func, void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(func);
   Elm_Entry_Item_Provider *ip = calloc(1, sizeof(Elm_Entry_Item_Provider));
   if (!ip) return;
   ip->func = func;
   ip->data = data;
   wd->item_providers = eina_list_append(wd->item_providers, ip);
}

EAPI void
elm_entry_item_provider_prepend(Evas_Object *obj, Elm_Entry_Item_Provider_Cb func, void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(func);
   Elm_Entry_Item_Provider *ip = calloc(1, sizeof(Elm_Entry_Item_Provider));
   if (!ip) return;
   ip->func = func;
   ip->data = data;
   wd->item_providers = eina_list_prepend(wd->item_providers, ip);
}

EAPI void
elm_entry_item_provider_remove(Evas_Object *obj, Elm_Entry_Item_Provider_Cb func, void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Elm_Entry_Item_Provider *ip;
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(func);
   EINA_LIST_FOREACH(wd->item_providers, l, ip)
     {
        if ((ip->func == func) && ((!data) || (ip->data == data)))
          {
             wd->item_providers = eina_list_remove_list(wd->item_providers, l);
             free(ip);
             return;
          }
     }
}

EAPI void
elm_entry_markup_filter_append(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data)
{
   Widget_Data *wd;
   Elm_Entry_Markup_Filter *tf;
   ELM_CHECK_WIDTYPE(obj, widtype);

   wd = elm_widget_data_get(obj);

   EINA_SAFETY_ON_NULL_RETURN(func);

   tf = _filter_new(func, data);
   if (!tf) return;

   wd->markup_filters = eina_list_append(wd->markup_filters, tf);
}

EAPI void
elm_entry_markup_filter_prepend(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data)
{
   Widget_Data *wd;
   Elm_Entry_Markup_Filter *tf;
   ELM_CHECK_WIDTYPE(obj, widtype);

   wd = elm_widget_data_get(obj);

   EINA_SAFETY_ON_NULL_RETURN(func);

   tf = _filter_new(func, data);
   if (!tf) return;

   wd->markup_filters = eina_list_prepend(wd->markup_filters, tf);
}

EAPI void
elm_entry_markup_filter_remove(Evas_Object *obj, Elm_Entry_Filter_Cb func, void *data)
{
   Widget_Data *wd;
   Eina_List *l;
   Elm_Entry_Markup_Filter *tf;
   ELM_CHECK_WIDTYPE(obj, widtype);

   wd = elm_widget_data_get(obj);

   EINA_SAFETY_ON_NULL_RETURN(func);

   EINA_LIST_FOREACH(wd->markup_filters, l, tf)
     {
        if ((tf->func == func) && ((!data) || (tf->data == data)))
          {
             wd->markup_filters = eina_list_remove_list(wd->markup_filters, l);
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
elm_entry_filter_limit_size(void *data, Evas_Object *entry, char **text)
{
   Elm_Entry_Filter_Limit_Size *lim = data;
   char *current, *utfstr;
   int len, newlen;
   const char *(*text_get)(const Evas_Object *);

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
             evas_object_smart_callback_call(entry, "maxlength,reached", NULL);
             free(*text);
             *text = NULL;
             free(current);
             free(utfstr);
             return;
          }
        if ((len + newlen) > lim->max_char_count)
          _add_chars_till_limit(entry, text, (lim->max_char_count - len), LENGTH_UNIT_CHAR);
     }
   else if (lim->max_byte_count > 0)
     {
        len = strlen(current);
        newlen = strlen(utfstr);
        if ((len >= lim->max_byte_count) && (newlen > 0))
          {
             evas_object_smart_callback_call(entry, "maxlength,reached", NULL);
             free(*text);
             *text = NULL;
             free(current);
             free(utfstr);
             return;
          }
        if ((len + newlen) > lim->max_byte_count)
          _add_chars_till_limit(entry, text, (lim->max_byte_count - len), LENGTH_UNIT_BYTE);
     }
   free(current);
   free(utfstr);
}

EAPI void
elm_entry_filter_accept_set(void *data, Evas_Object *entry __UNUSED__, char **text)
{
   Elm_Entry_Filter_Accept_Set *as = data;
   const char *set;
   char *insert;
   Eina_Bool goes_in;
   int read_idx, last_read_idx = 0, read_char;

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
elm_entry_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   if (wd->delay_write)
     {
        ecore_timer_del(wd->delay_write);
        wd->delay_write = NULL;
     }
   if (wd->autosave) _save(obj);
   eina_stringshare_replace(&wd->file, file);
   wd->format = format;
   return _load(obj);
}

EAPI void
elm_entry_file_get(const Evas_Object *obj, const char **file, Elm_Text_Format *format)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (file) *file = wd->file;
   if (format) *format = wd->format;
}

EAPI void
elm_entry_file_save(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->delay_write)
     {
        ecore_timer_del(wd->delay_write);
        wd->delay_write = NULL;
     }
   _save(obj);
   wd->delay_write = ecore_timer_add(2.0, _delay_write, obj);
}

EAPI void
elm_entry_autosave_set(Evas_Object *obj, Eina_Bool autosave)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->autosave = !!autosave;
}

EAPI Eina_Bool
elm_entry_autosave_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->autosave;
}

EINA_DEPRECATED EAPI void
elm_entry_cnp_textonly_set(Evas_Object *obj, Eina_Bool textonly)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Elm_Cnp_Mode cnp_mode = ELM_CNP_MODE_MARKUP;
   if (textonly)
     cnp_mode = ELM_CNP_MODE_NO_IMAGE;
   elm_entry_cnp_mode_set(obj, cnp_mode);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_entry_cnp_textonly_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   return elm_entry_cnp_mode_get(obj) != ELM_CNP_MODE_MARKUP;
}

EAPI void
elm_entry_cnp_mode_set(Evas_Object *obj, Elm_Cnp_Mode cnp_mode)
{
   Elm_Sel_Format format = ELM_SEL_FORMAT_MARKUP;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->cnp_mode == cnp_mode) return;
   wd->cnp_mode = cnp_mode;
   if (wd->cnp_mode == ELM_CNP_MODE_PLAINTEXT)
     format = ELM_SEL_FORMAT_TEXT;
   else if (cnp_mode == ELM_CNP_MODE_MARKUP) format |= ELM_SEL_FORMAT_IMAGE;
#ifdef HAVE_ELEMENTARY_X
   elm_drop_target_add(obj, format, _drag_drop_cb, NULL);
#endif
}

EAPI Elm_Cnp_Mode
elm_entry_cnp_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_CNP_MODE_MARKUP;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_CNP_MODE_MARKUP;
   return wd->cnp_mode;
}

EAPI void
elm_entry_scrollable_set(Evas_Object *obj, Eina_Bool scroll)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   const Elm_Scroller_Policy map[3] =
     {
        ELM_SMART_SCROLLER_POLICY_AUTO,
        ELM_SMART_SCROLLER_POLICY_ON,
        ELM_SMART_SCROLLER_POLICY_OFF
     };
   if (!wd) return;
   scroll = !!scroll;
   if (wd->scroll == scroll) return;
   wd->scroll = scroll;
   if (wd->scroll)
     {
        if (!wd->scroller)
          {
             wd->scroller = elm_smart_scroller_add(evas_object_evas_get(obj));
             elm_widget_resize_object_set(obj, wd->scroller);
             elm_smart_scroller_widget_set(wd->scroller, obj);
             elm_smart_scroller_object_theme_set(obj, wd->scroller, "scroller", "entry",
                                                 elm_widget_style_get(obj));
             evas_object_size_hint_weight_set(wd->scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set(wd->scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
             evas_object_propagate_events_set(wd->scroller, EINA_TRUE);
             evas_object_propagate_events_set(elm_smart_scroller_edje_object_get(wd->scroller),
                                              EINA_TRUE);
          }
        elm_smart_scroller_bounce_allow_set(wd->scroller, wd->h_bounce, wd->v_bounce);
        if (wd->single_line)
          elm_smart_scroller_policy_set(wd->scroller, ELM_SMART_SCROLLER_POLICY_OFF,
                                        ELM_SMART_SCROLLER_POLICY_OFF);
        else
          elm_smart_scroller_policy_set(wd->scroller, map[wd->policy_h], map[wd->policy_v]);
        elm_widget_sub_object_add(obj, wd->ent);
        elm_smart_scroller_child_set(wd->scroller, wd->ent);
        evas_object_show(wd->scroller);
        elm_widget_on_show_region_hook_set(obj, _show_region_hook, obj);
     }
   else
     {
        if (wd->scroller)
          {
             elm_smart_scroller_child_set(wd->scroller, NULL);
             evas_object_smart_member_add(wd->scroller, obj);
             elm_widget_sub_object_add(obj, wd->scroller);
             evas_object_hide(wd->scroller);
          }
        elm_widget_sub_object_del(obj, wd->ent);
        elm_widget_resize_object_set(obj, wd->ent);
        elm_widget_on_show_region_hook_set(obj, NULL, NULL);
     }
   wd->lastw = -1;
   _theme_hook(obj);
}

EAPI Eina_Bool
elm_entry_scrollable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->scroll;
}

EAPI void
elm_entry_icon_visible_set(Evas_Object *obj, Eina_Bool setting)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *edje;
   if (!wd) return;
   if (wd->scroll)
      edje = elm_smart_scroller_edje_object_get(wd->scroller);
   else
      edje = wd->ent;

   if ((!edje) || (!edje_object_part_swallow_get(edje, "elm.swallow.icon"))) return;
   if (setting)
     edje_object_signal_emit(edje, "elm,action,show,icon", "elm");
   else
     edje_object_signal_emit(edje, "elm,action,hide,icon", "elm");
   _sizing_eval(obj);
}

EAPI void
elm_entry_end_visible_set(Evas_Object *obj, Eina_Bool setting)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *edje;
   if (!wd) return;
   if (wd->scroll)
      edje = elm_smart_scroller_edje_object_get(wd->scroller);
   else
      edje = wd->ent;

   if ((!edje) || (!edje_object_part_swallow_get(edje, "elm.swallow.icon"))) return;
   if (setting)
     edje_object_signal_emit(edje, "elm,action,show,end", "elm");
   else
     edje_object_signal_emit(edje, "elm,action,hide,end", "elm");
   _sizing_eval(obj);
}

EAPI void
elm_entry_scrollbar_policy_set(Evas_Object *obj, Elm_Scroller_Policy h, Elm_Scroller_Policy v)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   const Elm_Scroller_Policy map[3] =
     {
        ELM_SMART_SCROLLER_POLICY_AUTO,
        ELM_SMART_SCROLLER_POLICY_ON,
        ELM_SMART_SCROLLER_POLICY_OFF
     };
   if (!wd) return;
   wd->policy_h = h;
   wd->policy_v = v;
   elm_smart_scroller_policy_set(wd->scroller,
                                 map[wd->policy_h],
                                 map[wd->policy_v]);
}

EAPI void
elm_entry_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->h_bounce = h_bounce;
   wd->v_bounce = v_bounce;
   elm_smart_scroller_bounce_allow_set(wd->scroller, h_bounce, v_bounce);
}

EAPI void
elm_entry_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_bounce_allow_get(wd->scroller, h_bounce, v_bounce);
}

EAPI void
elm_entry_input_panel_layout_set(Evas_Object *obj, Elm_Input_Panel_Layout layout)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->input_panel_layout = layout;

   edje_object_part_text_input_panel_layout_set(wd->ent, "elm.text", layout);
}

EAPI Elm_Input_Panel_Layout
elm_entry_input_panel_layout_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_INPUT_PANEL_LAYOUT_INVALID;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_INPUT_PANEL_LAYOUT_INVALID;

   return wd->input_panel_layout;
}

EAPI void
elm_entry_autocapital_type_set(Evas_Object *obj, Elm_Autocapital_Type autocapital_type)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->autocapital_type = autocapital_type;
   edje_object_part_text_autocapital_type_set(wd->ent, "elm.text", autocapital_type);
}

EAPI Elm_Autocapital_Type
elm_entry_autocapital_type_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_AUTOCAPITAL_TYPE_NONE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_AUTOCAPITAL_TYPE_NONE;

   return wd->autocapital_type;
}

EAPI void
elm_entry_prediction_allow_set(Evas_Object *obj, Eina_Bool prediction)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->prediction_allow = prediction;
   edje_object_part_text_prediction_allow_set(wd->ent, "elm.text", prediction);
}

EAPI Eina_Bool
elm_entry_prediction_allow_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_TRUE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_TRUE;

   return wd->prediction_allow;
}

EAPI void
elm_entry_imf_context_reset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   edje_object_part_text_imf_context_reset(wd->ent, "elm.text");
}

EAPI void
elm_entry_input_panel_enabled_set(Evas_Object *obj, Eina_Bool enabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->input_panel_enable = enabled;
   edje_object_part_text_input_panel_enabled_set(wd->ent, "elm.text", enabled);
}

EAPI Eina_Bool
elm_entry_input_panel_enabled_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_TRUE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_TRUE;

   return wd->input_panel_enable;
}

EAPI void
elm_entry_input_panel_show(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   edje_object_part_text_input_panel_show(wd->ent, "elm.text");
}

EAPI void
elm_entry_input_panel_hide(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   edje_object_part_text_input_panel_hide(wd->ent, "elm.text");
}

EAPI void
elm_entry_input_panel_language_set(Evas_Object *obj, Elm_Input_Panel_Lang lang)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->input_panel_lang = lang;
   edje_object_part_text_input_panel_language_set(wd->ent, "elm.text", lang);
}

EAPI Elm_Input_Panel_Lang
elm_entry_input_panel_language_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_INPUT_PANEL_LANG_AUTOMATIC;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_INPUT_PANEL_LANG_AUTOMATIC;

   return wd->input_panel_lang;
}

EAPI void
elm_entry_input_panel_imdata_set(Evas_Object *obj, const void *data, int len)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->input_panel_imdata)
     free(wd->input_panel_imdata);

   wd->input_panel_imdata = calloc(1, len);
   wd->input_panel_imdata_len = len;
   memcpy(wd->input_panel_imdata, data, len);

   edje_object_part_text_input_panel_imdata_set(wd->ent, "elm.text", wd->input_panel_imdata, wd->input_panel_imdata_len);
}

EAPI void
elm_entry_input_panel_imdata_get(const Evas_Object *obj, void *data, int *len)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   edje_object_part_text_input_panel_imdata_get(wd->ent, "elm.text", data, len);
}

EAPI void
elm_entry_input_panel_return_key_type_set(Evas_Object *obj, Elm_Input_Panel_Return_Key_Type return_key_type)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->input_panel_return_key_type = return_key_type;

   edje_object_part_text_input_panel_return_key_type_set(wd->ent, "elm.text", return_key_type);
}

EAPI Elm_Input_Panel_Return_Key_Type
elm_entry_input_panel_return_key_type_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;

   return wd->input_panel_return_key_type;
}

EAPI void
elm_entry_input_panel_return_key_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->input_panel_return_key_disabled = disabled;

   edje_object_part_text_input_panel_return_key_disabled_set(wd->ent, "elm.text", disabled);
}

EAPI Eina_Bool
elm_entry_input_panel_return_key_disabled_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   return wd->input_panel_return_key_disabled;
}

EAPI void
elm_entry_input_panel_return_key_autoenabled_set(Evas_Object *obj, Eina_Bool enabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->autoreturnkey = enabled;
   _check_enable_return_key(obj);
}

EAPI void*
elm_entry_imf_context_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   return edje_object_part_text_imf_context_get(wd->ent, "elm.text");
}

/* START - ANCHOR HOVER */
static void
_parent_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->anchor_hover.hover_parent = NULL;
}

static void
_anchor_hover_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (wd->anchor_hover.pop) evas_object_del(wd->anchor_hover.pop);
   wd->anchor_hover.pop = NULL;
   evas_object_event_callback_del_full(wd->anchor_hover.hover, EVAS_CALLBACK_DEL,
                                       _anchor_hover_del, obj);
}

EAPI void
elm_entry_anchor_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->anchor_hover.hover_parent)
     evas_object_event_callback_del_full(wd->anchor_hover.hover_parent, EVAS_CALLBACK_DEL, _parent_del, obj);
   wd->anchor_hover.hover_parent = parent;
   if (wd->anchor_hover.hover_parent)
     evas_object_event_callback_add(wd->anchor_hover.hover_parent, EVAS_CALLBACK_DEL, _parent_del, obj);
}

EAPI Evas_Object *
elm_entry_anchor_hover_parent_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->anchor_hover.hover_parent;
}

EAPI void
elm_entry_anchor_hover_style_set(Evas_Object *obj, const char *style)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->anchor_hover.hover_style, style);
}

EAPI const char *
elm_entry_anchor_hover_style_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->anchor_hover.hover_style;
}

EAPI void
elm_entry_anchor_hover_end(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->anchor_hover.hover) evas_object_del(wd->anchor_hover.hover);
   if (wd->anchor_hover.pop) evas_object_del(wd->anchor_hover.pop);
   wd->anchor_hover.hover = NULL;
   wd->anchor_hover.pop = NULL;
}


static void
_anchor_hover_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_entry_anchor_hover_end(data);
}

static void
_entry_hover_anchor_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Entry_Anchor_Info *info = event_info;
   Evas_Object *hover_parent;
   Elm_Entry_Anchor_Hover_Info ei;
   Evas_Coord x, w, y, h, px, py;
   if (!wd) return;

   ei.anchor_info = event_info;

   wd->anchor_hover.pop = elm_icon_add(obj);
   evas_object_move(wd->anchor_hover.pop, info->x, info->y);
   evas_object_resize(wd->anchor_hover.pop, info->w, info->h);

   wd->anchor_hover.hover = elm_hover_add(obj);
   evas_object_event_callback_add(wd->anchor_hover.hover, EVAS_CALLBACK_DEL,
                                  _anchor_hover_del, obj);
   elm_widget_mirrored_set(wd->anchor_hover.hover, elm_widget_mirrored_get(obj));
   if (wd->anchor_hover.hover_style)
     elm_object_style_set(wd->anchor_hover.hover, wd->anchor_hover.hover_style);

   hover_parent = wd->anchor_hover.hover_parent;
   if (!hover_parent) hover_parent = obj;
   elm_hover_parent_set(wd->anchor_hover.hover, hover_parent);
   elm_hover_target_set(wd->anchor_hover.hover, wd->anchor_hover.pop);
   ei.hover = wd->anchor_hover.hover;

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

   if (elm_widget_mirrored_get(wd->anchor_hover.hover))
     {  /* Swap right and left because they switch sides in RTL */
        Eina_Bool tmp = ei.hover_left;
        ei.hover_left = ei.hover_right;
        ei.hover_right = tmp;
     }

   evas_object_smart_callback_call(data, SIG_ANCHOR_HOVER_OPENED, &ei);
   evas_object_smart_callback_add(wd->anchor_hover.hover, "clicked", _anchor_hover_clicked, data);

   /* FIXME: Should just check if there's any callback registered to the smart
    * events instead.
    * This is used to determine if anyone cares about the hover or not. */
   if (!elm_object_part_content_get(wd->anchor_hover.hover, "middle") &&
         !elm_object_part_content_get(wd->anchor_hover.hover, "left") &&
         !elm_object_part_content_get(wd->anchor_hover.hover, "right") &&
         !elm_object_part_content_get(wd->anchor_hover.hover, "top") &&
         !elm_object_part_content_get(wd->anchor_hover.hover, "bottom"))
     {
        evas_object_del(wd->anchor_hover.hover);
        wd->anchor_hover.hover = NULL;
     }
   else
     evas_object_show(wd->anchor_hover.hover);
}
/* END - ANCHOR HOVER */
