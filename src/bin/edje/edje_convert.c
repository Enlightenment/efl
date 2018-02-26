#include "edje_private.h"

#include "edje_cc.h"
#include "edje_convert.h"

static const Edje_File *_current_edje_file = NULL;

const Edje_File *
_edje_file_get(void)
{
   return _current_edje_file;
}

void
_edje_file_set(const Edje_File *edf)
{
   _current_edje_file = edf;
}

static Eina_Bool
_edje_file_convert_external(Edje_File *edf, Old_Edje_File *oedf)
{
   Edje_External_Directory_Entry *ede;
   unsigned int max;
   unsigned int i = 0;

   edf->external_dir = calloc(1, sizeof (Edje_External_Directory));
   if (!edf->external_dir) return EINA_FALSE;
   if (!oedf->external_dir) return EINA_TRUE;

   max = eina_list_count(oedf->external_dir->entries);
   edf->external_dir->entries = calloc(1, sizeof (Edje_External_Directory_Entry) * max);
   edf->external_dir->entries_count = max;

   if (!edf->external_dir->entries && max)
     return EINA_FALSE;

   EINA_LIST_FREE(oedf->external_dir->entries, ede)
     {
        edf->external_dir->entries[i++].entry = ede->entry;
        free(ede);
     }

   free(oedf->external_dir);
   oedf->external_dir = NULL;

   return EINA_TRUE;
}

static Eina_Bool
_edje_file_convert_images(Edje_File *edf, Old_Edje_File *oedf)
{
   Edje_Image_Directory_Entry *de;
   Edje_Image_Directory_Set *ds;
   Eina_List *l;
   int max;

   edf->image_dir = calloc(1, sizeof (Edje_Image_Directory));
   if (!edf->image_dir) return EINA_FALSE;
   if (!oedf->image_dir) return EINA_TRUE;

   max = -1;
   EINA_LIST_FOREACH(oedf->image_dir->entries, l, de)
     if (max < de->id)
       max = de->id;

   edf->image_dir->entries = calloc(1, sizeof (Edje_Image_Directory_Entry) * (max + 1));
   edf->image_dir->entries_count = max + 1;

   if (!edf->image_dir->entries && edf->image_dir->entries_count)
     return EINA_FALSE;

   EINA_LIST_FREE(oedf->image_dir->entries, de)
     {
        memcpy(edf->image_dir->entries + de->id,
               de,
               sizeof (Edje_Image_Directory_Entry));
        free(de);
     }

   max = -1;
   EINA_LIST_FOREACH(oedf->image_dir->sets, l, ds)
     if (max < ds->id)
       max = ds->id;

   edf->image_dir->sets = calloc(1, sizeof (Edje_Image_Directory_Set) * (max + 1));
   edf->image_dir->sets_count = max + 1;

   if (!edf->image_dir->sets && edf->image_dir->sets_count)
     {
        free(edf->image_dir->entries);
        edf->image_dir->entries = NULL;
        return EINA_FALSE;
     }

   EINA_LIST_FREE(oedf->image_dir->sets, ds)
     {
        memcpy(edf->image_dir->sets + ds->id,
               ds,
               sizeof (Edje_Image_Directory_Set));
        free(ds);
     }

   return EINA_TRUE;
}

