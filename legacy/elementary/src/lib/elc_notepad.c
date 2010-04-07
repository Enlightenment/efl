#include <Elementary.h>
#include "elm_priv.h"
/**
 * @defgroup Notepad Notepad
 *
 * The notepad is an object for quickly loading a text file, displaying it,
 * allowing editing of it and saving of changes back to the file loaded.
 *
 * Signals that you can add callbacks for are:
 *
 * NONE
 *
 * A notepad object contains a scroller and an entry. It is a convenience
 * widget that loads a text file indicated, puts it in the scrollable entry
 * and allows the user to edit it. Changes are written back to the original
 * file after a short delay. The file to load and save to is specified by
 * elm_notepad_file_set().
 */
typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scr, *entry;
   const char *file;
   Elm_Text_Format format;
   Ecore_Timer *delay_write;
   Eina_Bool can_write : 1;
   Eina_Bool auto_write : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _load(Evas_Object *obj);
static void _save(Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->file) eina_stringshare_del(wd->file);
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
   if (!wd) return;
   evas_object_size_hint_min_set(obj, -1, -1);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj)) elm_widget_focus_steal(wd->entry);
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
	*alloc += 512 + len2;
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
   char *text = NULL, buf[4096];

   f = fopen(file, "rb");
   if (!f) return NULL;
   while ((size = fread(buf, 1, sizeof(buf), f)))
     {
	buf[size] = 0;
	text = _buf_append(text, buf, &len, &alloc);
     }
   fclose(f);
   return text;
}

static char *
_load_plain(const char *file)
{
   char *text, *text2;

   text = _load_file(file);
   if (text)
     {
	text2 = elm_entry_utf8_to_markup(text);
	free(text);
	return text2;
     }
   return NULL;
}

static void
_load(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char *text;
   if (!wd) return;
   if (!wd->file)
     {
	elm_entry_entry_set(wd->entry, "");
	return;
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
	elm_entry_entry_set(wd->entry, "Unknown Text Format");
	text = NULL;
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
   if (text2)
     {
	_save_markup_utf8(file, text2);
	free(text2);
     }
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
   if (!wd) return 0;
   _save(data);
   wd->delay_write = NULL;
   return 0;
}

static void
_entry_changed(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (wd->delay_write)
     {
	ecore_timer_del(wd->delay_write);
	wd->delay_write = NULL;
     }
   if (!wd->auto_write) return;
   wd->delay_write = ecore_timer_add(2.0, _delay_write, data);
}

static void
_hold_on(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_scroll_hold_push(wd->scr);
}

static void
_hold_off(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_scroll_hold_pop(wd->scr);
}

static void
_freeze_on(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_scroll_hold_push(wd->scr);
}

static void
_freeze_off(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_scroll_hold_pop(wd->scr);
}

/**
 * Add a new notepad to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Notepad
 */
EAPI Evas_Object *
elm_notepad_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "notepad");
   elm_widget_type_set(obj, "notepad");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_can_focus_set(obj, 1);

   wd->scr = elm_scroller_add(parent);
   elm_widget_resize_object_set(obj, wd->scr);
   wd->entry = elm_entry_add(parent);
   evas_object_size_hint_weight_set(wd->entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(wd->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_scroller_content_set(wd->scr, wd->entry);
   evas_object_show(wd->entry);

   elm_entry_entry_set(wd->entry, "");
   evas_object_smart_callback_add(wd->entry, "changed", _entry_changed, obj);
   
   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);
   
   wd->auto_write = EINA_TRUE;

   _sizing_eval(obj);
   return obj;
}

/**
 * This sets the file (and implicitly loads it) for the text to display and
 * then edit. All changes are written back to the file after a short delay.
 *
 * @param obj The notepad object
 * @param file The path to the file to load and save
 * @param format The file format
 *
 *
 * @ingroup Notepad
 */
EAPI void
elm_notepad_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format)
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
   eina_stringshare_replace(&wd->file, file);
   wd->format = format;
   _load(obj);
}

/**
 * This will enable or disable the scroller bounce mode for the notepad. See
 * elm_scroller_bounce_set() for details
 *
 * @param obj The notepad object
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * @ingroup Notepad
 */
EAPI void
elm_notepad_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_scroller_bounce_set(wd->scr, h_bounce, v_bounce);
}
