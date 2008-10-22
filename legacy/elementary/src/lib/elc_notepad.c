#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scroller, *entry;
   const char *file;
   Elm_Text_Format format;
   Ecore_Timer *delay_write;
   Evas_Bool can_write : 1;
   Evas_Bool auto_write : 1;
};

static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _load(Evas_Object *obj);
static void _save(Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->file) evas_stringshare_del(wd->file);
   if (wd->delay_write)
     {
	ecore_timer_del(wd->delay_write);
	_save(obj);
     }
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_on_focus_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_widget_focus_set(wd->entry, 1);
}

static char *
_buf_append(char *buf, const char *str, int *len, int *alloc)
{
   int len2;
   
   len2 = strlen(str);
   if ((*len + len2) >= *alloc)
     {
	char *buf2;
	
	buf2 = realloc(buf, *alloc + 512);
	if (!buf2) return NULL;
	buf = buf2;
	*alloc += 512;
     }
   strcpy(buf + *len, str);
   *len += len2;
   return buf;
}

static char *
_load_markup_utf8(const char *file)
{
   FILE *f;
   size_t size;
   int alloc = 0, len = 0, pos;
   char *text = NULL, buf[4096];
   
   f = fopen(file, "r");
   if (!f) return NULL;
   while (size = fread(buf, 1, sizeof(buf), f))
     {
	pos = 0;
	buf[size] = 0;
	while (pos < size)
	  {
	     int ch;
	     char str[2];
	     
	     ch = buf[pos];
	     if (ch != '\n')
	       {
		  str[0] = ch;
		  str[1] = 0;
		  text = _buf_append(text, str, &len, &alloc);
	       }
	     pos++;
	  }
     }
   fclose(f);
   return text;
}

static char *
_load_plain_utf8(const char *file)
{
   FILE *f;
   size_t size;
   int alloc = 0, len = 0, pos;
   char *text = NULL, buf[4096];
   
   f = fopen(file, "r");
   if (!f) return NULL;
   while (size = fread(buf, 1, sizeof(buf), f))
     {
	pos = 0;
	buf[size] = 0;
	while (pos < size)
	  {
	     int ch, ppos;
	     
	     ppos = pos;
	     ch = evas_common_font_utf8_get_next(buf, &pos);
	     if (ch == '\n')
	       text = _buf_append(text, "<br>", &len, &alloc);
	     else
	       {
		  int stlen = 0;
		  const char *escape;
		  char str[16];
		  
		  escape = evas_textblock_string_escape_get(buf + ppos, &stlen);
		  if (escape)
		    text = _buf_append(text, escape, &len, &alloc);
		  else
		    {
		       strncpy(str, buf + ppos, pos - ppos);
		       str[pos - ppos] = 0;
		       text = _buf_append(text, str, &len, &alloc);
		    }
	       }
	  }
     }
   fclose(f);
   return text;
}

static void
_load(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char *text;
   
   if (!wd->file)
     {
	elm_entry_entry_set(wd->entry, "");
	return;
     }
   switch (wd->format)
     {
      case ELM_TEXT_FORMAT_PLAIN_UTF8:
	text = _load_plain_utf8(wd->file);
	break;
      case ELM_TEXT_FORMAT_MARKUP_UTF8:
	text = _load_markup_utf8(wd->file);
	break;
      default:
	elm_entry_entry_set(wd->entry, "Unknown Text Format");
	break;
     }
   if (text)
     {
	elm_entry_entry_set(wd->entry, text);
	free(text);
     }
   else
     elm_entry_entry_set(wd->entry, "");
}

static void
_save_markup_utf8(const char *file, const char *text)
{
   FILE *f;

   if ((!text) || (text[0] == 0))
     {
	ecore_file_unlink(file);
	return;
     }
   f = fopen(file, "w");
   if (!f)
     {
	// FIXME: report a write error
	return;
     }
   fputs(text, f); // FIXME: catch error
   fputs("\n", f); // FIXME: catch error
   fclose(f);
}

static void
_save_plain_utf8(const char *file, const char *text)
{
   FILE *f;
   char *s, *p;
   char *tag_start, *tag_end, *esc_start, *esc_end;

   if ((!text) || (text[0] == 0))
     {
	ecore_file_unlink(file);
	return;
     }
   f = fopen(file, "w");
   if (!f)
     {
	// FIXME: report a write error
	return;
     }
   tag_start = tag_end = esc_start = esc_end = NULL;
   p = (char *)text;
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
		       if (!strcmp(ttag, "br")) fputs("\n", f); // FIXME: catch error
		       free(ttag);
		    }
		  tag_start = tag_end = NULL;
	       }
	     else if (esc_end)
	       {
		  char tesc[256];
		  char *str;
		  
		  if ((esc_end - esc_start) < (sizeof(tesc) - 2))
		    {
		       strncpy(tesc, esc_start, esc_end - esc_start + 1);
		       tesc[esc_end - esc_start + 1] = 0;
		    }
		  str = evas_textblock_escape_string_get(tesc);
		  if (str) fputs(str, f); // FIXME: catch error
		  esc_start = esc_end = NULL;
	       }
	     else if (*p == 0)
	       {
		  fwrite(s, p - s, 1, f); // FIXME: catch error
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
		  fwrite(s, p - s, 1, f); // FIXME: catch error
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
		  fwrite(s, p - s, 1, f); // FIXME: catch error
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
   fclose(f);
}

static void
_save(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd->file) return;
   switch (wd->format)
     {
      case ELM_TEXT_FORMAT_PLAIN_UTF8:
        _save_plain_utf8(wd->file, elm_entry_entry_get(wd->entry));
	break;
      case ELM_TEXT_FORMAT_MARKUP_UTF8:
	_save_markup_utf8(wd->file, elm_entry_entry_get(wd->entry));
	break;
      default:
	break;
     }
}

static int
_delay_write(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   _save(data);
   wd->delay_write = NULL;
   return 0;
}

static void
_entry_changed(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (wd->delay_write)
     {
	ecore_timer_del(wd->delay_write);
	wd->delay_write = NULL;
     }
   if (!wd->auto_write) return;
   wd->delay_write = ecore_timer_add(2.0, _delay_write, data);
}
    
EAPI Evas_Object *
elm_notepad_add(Evas_Object *parent)
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
   elm_widget_can_focus_set(obj, 0);
   
   wd->scroller = elm_scroller_add(parent);
   elm_widget_resize_object_set(obj, wd->scroller);
   wd->entry = elm_entry_add(parent);
   evas_object_size_hint_weight_set(wd->entry, 1.0, 1.0);
   evas_object_size_hint_align_set(wd->entry, -1.0, -1.0);
   elm_scroller_content_set(wd->scroller, wd->entry);
   evas_object_show(wd->entry);

   elm_entry_entry_set(wd->entry, "");
   evas_object_smart_callback_add(wd->entry, "changed", _entry_changed, obj);
   
   wd->auto_write = 1;
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_notepad_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->delay_write)
     {
	ecore_timer_del(wd->delay_write);
	wd->delay_write = NULL;
     }
   _save(obj);
   if (wd->file) evas_stringshare_del(wd->file);
   wd->file = NULL;
   if (file) wd->file = evas_stringshare_add(file);
   wd->format = format;
   _load(obj);
}
