//FIXME this widget should inherit from file selector button

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_fileselector_entry.h"

EAPI const char ELM_FILESELECTOR_ENTRY_SMART_NAME[] = "elm_fileselector_entry";

static const char SIG_CHANGED[] = "changed";
static const char SIG_ACTIVATED[] = "activated";
static const char SIG_PRESS[] = "press";
static const char SIG_LONGPRESSED[] = "longpressed";
static const char SIG_CLICKED[] = "clicked";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_FOCUSED[] = "focused";
static const char SIG_UNFOCUSED[] = "unfocused";
static const char SIG_SELECTION_PASTE[] = "selection,paste";
static const char SIG_SELECTION_COPY[] = "selection,copy";
static const char SIG_SELECTION_CUT[] = "selection,cut";
static const char SIG_UNPRESSED[] = "unpressed";
static const char SIG_FILE_CHOSEN[] = "file,chosen";
static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   {SIG_CHANGED, ""},
   {SIG_ACTIVATED, ""},
   {SIG_PRESS, ""},
   {SIG_LONGPRESSED, ""},
   {SIG_CLICKED, ""},
   {SIG_CLICKED_DOUBLE, ""},
   {SIG_FOCUSED, ""},
   {SIG_UNFOCUSED, ""},
   {SIG_SELECTION_PASTE, ""},
   {SIG_SELECTION_COPY, ""},
   {SIG_SELECTION_CUT, ""},
   {SIG_UNPRESSED, ""},
   {SIG_FILE_CHOSEN, "s"},
   {NULL, NULL}
};

#define SIG_FWD(name)                                                       \
  static void                                                               \
  _##name##_fwd(void *data, Evas_Object * obj __UNUSED__, void *event_info) \
  {                                                                         \
     evas_object_smart_callback_call(data, SIG_##name, event_info);         \
  }
SIG_FWD(CHANGED)
SIG_FWD(PRESS)
SIG_FWD(LONGPRESSED)
SIG_FWD(CLICKED)
SIG_FWD(CLICKED_DOUBLE)
SIG_FWD(FOCUSED)
SIG_FWD(UNFOCUSED)
SIG_FWD(SELECTION_PASTE)
SIG_FWD(SELECTION_COPY)
SIG_FWD(SELECTION_CUT)
SIG_FWD(UNPRESSED)
#undef SIG_FWD

EVAS_SMART_SUBCLASS_NEW
  (ELM_FILESELECTOR_ENTRY_SMART_NAME, _elm_fileselector_entry,
  Elm_Fileselector_Entry_Smart_Class, Elm_Layout_Smart_Class,
  elm_layout_smart_class_get, _smart_callbacks);

static void
_FILE_CHOSEN_fwd(void *data,
                 Evas_Object *obj __UNUSED__,
                 void *event_info)
{
   const char *file = event_info;
   char *s;

   ELM_FILESELECTOR_ENTRY_DATA_GET(data, sd);

   s = elm_entry_utf8_to_markup(file);
   if (!s) return;
   elm_object_text_set(sd->entry, s);
   free(s);
   evas_object_smart_callback_call(data, SIG_FILE_CHOSEN, event_info);
}

static void
_ACTIVATED_fwd(void *data,
               Evas_Object *obj __UNUSED__,
               void *event_info)
{
   const char *file;

   ELM_FILESELECTOR_ENTRY_DATA_GET(data, sd);

   file = elm_object_text_get(sd->entry);
   elm_fileselector_button_path_set(sd->button, file);
   evas_object_smart_callback_call(data, SIG_ACTIVATED, event_info);
}

static void
_elm_fileselector_entry_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;

   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static Eina_Bool
_elm_fileselector_entry_smart_focus_next(const Evas_Object *obj,
                                         Elm_Focus_Direction dir,
                                         Evas_Object **next)
{
   Evas_Object *chain[2];
   Evas_Object *to_focus;
   unsigned char i;

   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   /* Direction */
   if (dir == ELM_FOCUS_PREVIOUS)
     {
        chain[0] = sd->button;
        chain[1] = sd->entry;
     }
   else if (dir == ELM_FOCUS_NEXT)
     {
        chain[0] = sd->entry;
        chain[1] = sd->button;
     }
   else
     return EINA_FALSE;

   i = elm_widget_focus_get(chain[1]);

   if (elm_widget_focus_next_get(chain[i], dir, next))
     return EINA_TRUE;

   i = !i;

   if (elm_widget_focus_next_get(chain[i], dir, &to_focus))
     {
        *next = to_focus;
        return !!i;
     }

   return EINA_FALSE;
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   elm_widget_mirrored_set(sd->button, rtl);
}

static Eina_Bool
_elm_fileselector_entry_smart_theme(Evas_Object *obj)
{
   const char *style;
   char buf[1024];

   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_fileselector_entry_parent_sc)->theme(obj))
     return EINA_FALSE;

   style = elm_widget_style_get(obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   if (elm_object_disabled_get(obj))
     elm_layout_signal_emit(obj, "elm,state,disabled", "elm");

   if (!style) style = "default";
   snprintf(buf, sizeof(buf), "fileselector_entry/%s", style);
   elm_widget_style_set(sd->button, buf);
   elm_widget_style_set(sd->entry, buf);

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_fileselector_entry_smart_disable(Evas_Object *obj)
{
   Eina_Bool val;

   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_fileselector_entry_parent_sc)->disable(obj))
     return EINA_FALSE;

   val = elm_widget_disabled_get(obj);

   elm_widget_disabled_set(sd->button, val);
   elm_widget_disabled_set(sd->entry, val);

   return EINA_TRUE;
}

