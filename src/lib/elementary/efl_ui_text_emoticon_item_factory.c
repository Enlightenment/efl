#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_TEXT_EMOTICON_ITEM_FACTORY_CLASS

typedef struct _Efl_Ui_Text_Emoticon_Item_Factory_Data Efl_Ui_Text_Emoticon_Item_Factory_Data;

struct _Efl_Ui_Text_Emoticon_Item_Factory_Data
{
   const char *name;
};

EOLIAN static Eo *
_efl_ui_text_emoticon_item_factory_efl_object_constructor(Eo *obj,
     Efl_Ui_Text_Emoticon_Item_Factory_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   return obj;
}

EOLIAN static void
_efl_ui_text_emoticon_item_factory_efl_object_destructor(Eo *obj,
     Efl_Ui_Text_Emoticon_Item_Factory_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}


EOLIAN static Efl_Canvas_Object
*_efl_ui_text_emoticon_item_factory_efl_canvas_text_item_factory_create(
      Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Emoticon_Item_Factory_Data *pd EINA_UNUSED,
      Efl_Canvas_Object *object,
      const char *key)
{
   Eo *o;
   const char *style = elm_widget_style_get(object);

   o = edje_object_add(evas_object_evas_get(object));
   if (!_elm_theme_object_set
         (object, o, "entry", key, style))
     _elm_theme_object_set
       (object, o, "entry/emoticon", "wtf", style);
   return o;
}

#include "efl_ui_text_emoticon_item_factory.eo.c"
