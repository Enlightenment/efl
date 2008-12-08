#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *ent;
   Ecore_Job *deferred_recalc_job;
   Ecore_Event_Handler *sel_notify_handler; 
   Ecore_Event_Handler *sel_clear_handler;
   const char *cut_sel;
   Evas_Coord lastw;
   Evas_Bool changed : 1;
   Evas_Bool linewrap : 1;
   Evas_Bool single_line : 1;
   Evas_Bool password : 1;
   Evas_Bool editable : 1;
   Evas_Bool selection_asked : 1;
   Evas_Bool have_selection : 1;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _on_focus_hook(void *data, Evas_Object *obj);
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

static Eina_List *entries = NULL;

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   entries = eina_list_remove(entries, obj);
   ecore_event_handler_del(wd->sel_notify_handler);
   ecore_event_handler_del(wd->sel_clear_handler);
   if (wd->cut_sel) eina_stringshare_del(wd->cut_sel);
   if (wd->deferred_recalc_job) ecore_job_del(wd->deferred_recalc_job);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char *t;
   t = (char *)elm_entry_entry_get(obj);
   if (t) t = strdup(t);
   _elm_theme_set(wd->ent, "entry", _getbase(obj), "default");
   elm_entry_entry_set(obj, t);
   if (t) free(t);
   _sizing_eval(obj);
}

static void
_elm_win_recalc_job(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord resw, resh, minminw;
   
   wd->deferred_recalc_job = NULL;
   evas_object_geometry_get(wd->ent, NULL, NULL, &resw, &resh);
   resh = 0;
   minminw = 0;
   edje_object_size_min_restricted_calc(wd->ent, &minw, &minh, 0, 0);
   minminw = minw;
   edje_object_size_min_restricted_calc(wd->ent, &minw, &minh, resw, 0);
   evas_object_size_hint_min_set(data, minminw, minh);
   evas_object_size_hint_max_set(data, minminw, maxh);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord resw, resh, minminw;

   if (wd->linewrap)
     {
	evas_object_geometry_get(wd->ent, NULL, NULL, &resw, &resh);
	if ((resw == wd->lastw) && (!wd->changed)) return;
	wd->changed = 0;
	wd->lastw = resw;
	if (wd->deferred_recalc_job) ecore_job_del(wd->deferred_recalc_job);
	wd->deferred_recalc_job = ecore_job_add(_elm_win_recalc_job, obj);
     }
   else
     {
	evas_object_geometry_get(wd->ent, NULL, NULL, &resw, &resh);
	edje_object_size_min_calc(wd->ent, &minw, &minh);
	evas_object_size_hint_min_set(obj, minw, minh);
	evas_object_size_hint_max_set(obj, maxw, maxh);
     }
}

static void
_on_focus_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *top = elm_widget_top_get(obj);
   if (!wd->editable) return;
   if (elm_widget_focus_get(obj))
     {
        evas_object_focus_set(wd->ent, 1);
        edje_object_signal_emit(wd->ent, "elm,action,focus", "elm");
        elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_ON);
     }
   else
     {
        edje_object_signal_emit(wd->ent, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->ent, 0);
        elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_OFF);
     }
}

static void
_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (wd->linewrap) _sizing_eval(data);
   Evas_Coord ww, hh;
   evas_object_geometry_get(wd->ent, NULL, NULL, &ww, &hh);
}

static const char *
_getbase(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->editable)
     {
	if (wd->password) return "base-password";
	else
	  {
	     if (wd->single_line) return "base-single";
	     else
	       {
		  if (wd->linewrap) return "base";
		  else  return "base-nowrap";
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
		  if (wd->linewrap) return "base-noedit";
		  else  return "base-nowrap-noedit";
	       }
	  }
     }
   return "base";
}

static char *
_str_append(char *str, const char *txt, int *len, int *alloc)
{
   int txt_len = strlen(txt);
   if (txt_len <= 0) return str;
   if ((*len + txt_len) >= *alloc)
     {
        char *str2;
        int alloc2;
        
        alloc2 = *alloc + txt_len + 128;
        str2 = realloc(str, alloc2);
        if (!str2) return str;
        *alloc = alloc2;
        str = str2;
     }
   strcpy(str + *len, txt);
   *len += txt_len;
   return str;
}

