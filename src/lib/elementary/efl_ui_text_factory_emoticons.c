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

   o = efl_add(EFL_CANVAS_LAYOUT_CLASS, object);
   if (!elm_widget_element_update(object, o, key))
     {
        elm_widget_element_update(object, o, "wtf");
     }
   return o;
}

#include "efl_ui_text_factory_emoticons.eo.c"