static Eina_Bool
_elm_fileselector_entry_smart_text_set(Evas_Object *obj,
                                       const char *item,
                                       const char *label)
{
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   if (item && strcmp(item, "default"))
     return _elm_fileselector_entry_parent_sc->text_set(obj, item, label);

   elm_object_text_set(sd->button, label);

   return EINA_TRUE;
}

static const char *
_elm_fileselector_entry_smart_text_get(const Evas_Object *obj,
                                       const char *item)
{
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   if (item && strcmp(item, "default"))
     return _elm_fileselector_entry_parent_sc->text_get(obj, item);

   return elm_object_text_get(sd->button);
}

static Eina_Bool
_elm_fileselector_entry_smart_content_set(Evas_Object *obj,
                                          const char *part,
                                          Evas_Object *content)
{
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   if (part && strcmp(part, "button icon"))
     return ELM_CONTAINER_CLASS(_elm_fileselector_entry_parent_sc)->content_set
              (obj, part, content);

   elm_layout_content_set(sd->button, NULL, content);

   return EINA_TRUE;
}

static Evas_Object *
_elm_fileselector_entry_smart_content_get(const Evas_Object *obj,
                                          const char *part)
{
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   if (part && strcmp(part, "button icon"))
     return ELM_CONTAINER_CLASS(_elm_fileselector_entry_parent_sc)->
            content_get(obj, part);

   return elm_layout_content_get(sd->button, NULL);
}

static Evas_Object *
_elm_fileselector_entry_smart_content_unset(Evas_Object *obj,
                                            const char *part)
{
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);
   if (part && strcmp(part, "button icon"))
     return ELM_CONTAINER_CLASS(_elm_fileselector_entry_parent_sc)->
            content_unset(obj, part);

   return elm_layout_content_unset(sd->button, NULL);
}

static void
_elm_fileselector_entry_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Fileselector_Entry_Smart_Data);

   ELM_WIDGET_CLASS(_elm_fileselector_entry_parent_sc)->base.add(obj);

   elm_layout_theme_set
     (obj, "fileselector_entry", "base", elm_widget_style_get(obj));

   priv->button = elm_fileselector_button_add(obj);
   elm_widget_mirrored_automatic_set(priv->button, EINA_FALSE);
   elm_widget_style_set(priv->button, "fileselector_entry/default");

   elm_layout_content_set(obj, "elm.swallow.button", priv->button);
   elm_fileselector_button_expandable_set
     (priv->button, _elm_config->fileselector_expand_enable);