static char *
_mkup_to_text(const char *mkup)
{
   char *str = NULL;
   int str_len = 0, str_alloc = 0;
   // FIXME: markup -> text
   char *s, *p;
   char *tag_start, *tag_end, *esc_start, *esc_end, *ts;
   
   tag_start = tag_end = esc_start = esc_end = NULL;
   p = (char *)mkup;
   s = p;
   for (;;)
     {
        if ((*p == 0) ||
            (tag_end) || (esc_end) ||
            (tag_start) || (esc_start))
          {
             if (tag_end)
               {
                  char *ttag, *match;
                  
                  ttag = malloc(tag_end - tag_start);
                  if (ttag)
                    {
                       strncpy(ttag, tag_start + 1, tag_end - tag_start - 1);
                       ttag[tag_end - tag_start - 1] = 0;
                       if (!strcmp(ttag, "br"))
                         str = _str_append(str, "\n", &str_len, &str_alloc);
                       else if (!strcmp(ttag, "\n"))
                         str = _str_append(str, "\n", &str_len, &str_alloc);
                       else if (!strcmp(ttag, "\\n"))
                         str = _str_append(str, "\n", &str_len, &str_alloc);
                       else if (!strcmp(ttag, "\t"))
                         str = _str_append(str, "\t", &str_len, &str_alloc);
                       else if (!strcmp(ttag, "\\t"))
                         str = _str_append(str, "\t", &str_len, &str_alloc);
                       free(ttag);
                    }
                  tag_start = tag_end = NULL;
               }
             else if (esc_end)
               {
                  ts = malloc(esc_end - esc_start + 1);
                  if (ts)
                    {
                       const char *esc;
                       strncpy(ts, esc_start, esc_end - esc_start); 
                       ts[esc_end - esc_start] = 0;
                       esc = evas_textblock_escape_string_get(ts);
                       if (esc)
                         str = _str_append(str, esc, &str_len, &str_alloc);
                       free(ts);
                    }
                  esc_start = esc_end = NULL;
               }
             else if (*p == 0)
               {
                  ts = malloc(p - s + 1);
                  if (ts)
                    {
                       strncpy(ts, s, p - s);
                       ts[p - s] = 0;
                       str = _str_append(str, ts, &str_len, &str_alloc);
                       free(ts);
                    }
                  s = NULL;
               }
             if (*p == 0)
               break;
          }
        if (*p == '<')
          {
             if (!esc_start)
               {
                  tag_start = p;
                  tag_end = NULL;
                  ts = malloc(p - s + 1);
                  if (ts)
                    {
                       strncpy(ts, s, p - s);
                       ts[p - s] = 0;
                       str = _str_append(str, ts, &str_len, &str_alloc);
                       free(ts);
                    }
                  s = NULL;
               }
          }
        else if (*p == '>')
          {
             if (tag_start)
               {
                  tag_end = p;
                  s = p + 1;
               }
          }
        else if (*p == '&')
          {
             if (!tag_start)
               {
                  esc_start = p;
                  esc_end = NULL;
                  ts = malloc(p - s + 1);
                  if (ts)
                    {
                       strncpy(ts, s, p - s);
                       ts[p - s] = 0;
                       str = _str_append(str, ts, &str_len, &str_alloc);
                       free(ts);
                    }
                  s = NULL;
               }
          }
        else if (*p == ';')
          {
             if (esc_start)
               {
                  esc_end = p;
                  s = p + 1;
               }
          }
        p++;
     }
   return str;
}

