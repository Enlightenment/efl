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
   Eina_Hash *hash_mmap;
};

typedef struct
{
   const char *file;
   const char *key;
} File_Entry;

static void
_entry_free_cb(void *data)
{
   File_Entry *e = data;
   eina_stringshare_del(e->file);
   eina_stringshare_del(e->key);
   free(e);
}

typedef struct
{
   const Eina_File *file;
   const char *key;
} File_Mmap_Entry;

static void
_mmap_entry_free_cb(void *data)
{
   File_Mmap_Entry *e = data;
   eina_stringshare_del(e->key);
   free(e);
}

EOLIAN static Eo *
_efl_ui_text_image_item_factory_efl_object_constructor(Eo *obj,
     Efl_Ui_Text_Image_Item_Factory_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->hash = eina_hash_string_superfast_new(_entry_free_cb);
   pd->hash_mmap = eina_hash_string_superfast_new(_mmap_entry_free_cb);
   return obj;
}

EOLIAN static void
_efl_ui_text_image_item_factory_efl_object_destructor(Eo *obj,
     Efl_Ui_Text_Image_Item_Factory_Data *pd EINA_UNUSED)
{
   eina_hash_free(pd->hash);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Canvas_Object *
_efl_ui_text_image_item_factory_efl_canvas_text_item_factory_create(Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Image_Item_Factory_Data *pd EINA_UNUSED,
      Efl_Canvas_Object *object,
      const char *key)
{
   Efl_Canvas_Object *o;
   File_Mmap_Entry *emap;
   File_Entry *e;

   o = efl_add(EFL_CANVAS_IMAGE_CLASS, object);
   // First look at mmap'd files records
   emap = eina_hash_find(pd->hash_mmap, key);
   if (emap)
     {
        o = efl_add(EFL_CANVAS_IMAGE_CLASS, object);
        efl_file_mmap_set(o, emap->file, emap->key);
     }
   // If not found, look at file paths records
   else
     {
        const char *file = key, *filekey = NULL;
        e = eina_hash_find(pd->hash, key);

        if (e)
          {
             file = e->file;
             filekey = e->key;
          }

        efl_file_set(o, file, filekey);

     }

   if (efl_image_load_error_get(o) != EFL_IMAGE_LOAD_ERROR_NONE)
     {
        efl_del(o);
        o = NULL;
     }

   return o;
}

EOLIAN static Eina_Bool
_efl_ui_text_image_item_factory_matches_add(Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Image_Item_Factory_Data *pd,
      const char *name, const char *file, const char *key)
{
   File_Entry *e = malloc(sizeof(*e));
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

EOLIAN static Eina_Bool
_efl_ui_text_image_item_factory_matches_mmap_add(Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Image_Item_Factory_Data *pd,
      const char *name, const Eina_File *file, const char *key)
{
   File_Mmap_Entry *e = malloc(sizeof(*e));
   e->file = file;
   e->key = eina_stringshare_add(key);
   return eina_hash_add(pd->hash_mmap, name, e);
}

EOLIAN static Eina_Bool
_efl_ui_text_image_item_factory_matches_mmap_del(Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Image_Item_Factory_Data *pd,
      const char *name)
{
   return eina_hash_del(pd->hash_mmap, name, NULL);
}

#include "efl_ui_text_image_item_factory.eo.c"
