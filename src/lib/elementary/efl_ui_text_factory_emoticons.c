#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_TEXT_FACTORY_EMOTICONS_CLASS

typedef struct _Efl_Ui_Text_Factory_Emoticons_Data Efl_Ui_Text_Factory_Emoticons_Data;

struct _Efl_Ui_Text_Factory_Emoticons_Data
{
};

EOLIAN static Efl_Canvas_Object
*_efl_ui_text_factory_emoticons_efl_canvas_text_factory_create(
      Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Factory_Emoticons_Data *pd EINA_UNUSED,
      Efl_Canvas_Object *object,
      const char *key)
{
   Eo *o;
   const char *style = elm_widget_style_get(object);

   o = edje_object_add(evas_object_evas_get(object));
   if (!_elm_theme_object_set
         (object, o, "text", key, style))
     _elm_theme_object_set
       (object, o, "text/emoticon", "wtf", style);
   return o;
}

#include "efl_ui_text_factory_emoticons.eo.c"