static char *
_text_to_mkup(const char *text)
{
   char *str = NULL;
   int str_len = 0, str_alloc = 0;
   int ch, pos = 0, pos2 = 0;
   
   for (;;)
     {
        // FIXME: use evas_string_char_next_get()
        pos = pos2;
        ch = evas_common_font_utf8_get_next((unsigned char *)(text), &pos2);
        if (ch <= 0) break;
        if (ch == '\n') str = _str_append(str, "<br>", &str_len, &str_alloc);
        else if (ch == '\t') str = _str_append(str, "<\t>", &str_len, &str_alloc);
        else
          {
             char tstr[16];
             
             strncpy(tstr, text + pos, pos2 - pos);
             tstr[pos2 - pos] = 0;
             str = _str_append(str, tstr, &str_len, &str_alloc);
          }
     }
   return str;
}

static void
_signal_entry_changed(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   evas_object_smart_callback_call(data, "changed", NULL);
   wd->changed = 1;
   _sizing_eval(data);
}

static void
_signal_selection_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Eina_List *l;
   for (l = entries; l; l = l->next)
     {
        if (l->data != data) elm_entry_select_none(l->data);
     }
   wd->have_selection = 1;
   evas_object_smart_callback_call(data, "selection,start", NULL);
   if (wd->sel_notify_handler)
     {
        char *txt = _mkup_to_text(elm_entry_selection_get(data));
        if (txt)
          {
             ecore_x_selection_primary_set
               (elm_win_xwindow_get(elm_widget_top_get(data)),
                txt, strlen(txt));
             free(txt);
          }
     } 
}

static void
_signal_selection_changed(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->have_selection = 1;
   evas_object_smart_callback_call(data, "selection,changed", NULL);
   if (wd->sel_notify_handler)
     {
        char *txt = _mkup_to_text(elm_entry_selection_get(data));
        if (txt)
          {
             ecore_x_selection_primary_set
               (elm_win_xwindow_get(elm_widget_top_get(data)),
                txt, strlen(txt));
             free(txt);
          }
     }
}

static void
_signal_selection_cleared(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd->have_selection) return;
   wd->have_selection = 0;
   evas_object_smart_callback_call(data, "selection,cleared", NULL);
   if (wd->sel_notify_handler)
     {
        if (wd->cut_sel)
          {
             ecore_x_selection_primary_set
               (elm_win_xwindow_get(elm_widget_top_get(data)),
                wd->cut_sel, strlen(wd->cut_sel));
             eina_stringshare_del(wd->cut_sel);
             wd->cut_sel = NULL;
          }
        else
          ecore_x_selection_primary_clear();
     }
}

static void
_signal_entry_paste_request(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   evas_object_smart_callback_call(data, "selection,paste", NULL);
   if (wd->sel_notify_handler)
     {
        ecore_x_selection_primary_request
          (elm_win_xwindow_get(elm_widget_top_get(data)),
           ECORE_X_SELECTION_TARGET_UTF8_STRING);
        wd->selection_asked = 1;
     }
}

static void
_signal_entry_copy_notify(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   evas_object_smart_callback_call(data, "selection,copy", NULL);
}

static void
_signal_entry_cut_notify(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   char *txt;
   evas_object_smart_callback_call(data, "selection,cut", NULL);
   if (wd->cut_sel) eina_stringshare_del(wd->cut_sel);
   wd->cut_sel = NULL;
   txt = _mkup_to_text(elm_entry_selection_get(data));
   if (txt)
     {
        wd->cut_sel = eina_stringshare_add(txt);
        free(txt);
     }
   edje_object_part_text_insert(wd->ent, "elm.text", "");
   wd->changed = 1;
   _sizing_eval(data);
}

static void
_signal_cursor_changed(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord cx, cy, cw, ch;
   evas_object_smart_callback_call(data, "cursor,changed", NULL);
   edje_object_part_text_cursor_geometry_get(wd->ent, "elm.text", &cx, &cy, &cw, &ch);
   elm_widget_show_region_set(data, cx, cy, cw, ch);
}

static void
_signal_anchor_down(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
}

