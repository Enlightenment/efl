/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <sys/stat.h>

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#include "edje_private.h"

static Eina_Hash   *_edje_file_hash = NULL;
static int          _edje_file_cache_size = 16;
static Eina_List   *_edje_file_cache = NULL;

static int          _edje_collection_cache_size = 16;

static Edje_Part_Collection *
_edje_file_coll_open(Edje_File *edf, const char *coll)
{
   Edje_Part_Collection *edc = NULL;
   Edje_Part_Collection_Directory_Entry *ce;
   Eina_List *l = NULL;
   int id = -1, size = 0;
   char buf[256];
   void *data;

   EINA_LIST_FOREACH(edf->collection_dir->entries, l, ce)
     {
	if ((ce->entry) && (!strcmp(ce->entry, coll)))
	  {
	     id = ce->id;
	     break;
	  }
     }
   if (id < 0) return NULL;

   snprintf(buf, sizeof(buf), "collections/%i", id);
   edc = eet_data_read(edf->ef, _edje_edd_edje_part_collection, buf);
   if (!edc) return NULL;

   snprintf(buf, sizeof(buf), "scripts/%i", id);
   data = eet_read(edf->ef, buf, &size);

   if (data)
     {
	edc->script = embryo_program_new(data, size);
	free(data);
     }

   edc->part = eina_stringshare_add(coll);
   edc->references = 1;
   edf->collection_hash = evas_hash_add(edf->collection_hash, coll, edc);
   return edc;
}

static int
_edje_font_hash(Edje_File *edf)
{
   int	count = 0;

   if (edf->font_dir)
     {
	Eina_List *l;
	Edje_Font_Directory_Entry  *fnt;

	EINA_LIST_FOREACH(edf->font_dir->entries, l, fnt)
	  {
	     int                         length;
	     char                       *tmp;

	     length = strlen(fnt->entry) + 7;
	     tmp = alloca(length);

	     snprintf(tmp, length, "fonts/%s", fnt->entry);
	     fnt->path = eina_stringshare_add(tmp);
             if (edf->free_strings)
               eina_stringshare_del(fnt->entry);
	     fnt->entry = fnt->path + 6;
	     edf->font_hash = evas_hash_direct_add(edf->font_hash, fnt->entry, fnt);

	     count++;
	  }
     }
   return count;
}

static Edje_File *
_edje_file_open(const char *file, const char *coll, int *error_ret, Edje_Part_Collection **edc_ret)
{
   Edje_File *edf;
   Edje_Part_Collection *edc;
   Edje_Data *di;
   Eet_File *ef;
   Eina_List *l;
   struct stat st;

   if (stat(file, &st) != 0)
     {
	*error_ret = EDJE_LOAD_ERROR_DOES_NOT_EXIST;
	return NULL;
     }

   ef = eet_open(file, EET_FILE_MODE_READ);
   if (!ef)
     {
	*error_ret = EDJE_LOAD_ERROR_UNKNOWN_FORMAT;
	return NULL;
     }
   edf = eet_data_read(ef, _edje_edd_edje_file, "edje_file");
   if (!edf)
     {
	*error_ret = EDJE_LOAD_ERROR_CORRUPT_FILE;
	eet_close(ef);
	return NULL;
     }

   edf->free_strings = eet_dictionary_get(ef) ? 0 : 1;

   edf->ef = ef;

   if (edf->version != EDJE_FILE_VERSION)
     {
	*error_ret = EDJE_LOAD_ERROR_INCOMPATIBLE_FILE;
	_edje_file_free(edf);
	return NULL;
     }
   if (!edf->collection_dir)
     {
	*error_ret = EDJE_LOAD_ERROR_CORRUPT_FILE;
	_edje_file_free(edf);
	return NULL;
     }

   edf->path = eina_stringshare_add(file);
   edf->references = 1;

   _edje_textblock_style_parse_and_fix(edf);

   EINA_LIST_FOREACH(edf->data, l, di)
     edf->data_cache = evas_hash_add(edf->data_cache, eina_stringshare_add(di->key), di->value);

   if (coll)
     {
	edc = _edje_file_coll_open(edf, coll);
	if (!edc)
	  {
	     *error_ret = EDJE_LOAD_ERROR_UNKNOWN_COLLECTION;
	  }
	if (edc_ret) *edc_ret = edc;
     }

   edf->font_hash = NULL;

   _edje_font_hash(edf);

   return edf;
}

