#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_TEXT_FACTORY_IMAGES_CLASS

typedef struct _Efl_Ui_Text_Factory_Images_Data Efl_Ui_Text_Factory_Images_Data;

struct _Efl_Ui_Text_Factory_Images_Data
{
   const char *name;
   Eina_Hash  *hash;
};

typedef struct
{
   Eina_File  *file;
   const char *key;
} File_Entry;

static void
_entry_free_cb(void *data)
{
   File_Entry *e = data;
   eina_file_close(e->file);
   eina_stringshare_del(e->key);
   free(e);
}

EOLIAN static Eo *
_efl_ui_text_factory_images_efl_object_constructor(Eo *obj,
     Efl_Ui_Text_Factory_Images_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->hash = eina_hash_string_superfast_new(_entry_free_cb);
   return obj;
}

EOLIAN static void
_efl_ui_text_factory_images_efl_object_destructor(Eo *obj,
     Efl_Ui_Text_Factory_Images_Data *pd EINA_UNUSED)
{
   eina_hash_free(pd->hash);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Canvas_Object *
_efl_ui_text_factory_images_efl_canvas_text_factory_create(Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Factory_Images_Data *pd EINA_UNUSED,
      Efl_Canvas_Object *object,
      const char *key)
{
   Efl_Canvas_Object *o;
   File_Entry *e;

   o = efl_add(EFL_CANVAS_IMAGE_CLASS, object);
   e = eina_hash_find(pd->hash, key);
   if (e)
     {
        efl_file_mmap_set(o, e->file, e->key);
     }
   else
     {
        efl_file_set(o, key, NULL);
     }

   if (efl_file_load_error_get(o) != EFL_GFX_IMAGE_LOAD_ERROR_NONE)
     {
        efl_del(o);
        o = NULL;
     }

   return o;
}

EOLIAN static Eina_Bool
_efl_ui_text_factory_images_matches_add(Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Factory_Images_Data *pd,
      const char *name, const char *file, const char *key)
{
   File_Entry *e;
   Eina_File *f = eina_file_open(file, EINA_FALSE);

   if (!f) return EINA_FALSE;

   e = malloc(sizeof(*e));
   e->file = f;
   e->key = eina_stringshare_add(key);

   if (!eina_hash_add(pd->hash, name, e))
     {
        ERR("Failed to add file path %s to key %s\n", file, key);
        eina_file_close(f);
        free(e);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_factory_images_matches_del(Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Factory_Images_Data *pd,
      const char *name)
{
   return eina_hash_del(pd->hash, name, NULL);
}

EOLIAN static Eina_Bool
_efl_ui_text_factory_images_matches_mmap_add(Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Factory_Images_Data *pd,
      const char *name, const Eina_File *file, const char *key)
{
   File_Entry *e;
   Eina_File *f;

   if (!file) return EINA_FALSE;

   f = eina_file_dup(file);
   e = malloc(sizeof(*e));
   e->file = f;
   e->key = eina_stringshare_add(key);

   if (!eina_hash_add(pd->hash, name, e))
     {
        ERR("Failed to add Eina_File %p to key %s\n", file, key);
        eina_file_close(f);
        free(e);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_text_factory_images_matches_mmap_del(Eo *obj EINA_UNUSED,
      Efl_Ui_Text_Factory_Images_Data *pd,
      const char *name)
{
   return eina_hash_del(pd->hash, name, NULL);
}

#include "efl_ui_text_factory_images.eo.c"