Edje_File *
_edje_file_convert(Eet_File *ef, Old_Edje_File *oedf)
{
   Edje_Part_Collection_Directory_Entry *ce;
   Edje_Font_Directory_Entry *fnt;
   Edje_File *edf;
   Eina_List *l;
   Old_Edje_Data *ed;

   if (oedf->version < 2) return NULL;

   edf = calloc(1, sizeof (Edje_File));
   if (!edf) return NULL;

   edf->free_strings = 0;

   edf->fonts = eina_hash_string_small_new(free);
   edf->collection = eina_hash_string_small_new(free);
   edf->data = eina_hash_string_small_new(free);

   if (!edf->fonts || !edf->collection || !edf->data)
     goto on_error;

   EINA_LIST_FREE(oedf->data, ed)
     {
        Edje_String *es;

        es = calloc(1, sizeof (Edje_String));
        if (!es) continue;

        es->str = ed->value;

        eina_hash_direct_add(edf->data, ed->key, es);
        free(ed);
     }

   EINA_LIST_FOREACH(oedf->collection_dir->entries, l, ce)
     if (ce->entry)
       eina_hash_direct_add(edf->collection, ce->entry, ce);
     else
       error_and_abort(ef, "Collection %i: name missing.\n", ce->id);

   if (oedf->font_dir)
     EINA_LIST_FOREACH(oedf->font_dir->entries, l, fnt)
       eina_hash_direct_add(edf->fonts, fnt->entry, fnt);

   if (!_edje_file_convert_images(edf, oedf))
     goto on_error;

   if (!_edje_file_convert_external(edf, oedf))
     goto on_error;

   edf->styles = oedf->styles;
   edf->color_classes = oedf->color_classes;
   edf->text_classes = oedf->text_classes;
   edf->size_classes = oedf->size_classes;
   edf->version = EDJE_FILE_VERSION;
   edf->feature_ver = oedf->feature_ver;
   edf->compiler = oedf->compiler;

   edf->dangling = EINA_FALSE;
   edf->warning = EINA_FALSE;

   /* Below you will find all memory structure that could be cleaned when under
      memory pressure */
   edf->collection_cache = NULL;
   edf->collection_patterns = NULL;

   return edf;

on_error:
   eina_hash_free(edf->fonts);
   eina_hash_free(edf->collection);
   eina_hash_free(edf->data);
   free(edf->image_dir);
   free(edf->external_dir);
   free(edf);
   return NULL;
}

static void
_edje_collection_program_add(Edje_Program ***array,
                             unsigned int *count,
                             Edje_Program *add)
{
   Edje_Program **tmp;

   tmp = realloc(*array, sizeof (Edje_Program *) * (*count + 1));
   if (!tmp)
     {
        error_and_abort(ef, "Not enough memory");
        return;
     }

   tmp[(*count)++] = add;
   *array = tmp;
}