Edje_File *
_edje_cache_file_coll_open(const char *file, const char *coll, int *error_ret, Edje_Part_Collection **edc_ret)
{
   Edje_File *edf;
   Eina_List *l, *hist;
   Edje_Part_Collection *edc;
   Edje_Part *ep;

   edf = evas_hash_find(_edje_file_hash, file);
   if (edf)
     {
	edf->references++;
     }
   else
     {
        EINA_LIST_FOREACH(_edje_file_cache, l, edf)
	  {
	     if (!strcmp(edf->path, file))
	       {
		  edf->references = 1;
		  _edje_file_cache = eina_list_remove_list(_edje_file_cache, l);
		  _edje_file_hash = evas_hash_add(_edje_file_hash, file, edf);
		  break;
	       }
	     edf = NULL;
	  }
     }
   if (!edf)
     {
	edf = _edje_file_open(file, coll, error_ret, edc_ret);
	if (!edf) return NULL;
	_edje_file_hash = evas_hash_add(_edje_file_hash, file, edf);
	return edf;
     }

   if (!coll) return edf;

   edc = evas_hash_find(edf->collection_hash, coll);
   if (edc)
     {
	edc->references++;
     }
   else
     {
        EINA_LIST_FOREACH(edf->collection_cache, l, edc)
	  {
	     if (!strcmp(edc->part, coll))
	       {
		  edc->references = 1;
		  edf->collection_cache = eina_list_remove_list(edf->collection_cache, l);
		  edf->collection_hash = evas_hash_add(edf->collection_hash, coll, edc);
		  break;
	       }
	     edc = NULL;
	  }
     }
   if (!edc)
     {
	edc = _edje_file_coll_open(edf, coll);
	if (!edc)
	  {
	     *error_ret = EDJE_LOAD_ERROR_UNKNOWN_COLLECTION;
	  }
	else
	  {
	    EINA_LIST_FOREACH(edc->parts, l, ep)
	       {
		  Edje_Part *ep2;

		  /* Register any color classes in this parts descriptions. */
		  hist = NULL;
		  hist = eina_list_append(hist, ep);
		  ep2 = ep;
		  while (ep2->dragable.confine_id >= 0)
		    {
		       ep2 = eina_list_nth(edc->parts, ep2->dragable.confine_id);
		       if (eina_list_data_find(hist, ep2))
			 {
			    printf("EDJE ERROR: confine_to loops. invalidating loop.\n");
			    ep2->dragable.confine_id = -1;
			    break;
			 }
		       hist = eina_list_append(hist, ep2);
		    }
		  eina_list_free(hist);
		  hist = NULL;
		  hist = eina_list_append(hist, ep);
		  ep2 = ep;
		  while (ep2->dragable.events_id >= 0)
		    {
		       Edje_Part* prev;

		       prev = ep2;

		       ep2 = eina_list_nth(edc->parts, ep2->dragable.events_id);
		       if (!ep2->dragable.x && !ep2->dragable.y)
			 {
			    prev->dragable.events_id = -1;
			    break;
			 }

		       if (eina_list_data_find(hist, ep2))
			 {
			    printf("EDJE ERROR: events_to loops. invalidating loop.\n");
			    ep2->dragable.events_id = -1;
			    break;
			 }
		       hist = eina_list_append(hist, ep2);
		    }
		  eina_list_free(hist);
		  hist = NULL;
		  hist = eina_list_append(hist, ep);
		  ep2 = ep;
		  while (ep2->clip_to_id >= 0)
		    {
		       ep2 = eina_list_nth(edc->parts, ep2->clip_to_id);
		       if (eina_list_data_find(hist, ep2))
			 {
			    printf("EDJE ERROR: clip_to loops. invalidating loop.\n");
			    ep2->clip_to_id = -1;
			    break;
			 }
		       hist = eina_list_append(hist, ep2);
		    }
		  eina_list_free(hist);
		  hist = NULL;
	       }
	  }
     }
   if (edc_ret) *edc_ret = edc;

   return edf;
}

