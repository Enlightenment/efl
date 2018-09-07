#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_TEXT_FACTORY_FALLBACK_CLASS

typedef struct _Efl_Ui_Text_Factory_Fallback_Data Efl_Ui_Text_Factory_Fallback_Data;

struct _Efl_Ui_Text_Factory_Fallback_Data
{
   Efl_Canvas_Text_Factory *emoticon_factory, *image_factory;
};

EOLIAN static Eo *
_efl_ui_text_factory_fallback_efl_object_constructor(Eo *obj,
     Efl_Ui_Text_Factory_Fallback_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->image_factory = efl_add(EFL_UI_TEXT_FACTORY_IMAGES_CLASS, obj);
   pd->emoticon_factory = efl_add(EFL_UI_TEXT_FACTORY_EMOTICONS_CLASS, obj);
   return obj;
}

EOLIAN static void
_efl_ui_text_factory_fallback_efl_object_destructor(Eo *obj,
     Efl_Ui_Text_Factory_Fallback_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}


EOLIAN static Efl_Canvas_Object
*_efl_ui_text_factory_fallback_efl_canvas_text_factory_create(
      Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Factory_Fallback_Data *pd EINA_UNUSED,
      Efl_Canvas_Object *object,
      const char *key)
{
   Efl_Canvas_Object *o = NULL;

   // Parse the string. Can be either:
   //   1. some/name - an emoticon (load from theme)
   //   2. file:// - image file
   if (key && !strncmp(key, "file://", 7))
     {
        const char *fname = key + 7;
        o = efl_canvas_text_factory_create(pd->image_factory, object, fname);
     }
   else
     {
        o = efl_canvas_text_factory_create(pd->emoticon_factory, object, key);
     }
   return o;
}

#include "efl_ui_text_factory_fallback.eo.c"
