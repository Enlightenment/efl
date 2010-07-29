#include "edje_private.h"

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

static void
_edje_font_string_free(void *data)
{
   Edje_Font_Directory_Entry *fe = data;

   eina_stringshare_del(fe->path);
   free(fe);
}

static void
_edje_collection_string_free(void *data)
{
   Edje_Part_Collection_Directory_Entry *ce = data;

   eina_stringshare_del(ce->entry);

   if (ce->ref)
     {
	Edje_File *edf;

	edf = (Edje_File*) _edje_file_get();

	if (!edf->warning)
	  ERR("EDJE ERROR:\n"
	      "\n"
	      "Naughty Programmer - spank spank!\n"
	      "\n"
	      "This program as probably called edje_shutdown() with active Edje objects\n"
	      "still around.\n This can cause problems as both Evas and Edje retain\n"
	      "references to the objects. you should shut down all canvases and objects\n"
	      "before calling edje_shutdown().\n"
	      "The following errors are the edje object files and parts that are still\n"
	      "hanging around, with their reference counts");

	edf->warning = 1;
	ERR("EEK: EDJE FILE: \"%s\" ref(%i) PART: \"%s\" ref(%i) ",
	    edf->path, edf->references,
	    ce->ref->part, ce->ref->references);

	_edje_collection_free(edf, ce->ref, ce);
     }

   free(ce);
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
_edje_file_convert(Eet_File *file, Old_Edje_File *oedf)
{
   Edje_Part_Collection_Directory_Entry *ce;
   Edje_Font_Directory_Entry *fnt;
   Edje_File *edf;
   Eina_List *l;
   Edje_Data *ed;

   edf = calloc(1, sizeof (Edje_File));
   if (!edf) return NULL;

   edf->free_strings = eet_dictionary_get(file) ? 0 : 1;

   if (edf->free_strings)
     {
	edf->fonts = eina_hash_string_small_new(_edje_font_string_free);
	edf->collection = eina_hash_string_small_new(_edje_collection_string_free);
	edf->data = eina_hash_string_small_new((Eina_Free_Cb) eina_stringshare_del);
     }
   else
     {
	edf->fonts = eina_hash_string_small_new(free);
	edf->collection = eina_hash_string_small_new(free);
	edf->data = eina_hash_string_small_new(NULL);
     }

   if (!edf->fonts || !edf->collection || !edf->data)
     goto on_error;

   EINA_LIST_FREE(oedf->data, ed)
     {
	eina_hash_direct_add(edf->data, ed->key, ed->value);
	free(ed);
     }

   EINA_LIST_FOREACH(oedf->collection_dir->entries, l, ce)
     if (ce->entry)
       eina_hash_direct_add(edf->collection, ce->entry, ce);

   if (oedf->font_dir)
     EINA_LIST_FOREACH(oedf->font_dir->entries, l, fnt)
       {
	  char *tmp;
	  int length;

	  length = strlen(fnt->entry) + 7;
	  tmp = alloca(length);

	  snprintf(tmp, length, "fonts/%s", fnt->entry);
	  fnt->path = eina_stringshare_add(tmp);
	  if (edf->free_strings)
	    eina_stringshare_del(fnt->entry);
	  fnt->entry = fnt->path + 6;

	  eina_hash_direct_add(edf->fonts, fnt->entry, fnt);
       }

   if (!_edje_file_convert_images(edf, oedf))
     goto on_error;

   if (!_edje_file_convert_external(edf, oedf))
     goto on_error;

   edf->oef = oedf;
   edf->styles = oedf->styles;
   edf->color_classes = oedf->color_classes;
   edf->version = oedf->version;
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

Edje_Part_Collection *
_edje_collection_convert(Edje_File *file, Old_Edje_Part_Collection *oedc)
{
   Edje_Part_Collection_Directory_Entry *ce;
   Edje_Part_Collection *edc;
   Old_Edje_Part *part;
   Edje_Data *di;
   Eina_List *l;
   unsigned int k;

   ce = eina_hash_find(file->collection, oedc->part);

   /* Count each type part and their respective state */
   EINA_LIST_FOREACH(oedc->parts, l, part)
     {
	int *count;
	int dummy = 0;


	switch (part->type)
	  {
#define CSP(Tp, Ce)						\
	     case EDJE_PART_TYPE_##Tp :				\
		count = &Ce->count.Tp;				\
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
	   default:
	      count = &dummy;
	      break;
	  }

	*count += eina_list_count(part->other_desc) + 1;
     }
   ce->count.part = eina_list_count(oedc->parts);

#define EMN(Tp, Sz, Ce)							\
   Ce->mp.Tp = eina_mempool_add("one_big", #Tp, NULL, sizeof (Sz), Ce->count.Tp);

   EMN(RECTANGLE, Edje_Part_Description_Common, ce);
   EMN(TEXT, Edje_Part_Description_Text, ce);
   EMN(IMAGE, Edje_Part_Description_Image, ce);
   EMN(SWALLOW, Edje_Part_Description_Common, ce);
   EMN(TEXTBLOCK, Edje_Part_Description_Text, ce);
   EMN(GROUP, Edje_Part_Description_Common, ce);
   EMN(BOX, Edje_Part_Description_Box, ce);
   EMN(TABLE, Edje_Part_Description_Table, ce);
   EMN(EXTERNAL, Edje_Part_Description_External, ce);
   EMN(part, Edje_Part, ce);

   /* Change structure layout */
   edc = calloc(1, sizeof (Edje_Part_Collection));
   if (!edc) return NULL;
   ce->ref = edc;

   edc->programs = oedc->programs;
   oedc->programs = NULL;

   edc->data = eina_hash_string_small_new(NULL);
   EINA_LIST_FREE(oedc->data, di)
     {
	eina_hash_direct_add(edc->data, di->key, di->value);
	free(di);
     }

   edc->parts_count = eina_list_count(oedc->parts);
   edc->parts = calloc(edc->parts_count, sizeof (Edje_Part *));
   k = 0;

   EINA_LIST_FREE(oedc->parts, part)
     {
	Old_Edje_Part_Description *oepd;
	Edje_Pack_Element *elm;
	Edje_Part *replacement;
	unsigned int i;

	replacement = eina_mempool_malloc(ce->mp.part, sizeof (Edje_Part));

	replacement->name = part->name;
	replacement->default_desc = _edje_description_convert(part->type, ce, part->default_desc);

	replacement->other_count = eina_list_count(part->other_desc);
	replacement->other_desc = calloc(replacement->other_count, sizeof (Edje_Part_Description_Common*));

	i = 0;
	EINA_LIST_FREE(part->other_desc, oepd)
	  replacement->other_desc[i++] = _edje_description_convert(part->type, ce, oepd);

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
	replacement->items = calloc(replacement->items_count, sizeof (Edje_Pack_Element*));

	i = 0;
	EINA_LIST_FREE(part->items, elm)
	  replacement->items[i++] = elm;

	replacement->type = part->type;
	replacement->effect = part->effect;
	replacement->mouse_events = part->mouse_events;
	replacement->repeat_events = part->repeat_events;
	replacement->ignore_flags = part->ignore_flags;
	replacement->scale = part->scale;
	replacement->precise_is_inside = part->precise_is_inside;
	replacement->use_alternate_font_metrics = part->use_alternate_font_metrics;
	replacement->pointer_mode = part->pointer_mode;
	replacement->entry_mode = part->entry_mode;
	replacement->select_mode = part->select_mode;
	replacement->multiline = part->multiline;
	replacement->api = part->api;

	edc->parts[k++] = replacement;

	free(part);
     }

   edc->id = oedc->id;
   edc->alias = oedc->alias;
   edc->prop.min = oedc->prop.min;
   edc->prop.max = oedc->prop.max;
   edc->script = oedc->script;
   edc->part = oedc->part;
   edc->script_only = oedc->script_only;
   edc->lua_script_only = oedc->lua_script_only;
   edc->checked = oedc->checked;

   free(oedc);

   return edc;
}

Edje_Part_Description_Common*
_edje_description_convert(int type,
			  Edje_Part_Collection_Directory_Entry *ce,
			  Old_Edje_Part_Description *oed)
{
   Edje_Part_Description_Common *result = NULL;

   switch (type)
     {
      case EDJE_PART_TYPE_RECTANGLE:
	 result = eina_mempool_malloc(ce->mp.RECTANGLE, sizeof (Edje_Part_Description_Common));
	 break;
      case EDJE_PART_TYPE_SWALLOW:
	 result = eina_mempool_malloc(ce->mp.SWALLOW, sizeof (Edje_Part_Description_Common));
	 break;
      case EDJE_PART_TYPE_GROUP:
	 result = eina_mempool_malloc(ce->mp.GROUP, sizeof (Edje_Part_Description_Common));
	 break;

#define ALLOC_POOL(Short, Type, Name)					\
	 case EDJE_PART_TYPE_##Short:					\
	   {								\
	      Edje_Part_Description_##Type *Name;			\
	      								\
	      Name = eina_mempool_malloc(ce->mp.Short, sizeof (Edje_Part_Description_##Type)); \
	      Name->Name = oed->Name;					\
	      result = &Name->common;					\
	      break;							\
	   }

	 ALLOC_POOL(IMAGE, Image, image);
	 ALLOC_POOL(TEXT, Text, text);
	 ALLOC_POOL(TEXTBLOCK, Text, text);
	 ALLOC_POOL(BOX, Box, box);
	 ALLOC_POOL(TABLE, Table, table);
	 ALLOC_POOL(EXTERNAL, External, external_params);
     }

   *result = oed->common;

   free(oed);
   return result;
}