void
_edje_cache_coll_clean(Edje_File *edf)
{
   int count;

   count = eina_list_count(edf->collection_cache);
   while ((edf->collection_cache) && (count > _edje_collection_cache_size))
     {
	Edje_Part_Collection *edc;

	edc = eina_list_data_get(eina_list_last(edf->collection_cache));
	edf->collection_cache = eina_list_remove_list(edf->collection_cache, eina_list_last(edf->collection_cache));
	_edje_collection_free(edf, edc);
	count = eina_list_count(edf->collection_cache);
     }
}

void
_edje_cache_coll_flush(Edje_File *edf)
{
   while (edf->collection_cache)
     {
	Edje_Part_Collection *edc;

	edc = eina_list_data_get(eina_list_last(edf->collection_cache));
	edf->collection_cache = eina_list_remove_list(edf->collection_cache, eina_list_last(edf->collection_cache));
	_edje_collection_free(edf, edc);
     }
}

void
_edje_cache_coll_unref(Edje_File *edf, Edje_Part_Collection *edc)
{
   edc->references--;
   if (edc->references != 0) return;
   edf->collection_hash = evas_hash_del(edf->collection_hash, edc->part, edc);
   edf->collection_cache = eina_list_prepend(edf->collection_cache, edc);
   _edje_cache_coll_clean(edf);
}

static void
_edje_cache_file_clean(void)
{
   int count;

   count = eina_list_count(_edje_file_cache);
   while ((_edje_file_cache) && (count > _edje_file_cache_size))
     {
	Edje_File *edf;

	edf = eina_list_data_get(eina_list_last(_edje_file_cache));
	_edje_file_cache = eina_list_remove_list(_edje_file_cache, eina_list_last(_edje_file_cache));
	_edje_file_free(edf);
	count = eina_list_count(_edje_file_cache);
     }
}

void
_edje_cache_file_unref(Edje_File *edf)
{
   edf->references--;
   if (edf->references != 0) return;
   _edje_file_hash = evas_hash_del(_edje_file_hash, edf->path, edf);
   _edje_file_cache = eina_list_prepend(_edje_file_cache, edf);
   _edje_cache_file_clean();
}

void
_edje_file_cache_shutdown(void)
{
   edje_file_cache_flush();
}






EAPI void
edje_file_cache_set(int count)
{
   if (count < 0) count = 0;
   _edje_file_cache_size = count;
   _edje_cache_file_clean();
}

EAPI int
edje_file_cache_get(void)
{
   return _edje_file_cache_size;
}

EAPI void
edje_file_cache_flush(void)
{
   int ps;

   ps = _edje_file_cache_size;
   _edje_file_cache_size = 0;
   _edje_cache_file_clean();
   _edje_file_cache_size = ps;
}

EAPI void
edje_collection_cache_set(int count)
{
   Eina_List *l;
   Edje_File *edf;

   if (count < 0) count = 0;
   _edje_collection_cache_size = count;
   EINA_LIST_FOREACH(_edje_file_cache, l, edf)
     _edje_cache_coll_clean(edf);
   /* FIXME: freach in file hash too! */
}

EAPI int
edje_collection_cache_get(void)
{
   return _edje_collection_cache_size;
}

EAPI void
edje_collection_cache_flush(void)
{
   int ps;
   Eina_List *l;
   Edje_File *edf;

   ps = _edje_collection_cache_size;
   _edje_collection_cache_size = 0;
   EINA_LIST_FOREACH(_edje_file_cache, l, edf)
     _edje_cache_coll_flush(edf);
   /* FIXME: freach in file hash too! */
   _edje_collection_cache_size = ps;
}