#define SIG_FWD(name) \
  evas_object_smart_callback_add(priv->button, SIG_##name, _##name##_fwd, obj)
   SIG_FWD(CLICKED);
   SIG_FWD(UNPRESSED);
   SIG_FWD(FILE_CHOSEN);
#undef SIG_FWD

   priv->entry = elm_entry_add(obj);
   elm_entry_scrollable_set(priv->entry, EINA_TRUE);
   elm_widget_mirrored_automatic_set(priv->entry, EINA_FALSE);
   elm_widget_style_set(priv->entry, "fileselector_entry/default");
   elm_entry_single_line_set(priv->entry, EINA_TRUE);
   elm_entry_editable_set(priv->entry, EINA_TRUE);
   elm_layout_content_set(obj, "elm.swallow.entry", priv->entry);

#define SIG_FWD(name) \
  evas_object_smart_callback_add(priv->entry, SIG_##name, _##name##_fwd, obj)
   SIG_FWD(CHANGED);
   SIG_FWD(ACTIVATED);
   SIG_FWD(PRESS);
   SIG_FWD(LONGPRESSED);
   SIG_FWD(CLICKED);
   SIG_FWD(CLICKED_DOUBLE);
   SIG_FWD(FOCUSED);
   SIG_FWD(UNFOCUSED);
   SIG_FWD(SELECTION_PASTE);
   SIG_FWD(SELECTION_COPY);
   SIG_FWD(SELECTION_CUT);
#undef SIG_FWD

   elm_widget_can_focus_set(obj, EINA_FALSE);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   elm_layout_sizing_eval(obj);
}

static void
_elm_fileselector_entry_smart_del(Evas_Object *obj)
{
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   if (sd->path) free(sd->path);

   ELM_WIDGET_CLASS(_elm_fileselector_entry_parent_sc)->base.del(obj);
}

static void
_elm_fileselector_entry_smart_set_user(Elm_Fileselector_Entry_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_fileselector_entry_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_fileselector_entry_smart_del;

   ELM_WIDGET_CLASS(sc)->disable = _elm_fileselector_entry_smart_disable;
   ELM_WIDGET_CLASS(sc)->theme = _elm_fileselector_entry_smart_theme;

   ELM_WIDGET_CLASS(sc)->focus_next = _elm_fileselector_entry_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_CONTAINER_CLASS(sc)->content_set =
     _elm_fileselector_entry_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_get =
     _elm_fileselector_entry_smart_content_get;
   ELM_CONTAINER_CLASS(sc)->content_unset =
     _elm_fileselector_entry_smart_content_unset;

   ELM_LAYOUT_CLASS(sc)->text_set = _elm_fileselector_entry_smart_text_set;
   ELM_LAYOUT_CLASS(sc)->text_get = _elm_fileselector_entry_smart_text_get;
   ELM_LAYOUT_CLASS(sc)->sizing_eval =
     _elm_fileselector_entry_smart_sizing_eval;
}

EAPI const Elm_Fileselector_Entry_Smart_Class *
elm_fileselector_entry_smart_class_get(void)
{
   static Elm_Fileselector_Entry_Smart_Class _sc =
     ELM_FILESELECTOR_ENTRY_SMART_CLASS_INIT_NAME_VERSION
       (ELM_FILESELECTOR_ENTRY_SMART_NAME);
   static const Elm_Fileselector_Entry_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_fileselector_entry_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_fileselector_entry_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_fileselector_entry_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_fileselector_entry_selected_set(Evas_Object *obj,
                                    const char *path)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   elm_fileselector_button_path_set(sd->button, path);
}

EAPI const char *
elm_fileselector_entry_selected_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj) NULL;
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   return elm_fileselector_button_path_get(sd->button);
}

EAPI void
elm_fileselector_entry_window_title_set(Evas_Object *obj,
                                        const char *title)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   elm_fileselector_button_window_title_set(sd->button, title);
}

EAPI const char *
elm_fileselector_entry_window_title_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj) NULL;
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   return elm_fileselector_button_window_title_get(sd->button);
}

EAPI void
elm_fileselector_entry_window_size_set(Evas_Object *obj,
                                       Evas_Coord width,
                                       Evas_Coord height)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   elm_fileselector_button_window_size_set(sd->button, width, height);
}

EAPI void
elm_fileselector_entry_window_size_get(const Evas_Object *obj,
                                       Evas_Coord *width,
                                       Evas_Coord *height)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   elm_fileselector_button_window_size_get(sd->button, width, height);
}

EAPI void
elm_fileselector_entry_path_set(Evas_Object *obj,
                                const char *path)
{
   char *s;

   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   elm_fileselector_button_path_set(sd->button, path);
   s = elm_entry_utf8_to_markup(path);
   if (s)
     {
        elm_object_text_set(sd->entry, s);
        free(s);
     }
}

EAPI const char *
elm_fileselector_entry_path_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj) NULL;
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   if (sd->path) free(sd->path);
   sd->path = elm_entry_markup_to_utf8(elm_object_text_get(sd->entry));
   return sd->path;
}

EAPI void
elm_fileselector_entry_expandable_set(Evas_Object *obj,
                                      Eina_Bool value)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   elm_fileselector_button_expandable_set(sd->button, value);
}

EAPI Eina_Bool
elm_fileselector_entry_expandable_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   return elm_fileselector_button_expandable_get(sd->button);
}

EAPI void
elm_fileselector_entry_folder_only_set(Evas_Object *obj,
                                       Eina_Bool value)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   elm_fileselector_button_folder_only_set(sd->button, value);
}

EAPI Eina_Bool
elm_fileselector_entry_folder_only_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   return elm_fileselector_button_folder_only_get(sd->button);
}

EAPI void
elm_fileselector_entry_is_save_set(Evas_Object *obj,
                                   Eina_Bool value)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   elm_fileselector_button_is_save_set(sd->button, value);
}

EAPI Eina_Bool
elm_fileselector_entry_is_save_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   return elm_fileselector_button_is_save_get(sd->button);
}

EAPI void
elm_fileselector_entry_inwin_mode_set(Evas_Object *obj,
                                      Eina_Bool value)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj);
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   elm_fileselector_button_inwin_mode_set(sd->button, value);
}

EAPI Eina_Bool
elm_fileselector_entry_inwin_mode_get(const Evas_Object *obj)
{
   ELM_FILESELECTOR_ENTRY_CHECK(obj) EINA_FALSE;
   ELM_FILESELECTOR_ENTRY_DATA_GET(obj, sd);

   return elm_fileselector_button_inwin_mode_get(sd->button);
}
