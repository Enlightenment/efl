#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_TEXT_IMAGE_ITEM_FACTORY_CLASS

typedef struct _Efl_Ui_Text_Image_Item_Factory_Data Efl_Ui_Text_Image_Item_Factory_Data;

struct _Efl_Ui_Text_Image_Item_Factory_Data
{
   const char *name;
   Eina_Hash *hash;
};

typedef struct
{
   const char *file;
   const char *key;
} Image_Entry;

static void
_entry_free_cb(void *data)
{
   Image_Entry *e = data;
   eina_stringshare_del(e->file);
   eina_stringshare_del(e->key);
   free(e);
}

EOLIAN static Eo *
_efl_ui_text_image_item_factory_efl_object_constructor(Eo *obj,
     Efl_Ui_Text_Image_Item_Factory_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->hash = eina_hash_string_superfast_new(_entry_free_cb);
   return obj;
}

EOLIAN static void
_efl_ui_text_image_item_factory_efl_object_destructor(Eo *obj,
     Efl_Ui_Text_Image_Item_Factory_Data *pd EINA_UNUSED)
{
   eina_hash_free(pd->hash);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Canvas_Object
*_efl_ui_text_image_item_factory_efl_canvas_text_item_factory_create(Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Image_Item_Factory_Data *pd EINA_UNUSED,
      Efl_Canvas_Object *object,
      const char *key)
{
   Efl_Canvas_Object *o;
   Image_Entry *e;
   const char *file = key, *filekey = NULL;

   e = eina_hash_find(pd->hash, key);
   if (e)
     {
        file = e->file;
        filekey = e->key;
     }

   o = evas_object_image_filled_add(evas_object_evas_get(object));
   evas_object_image_file_set(o, file, filekey);
   if (evas_object_image_load_error_get(o) != EVAS_LOAD_ERROR_NONE)
     {
        evas_object_del(o);
        o = NULL;
     }
   return o;
}

EOLIAN static Eina_Bool
_efl_ui_text_image_item_factory_matches_add(Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Image_Item_Factory_Data *pd,
      const char *name, const char *file, const char *key)
{
   Image_Entry *e = malloc(sizeof(*e));
   e->file = eina_stringshare_add(file);
   e->key = eina_stringshare_add(key);
   return eina_hash_add(pd->hash, name, e);
}

EOLIAN static Eina_Bool
_efl_ui_text_image_item_factory_matches_del(Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Image_Item_Factory_Data *pd,
      const char *name)
{
   return eina_hash_del(pd->hash, name, NULL);
}

#include "efl_ui_text_image_item_factory.eo.c"