static void
_signal_anchor_up(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Entry_Anchor_Info ei;
   char *buf, *buf2, *p, *p2, *n;
   int buflen;
   p = strrchr(emission, ',');
   if (p)
     {
	Eina_List *geoms;
	
	n = p + 1;
	p2 = p -1;
	while (p2 >= emission)
	  {
	     if (*p2 == ',') break;
	     p2--;
	  }
	p2++;
	buf2 = alloca(5 + p - p2);
	strncpy(buf2, p2, p - p2);
	buf2[p - p2] = 0;
	ei.name = n;
	ei.button = atoi(buf2);
	ei.x = ei.y = ei.w = ei.h = 0;
	geoms = edje_object_part_text_anchor_geometry_get(wd->ent, "elm.text", ei.name);
	if (geoms)
	  {
	     Evas_Textblock_Rectangle *r;
	     Eina_List *l;
	     Evas_Coord px, py, x, y;

	     evas_object_geometry_get(wd->ent, &x, &y, NULL, NULL);
	     evas_pointer_output_xy_get(evas_object_evas_get(wd->ent), &px, &py);
	     EINA_LIST_FOREACH(geoms, l, r)
	       {
		  if (((r->x + x) <= px) && ((r->y + y) <= py) && 
		      ((r->x + x + r->w) > px) && ((r->y + y + r->h) > py))
		    {
		       ei.x = r->x + x;
		       ei.y = r->y + y;
		       ei.w = r->w;
		       ei.h = r->h;
		       break;
		    }
	       }
	  }
	evas_object_smart_callback_call(data, "anchor,clicked", &ei);
     }
}

static void
_signal_anchor_move(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
}

static void
_signal_anchor_in(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
}

static void
_signal_anchor_out(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
}

static void
_signal_key_enter(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   evas_object_smart_callback_call(data, "activated", NULL);
}

static int
_event_selection_notify(void *data, int type, void *event)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Ecore_X_Event_Selection_Notify *ev = event;
   if (!wd->selection_asked) return 1;
   if ((ev->selection == ECORE_X_SELECTION_CLIPBOARD) ||
       (ev->selection == ECORE_X_SELECTION_PRIMARY))
     {
        Ecore_X_Selection_Data_Text *text_data;
        
        text_data = ev->data;
        if (text_data->data.content == ECORE_X_SELECTION_CONTENT_TEXT)
          {
             if (text_data->text)
               {
                  char *txt = _text_to_mkup(text_data->text);
                  if (txt)
                    {
                       elm_entry_entry_insert(data, txt);
                       free(txt);
                    }
               }
          }
        wd->selection_asked = 0;
     }
   return 1;
}
    
static int
_event_selection_clear(void *data, int type, void *event)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Ecore_X_Event_Selection_Clear *ev = event;
   if (!wd->have_selection) return 1;
   if ((ev->selection == ECORE_X_SELECTION_CLIPBOARD) ||
       (ev->selection == ECORE_X_SELECTION_PRIMARY))
     {
        elm_entry_select_none(data);
     }
   return 1;
}

EAPI Evas_Object *
elm_entry_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, 1);

   wd->linewrap = 1;
   wd->editable = 1;
   
   wd->ent = edje_object_add(e);
   evas_object_event_callback_add(wd->ent, EVAS_CALLBACK_RESIZE, _resize, obj);
					  
   _elm_theme_set(wd->ent, "entry", "base", "default");
   edje_object_signal_callback_add(wd->ent, "entry,changed", "elm.text", _signal_entry_changed, obj);
   edje_object_signal_callback_add(wd->ent, "selection,start", "elm.text", _signal_selection_start, obj);
   edje_object_signal_callback_add(wd->ent, "selection,changed", "elm.text", _signal_selection_changed, obj);
   edje_object_signal_callback_add(wd->ent, "selection,cleared", "elm.text", _signal_selection_cleared, obj);
   edje_object_signal_callback_add(wd->ent, "entry,paste,request", "elm.text", _signal_entry_paste_request, obj);
   edje_object_signal_callback_add(wd->ent, "entry,copy,notify", "elm.text", _signal_entry_copy_notify, obj);
   edje_object_signal_callback_add(wd->ent, "entry,cut,notify", "elm.text", _signal_entry_cut_notify, obj);
   edje_object_signal_callback_add(wd->ent, "cursor,changed", "elm.text", _signal_cursor_changed, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,down,*", "elm.text", _signal_anchor_down, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,up,*", "elm.text", _signal_anchor_up, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,move,*", "elm.text", _signal_anchor_move, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,in,*", "elm.text", _signal_anchor_in, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,out,*", "elm.text", _signal_anchor_out, obj);
   edje_object_signal_callback_add(wd->ent, "entry,key,enter", "elm.text", _signal_key_enter, obj);
   edje_object_part_text_set(wd->ent, "elm.text", "<br>");
   elm_widget_resize_object_set(obj, wd->ent);
   _sizing_eval(obj);

   if (elm_win_xwindow_get(elm_widget_top_get(parent)) != 0)
     {
        wd->sel_notify_handler = 
          ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY,
                                  _event_selection_notify, obj);
        wd->sel_clear_handler = 
          ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR,
                                  _event_selection_clear, obj);
     }
   
   entries = eina_list_prepend(entries, obj);
   return obj;
}

