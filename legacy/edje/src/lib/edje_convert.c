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

	_edje_collection_free(edf, ce->ref);
     }

   free(ce);
}

Edje_File *
_edje_file_convert(Eet_File *file, Old_Edje_File *oedf)
{
   Edje_Part_Collection_Directory_Entry *ce;
   Edje_Font_Directory_Entry *fnt;
   Edje_File *edf;
   Eina_List *l;
   Edje_Data *ed;

   edf = malloc(sizeof (Edje_File));
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

   edf->oef = oedf;
   edf->external_dir = oedf->external_dir;
   edf->image_dir = oedf->image_dir;
   edf->spectrum_dir = oedf->spectrum_dir;
   edf->styles = oedf->styles;
   edf->color_classes = oedf->color_classes;
   edf->version = oedf->version;
   edf->feature_ver = oedf->feature_ver;
   edf->compiler = oedf->compiler;

   edf->dangling = EINA_FALSE;
   edf->warning = EINA_FALSE;

   edf->collection_cache = NULL;

   return edf;
}

Edje_Part_Collection *
_edje_collection_convert(Edje_File *file, Old_Edje_Part_Collection *oedc)
{
   Edje_Part_Collection_Directory_Entry *ce;
   Edje_Part_Collection *edc;

   edc = oedc;

   ce = eina_hash_find(file->collection, oedc->part);

   ce->ref = edc;

   /* FIXME : Count type part and change their structure */
   return edc;
}
