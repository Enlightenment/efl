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
_load_markup_utf8(const char *file)
{
   FILE *f;
   long size;
   char *text = NULL;
   
   f = fopen(file, "r");
   if (!f) return NULL;
   fseek(f, 0, SEEK_END);
   size = ftell(f);
   if (size > 0)
     {
	rewind(f);
	text = malloc(size + 1);
	if (text)
	  {
	     if (fread(text, size, 1, f))
	       {
		  text[size] = 0;
	       }
	     else
	       {
		  free(text);
		  text = NULL;
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
_save(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd->file) return;
   switch (wd->format)
     {
      case ELM_TEXT_FORMAT_PLAIN_UTF8:
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
   evas_object_size_hint_weight_set(wd->entry, 1.0, 0.0);
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