EAPI void
elm_entry_single_line_set(Evas_Object *obj, Evas_Bool single_line)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char *t;
   if (wd->single_line == single_line) return;
   wd->single_line = single_line;
   wd->linewrap = 0;
   t = elm_entry_entry_get(obj);
   if (t) t = strdup(t);
   _elm_theme_set(wd->ent, "entry", _getbase(obj), "default");
   elm_entry_entry_set(obj, t);
   if (t) free(t);
   _sizing_eval(obj);
}

EAPI void
elm_entry_password_set(Evas_Object *obj, Evas_Bool password)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char *t;
   if (wd->password == password) return;
   wd->password = password;
   wd->single_line = 1;
   wd->linewrap = 0;
   t = elm_entry_entry_get(obj);
   if (t) t = strdup(t);
   _elm_theme_set(wd->ent, "entry", _getbase(obj), "default");
   elm_entry_entry_set(obj, t);
   if (t) free(t);
   _sizing_eval(obj);
}

EAPI void
elm_entry_entry_set(Evas_Object *obj, const char *entry)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!entry) entry = "<br>";
   edje_object_part_text_set(wd->ent, "elm.text", entry);
   // debug
#if 0
     {
	Eina_List *l, *an;
	an = edje_object_part_text_anchor_list_get(wd->ent, "elm.text");
	for (l = an; l; l = l->next)
	  printf("ANCHOR: %s\n", l->data);
     }
#endif   
   wd->changed = 1;
   _sizing_eval(obj);
}

EAPI const char *
elm_entry_entry_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return edje_object_part_text_get(wd->ent, "elm.text");
}

EAPI const char *
elm_entry_selection_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return edje_object_part_text_selection_get(wd->ent, "elm.text");
}

EAPI void
elm_entry_entry_insert(Evas_Object *obj, const char *entry)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_part_text_insert(wd->ent, "elm.text", entry);
   wd->changed = 1;
   _sizing_eval(obj);
}

EAPI void
elm_entry_line_wrap_set(Evas_Object *obj, Evas_Bool wrap)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char *t;
   if (wd->linewrap == wrap) return;
   wd->linewrap = wrap;
   t = elm_entry_entry_get(obj);
   if (t) t = strdup(t);
   _elm_theme_set(wd->ent, "entry", _getbase(obj), "default");
   elm_entry_entry_set(obj, t);
   if (t) free(t);
   _sizing_eval(obj);
}

EAPI void
elm_entry_editable_set(Evas_Object *obj, Evas_Bool editable)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char *t;
   if (wd->editable == editable) return;
   wd->editable = editable;
   t = elm_entry_entry_get(obj);
   if (t) t = strdup(t);
   _elm_theme_set(wd->ent, "entry", _getbase(obj), "default");
   elm_entry_entry_set(obj, t);
   if (t) free(t);
   _sizing_eval(obj);
}

EAPI void
elm_entry_select_none(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->have_selection = 0;
   edje_object_part_text_select_none(wd->ent, "elm.text");
}

EAPI void
elm_entry_select_all(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->have_selection = 1;
   edje_object_part_text_select_all(wd->ent, "elm.text");
}