Edje_Part_Collection *
_edje_collection_convert(Eet_File *ef, Edje_Part_Collection_Directory_Entry *ce, Old_Edje_Part_Collection *oedc)
{
   Edje_Part_Collection *edc;
   Old_Edje_Part *part;
   Edje_Program *pg;
   Old_Edje_Data *di;
   Eina_List *l;
   char *buffer;
   unsigned int k;

   oedc->part = ce->entry;

   /* Count each type part and their respective state */
   EINA_LIST_FOREACH(oedc->parts, l, part)
     {
        int *count;
        int dummy = 0;

        switch (part->type)
          {
#define CSP(Tp, Ce)       \
case EDJE_PART_TYPE_##Tp: \
  count = &Ce->count.Tp;  \
  break;

             CSP(RECTANGLE, ce);
             CSP(TEXT, ce);
             CSP(IMAGE, ce);
             CSP(SWALLOW, ce);
             CSP(TEXTBLOCK, ce);
             CSP(GROUP, ce);
             CSP(BOX, ce);
             CSP(TABLE, ce);
             CSP(EXTERNAL, ce);
             CSP(VECTOR, ce);

           default:
             count = &dummy;
             break;
          }

        *count += eina_list_count(part->other_desc) + 1;
     }
   ce->count.part = eina_list_count(oedc->parts);

#define CONVERT_EMN(Tp, Sz, Ce)                         \
  buffer = alloca(strlen(ce->entry) + strlen(#Tp) + 2); \
  sprintf(buffer, "%s/%s", ce->entry, #Tp);             \
  Ce->mp.Tp = eina_mempool_add("one_big", buffer, NULL, sizeof (Sz), Ce->count.Tp);

   CONVERT_EMN(RECTANGLE, Edje_Part_Description_Common, ce);
   CONVERT_EMN(TEXT, Edje_Part_Description_Text, ce);
   CONVERT_EMN(IMAGE, Edje_Part_Description_Image, ce);
   CONVERT_EMN(SWALLOW, Edje_Part_Description_Common, ce);
   CONVERT_EMN(TEXTBLOCK, Edje_Part_Description_Text, ce);
   CONVERT_EMN(GROUP, Edje_Part_Description_Common, ce);
   CONVERT_EMN(BOX, Edje_Part_Description_Box, ce);
   CONVERT_EMN(TABLE, Edje_Part_Description_Table, ce);
   CONVERT_EMN(EXTERNAL, Edje_Part_Description_External, ce);
   CONVERT_EMN(part, Edje_Part, ce);
   CONVERT_EMN(VECTOR, Edje_Part_Description_Vector, ce);

   /* Change structure layout */
   edc = calloc(1, sizeof (Edje_Part_Collection));
   if (!edc) error_and_abort(ef, "Not enough memory");
   ce->ref = edc;

   EINA_LIST_FREE(oedc->programs, pg)
     {
        if (!pg->signal && !pg->source)
          _edje_collection_program_add(&edc->programs.nocmp,
                                       &edc->programs.nocmp_count,
                                       pg);
        else if (pg->signal && !strpbrk(pg->signal, "*?[\\")
                 && pg->source && !strpbrk(pg->source, "*?[\\"))
          _edje_collection_program_add(&edc->programs.strcmp,
                                       &edc->programs.strcmp_count,
                                       pg);
        else if (pg->signal && edje_program_is_strncmp(pg->signal)
                 && pg->source && edje_program_is_strncmp(pg->source))
          _edje_collection_program_add(&edc->programs.strncmp,
                                       &edc->programs.strncmp_count,
                                       pg);
        else if (pg->signal && edje_program_is_strrncmp(pg->signal)
                 && pg->source && edje_program_is_strrncmp(pg->source))
          _edje_collection_program_add(&edc->programs.strrncmp,
                                       &edc->programs.strrncmp_count,
                                       pg);
        else
          _edje_collection_program_add(&edc->programs.fnmatch,
                                       &edc->programs.fnmatch_count,
                                       pg);
     }

   edc->data = eina_hash_string_small_new(NULL);
   EINA_LIST_FREE(oedc->data, di)
     {
        Edje_String *es;

        es = calloc(1, sizeof (Edje_String));
        if (!es) continue;

        es->str = di->value;

        eina_hash_direct_add(edc->data, di->key, es);
        free(di);
     }

   edc->parts_count = eina_list_count(oedc->parts);
   edc->parts = calloc(edc->parts_count, sizeof (Edje_Part *));
   if (edc->parts_count && !edc->parts)
     error_and_abort(ef, "Not enough memory");
   k = 0;

   EINA_LIST_FREE(oedc->parts, part)
     {
        Old_Edje_Part_Description *oepd;
        Edje_Pack_Element *elm;
        Edje_Part *replacement;
        unsigned int i;

        replacement = eina_mempool_malloc(ce->mp.part, sizeof (Edje_Part));
        if (!replacement)
          error_and_abort(ef, "Not enough memory");

        replacement->name = part->name;
        replacement->default_desc = _edje_description_convert(part->type, ce, part->default_desc);

        replacement->other.desc_count = eina_list_count(part->other_desc);
        replacement->other.desc = calloc(replacement->other.desc_count, sizeof (Edje_Part_Description_Common *));

        i = 0;
        EINA_LIST_FREE(part->other_desc, oepd)
          replacement->other.desc[i++] = _edje_description_convert(part->type, ce, oepd);

        replacement->source = part->source;
        replacement->source2 = part->source2;
        replacement->source3 = part->source3;
        replacement->source4 = part->source4;
        replacement->source5 = part->source5;
        replacement->source6 = part->source6;
        replacement->id = part->id;
        replacement->clip_to_id = part->clip_to_id;
        replacement->dragable = part->dragable;
        replacement->items_count = eina_list_count(part->items);
        replacement->items = calloc(replacement->items_count, sizeof (Edje_Pack_Element *));

        i = 0;
        EINA_LIST_FREE(part->items, elm)
          replacement->items[i++] = elm;

        replacement->type = part->type;
        replacement->effect = part->effect;
        replacement->mouse_events = part->mouse_events;
        replacement->repeat_events = part->repeat_events;
        replacement->ignore_flags = part->ignore_flags;
        replacement->mask_flags = part->mask_flags;
        replacement->scale = part->scale;
        replacement->precise_is_inside = part->precise_is_inside;
        replacement->use_alternate_font_metrics = part->use_alternate_font_metrics;
        replacement->pointer_mode = part->pointer_mode;
        replacement->entry_mode = part->entry_mode;
        replacement->select_mode = part->select_mode;
        replacement->multiline = part->multiline;
        replacement->api = part->api;
        replacement->required = part->required;

        edc->parts[k++] = replacement;

        free(part);
     }

   edc->id = oedc->id;
   edc->alias = oedc->alias;
   edc->prop.min = oedc->prop.min;
   edc->prop.max = oedc->prop.max;
   edc->script = oedc->script;
   edc->part = oedc->part;
   edc->lua_script_only = oedc->lua_script_only;
   edc->checked = oedc->checked;

   free(oedc);

   return edc;
}

Edje_Part_Description_Common *
_edje_description_convert(int type,
                          Edje_Part_Collection_Directory_Entry *ce,
                          Old_Edje_Part_Description *oed)
{
   Edje_Part_Description_Common *result = NULL;

   switch (type)
     {
      case EDJE_PART_TYPE_RECTANGLE:
        result = eina_mempool_malloc(ce->mp.RECTANGLE,
                                     sizeof (Edje_Part_Description_Common));
        break;

      case EDJE_PART_TYPE_SWALLOW:
        result = eina_mempool_malloc(ce->mp.SWALLOW,
                                     sizeof (Edje_Part_Description_Common));
        break;

      case EDJE_PART_TYPE_GROUP:
        result = eina_mempool_malloc(ce->mp.GROUP,
                                     sizeof (Edje_Part_Description_Common));
        break;

      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *img;
         Edje_Part_Image_Id *id;
         unsigned int i = 0;

         img = eina_mempool_malloc(ce->mp.IMAGE, sizeof (Edje_Part_Description_Image));

         img->image.tweens_count = eina_list_count(oed->image.tween_list);
         img->image.tweens = calloc(img->image.tweens_count,
                                    sizeof (Edje_Part_Image_Id *));
         if (img->image.tweens_count > 0 && !img->image.tweens)
           {
              eina_mempool_free(ce->mp.IMAGE, img);
              return NULL;
           }

         EINA_LIST_FREE(oed->image.tween_list, id)
           img->image.tweens[i++] = id;

         img->image.id = oed->image.id;
         img->image.scale_hint = oed->image.scale_hint;
         img->image.set = oed->image.set;

         img->image.border = oed->image.border;
         img->image.fill = oed->image.fill;

         result = &img->common;
         break;
      }

#define CONVERT_ALLOC_POOL(Short, Type, Name)                                       \
case EDJE_PART_TYPE_##Short:                                                        \
{                                                                                   \
   Edje_Part_Description_##Type * Name;                                             \
                                                                                    \
   Name = eina_mempool_malloc(ce->mp.Short, sizeof (Edje_Part_Description_##Type)); \
   Name->Name = oed->Name;                                                          \
   result = &Name->common;                                                          \
   break;                                                                           \
}

        CONVERT_ALLOC_POOL(TEXT, Text, text);
        CONVERT_ALLOC_POOL(TEXTBLOCK, Text, text);
        CONVERT_ALLOC_POOL(BOX, Box, box);
        CONVERT_ALLOC_POOL(TABLE, Table, table);
        CONVERT_ALLOC_POOL(EXTERNAL, External, external_params);
        CONVERT_ALLOC_POOL(VECTOR, Vector, vector);
     }

   if (result)
     *result = oed->common;

   free(oed);
   return result;
}

