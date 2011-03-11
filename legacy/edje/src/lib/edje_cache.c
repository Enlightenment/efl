#include "edje_private.h"


/**
 * @cond
 */

static Eina_Hash   *_edje_file_hash = NULL;
static int          _edje_file_cache_size = 16;
static Eina_List   *_edje_file_cache = NULL;

static int          _edje_collection_cache_size = 16;

static Edje_Part_Collection *
_edje_file_coll_open(Edje_File *edf, const char *coll)
{
   Edje_Part_Collection *edc = NULL;
   Edje_Part_Collection_Directory_Entry *ce;
   int id = -1, size = 0;
   Eina_List *l;
   char buf[256];
   char *buffer;
   void *data;

   ce = eina_hash_find(edf->collection, coll);
   if (!ce) return NULL;

   if (ce->ref)
     {
	ce->ref->references++;
	return ce->ref;
     }

   EINA_LIST_FOREACH(edf->collection_cache, l, edc)
     {
	if (!strcmp(edc->part, coll))
	  {
	     edc->references = 1;
	     ce->ref = edc;

	     edf->collection_cache = eina_list_remove_list(edf->collection_cache, l);
	     return ce->ref;
	  }
     }

   id = ce->id;
   if (id < 0) return NULL;

#define INIT_EMP(Tp, Sz, Ce)                                           \
   buffer = alloca(strlen(ce->entry) + strlen(#Tp) + 2);               \
   sprintf(buffer, "%s/%s", ce->entry, #Tp);                           \
   Ce->mp.Tp = eina_mempool_add("one_big", buffer, NULL, sizeof (Sz), Ce->count.Tp); \
   _emp_##Tp = Ce->mp.Tp;

#define INIT_EMP_BOTH(Tp, Sz, Ce)                                       \
   INIT_EMP(Tp, Sz, Ce)                                                 \
   Ce->mp_rtl.Tp = eina_mempool_add("one_big", buffer, NULL,            \
         sizeof (Sz), Ce->count.Tp);

   INIT_EMP_BOTH(RECTANGLE, Edje_Part_Description_Common, ce);
   INIT_EMP_BOTH(TEXT, Edje_Part_Description_Text, ce);
   INIT_EMP_BOTH(IMAGE, Edje_Part_Description_Image, ce);
   INIT_EMP_BOTH(PROXY, Edje_Part_Description_Proxy, ce);
   INIT_EMP_BOTH(SWALLOW, Edje_Part_Description_Common, ce);
   INIT_EMP_BOTH(TEXTBLOCK, Edje_Part_Description_Text, ce);
   INIT_EMP_BOTH(GROUP, Edje_Part_Description_Common, ce);
   INIT_EMP_BOTH(BOX, Edje_Part_Description_Box, ce);
   INIT_EMP_BOTH(TABLE, Edje_Part_Description_Table, ce);
   INIT_EMP_BOTH(EXTERNAL, Edje_Part_Description_External, ce);
   INIT_EMP(part, Edje_Part, ce);

   snprintf(buf, sizeof(buf), "edje/collections/%i", id);
   edc = eet_data_read(edf->ef, _edje_edd_edje_part_collection, buf);
   if (!edc) return NULL;

   edc->references = 1;
   edc->part = ce->entry;

   snprintf(buf, sizeof(buf), "edje/scripts/embryo/compiled/%i", id);
   data = eet_read(edf->ef, buf, &size);

   if (data)
     {
	edc->script = embryo_program_new(data, size);
	_edje_embryo_script_init(edc);
	free(data);
     }

   snprintf(buf, sizeof(buf), "edje/scripts/lua/%i", id);
   data = eet_read(edf->ef, buf, &size);

   if (data)
     {
        _edje_lua2_script_load(edc, data, size);
	free(data);
     }

   ce->ref = edc;

   return edc;
}

static Edje_File *
_edje_file_open(const char *file, const char *coll, int *error_ret, Edje_Part_Collection **edc_ret)
{
   Edje_File *edf;
   Edje_Part_Collection *edc;
   Eet_File *ef;
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
   edf = eet_data_read(ef, _edje_edd_edje_file, "edje/file");
   if (!edf)
     {
	*error_ret = EDJE_LOAD_ERROR_CORRUPT_FILE;
	eet_close(ef);
	return NULL;
     }

   edf->ef = ef;
   edf->mtime = st.st_mtime;

   if (edf->version != EDJE_FILE_VERSION)
     {
	*error_ret = EDJE_LOAD_ERROR_INCOMPATIBLE_FILE;
	_edje_file_free(edf);
	return NULL;
     }
   if (!edf->collection)
     {
	*error_ret = EDJE_LOAD_ERROR_CORRUPT_FILE;
	_edje_file_free(edf);
	return NULL;
     }

   if (edf->minor > EDJE_FILE_MINOR)
     {
	WRN("`%s` may use feature from a newer edje and could not show up as expected.", file);
     }

   edf->path = eina_stringshare_add(file);
   edf->references = 1;

   /* This should be done at edje generation time */
   _edje_textblock_style_parse_and_fix(edf);

   if (coll)
     {
	edc = _edje_file_coll_open(edf, coll);
	if (!edc)
	  {
	     *error_ret = EDJE_LOAD_ERROR_UNKNOWN_COLLECTION;
	  }
	if (edc_ret) *edc_ret = edc;
     }

   return edf;
}

static void
_edje_file_dangling(Edje_File *edf)
{
   if (edf->dangling) return;
   edf->dangling = EINA_TRUE;

   eina_hash_del(_edje_file_hash, edf->path, edf);
   if (!eina_hash_population(_edje_file_hash))
     {
       eina_hash_free(_edje_file_hash);
       _edje_file_hash = NULL;
     }
}

Edje_File *
_edje_cache_file_coll_open(const char *file, const char *coll, int *error_ret, Edje_Part_Collection **edc_ret)
{
   Edje_File *edf;
   Eina_List *l, *hist;
   Edje_Part_Collection *edc;
   Edje_Part *ep;
   struct stat st;

   if (stat(file, &st) != 0)
      return NULL;

   if (!_edje_file_hash)
     {
	_edje_file_hash = eina_hash_string_small_new(NULL);
	goto open_new;
     }

   edf = eina_hash_find(_edje_file_hash, file);
   if (edf)
     {
	if (edf->mtime != st.st_mtime)
	  {
	     _edje_file_dangling(edf);
	     goto open_new;
	  }

	edf->references++;
	goto open;
     }

   EINA_LIST_FOREACH(_edje_file_cache, l, edf)
     {
	if (!strcmp(edf->path, file))
	  {
	     if (edf->mtime != st.st_mtime)
	       {
		  _edje_file_cache = eina_list_remove_list(_edje_file_cache, l);
		  _edje_file_free(edf);
		  goto open_new;
	       }

	     edf->references = 1;
	     _edje_file_cache = eina_list_remove_list(_edje_file_cache, l);
	     eina_hash_add(_edje_file_hash, file, edf);
	     goto open;
	  }
     }

open_new:
   if (!_edje_file_hash)
      _edje_file_hash = eina_hash_string_small_new(NULL);

   edf = _edje_file_open(file, coll, error_ret, edc_ret);
   if (!edf)
      return NULL;

   eina_hash_add(_edje_file_hash, file, edf);
   return edf;

open:
   if (!coll)
      return edf;

   edc = _edje_file_coll_open(edf, coll);
   if (!edc)
     {
	*error_ret = EDJE_LOAD_ERROR_UNKNOWN_COLLECTION;
     }
   else
     {
	if (!edc->checked)
	  {
	     unsigned int j;

	     for (j = 0; j < edc->parts_count; ++j)
	       {
		  Edje_Part *ep2;

		  ep = edc->parts[j];

		  /* Register any color classes in this parts descriptions. */
		  hist = NULL;
		  hist = eina_list_append(hist, ep);
		  ep2 = ep;
		  while (ep2->dragable.confine_id >= 0)
		    {
		       if (ep2->dragable.confine_id >= (int) edc->parts_count)
			 {
			    ERR("confine_to above limit. invalidating it.");
			    ep2->dragable.confine_id = -1;
			    break;
			 }

		       ep2 = edc->parts[ep2->dragable.confine_id];
		       if (eina_list_data_find(hist, ep2))
			 {
			    ERR("confine_to loops. invalidating loop.");
			    ep2->dragable.confine_id = -1;
			    break;
			 }
		       hist = eina_list_append(hist, ep2);
		    }
		  eina_list_free(hist);
		  hist = NULL;
		  hist = eina_list_append(hist, ep);
		  ep2 = ep;
		  while (ep2->dragable.event_id >= 0)
		    {
		       Edje_Part* prev;

		       if (ep2->dragable.event_id >= (int) edc->parts_count)
			 {
			    ERR("event_id above limit. invalidating it.");
			    ep2->dragable.event_id = -1;
			    break;
			 }
		       prev = ep2;

		       ep2 = edc->parts[ep2->dragable.event_id];
		       if (!ep2->dragable.x && !ep2->dragable.y)
			 {
			    prev->dragable.event_id = -1;
			    break;
			 }

		       if (eina_list_data_find(hist, ep2))
			 {
			    ERR("events_to loops. invalidating loop.");
			    ep2->dragable.event_id = -1;
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
		       if (ep2->clip_to_id >= (int) edc->parts_count)
			 {
			    ERR("clip_to_id above limit. invalidating it.");
			    ep2->clip_to_id = -1;
			    break;
			 }

		       ep2 = edc->parts[ep2->clip_to_id];
		       if (eina_list_data_find(hist, ep2))
			 {
			    ERR("clip_to loops. invalidating loop.");
			    ep2->clip_to_id = -1;
			    break;
			 }
		       hist = eina_list_append(hist, ep2);
		    }
		  eina_list_free(hist);
		  hist = NULL;
	       }
	    edc->checked = 1;
	  }
     }
   if (edc_ret) *edc_ret = edc;

   return edf;
}

void
_edje_cache_coll_clean(Edje_File *edf)
{
   while ((edf->collection_cache) &&
	  (eina_list_count(edf->collection_cache) > (unsigned int) _edje_collection_cache_size))
     {
	Edje_Part_Collection_Directory_Entry *ce;
	Edje_Part_Collection *edc;

	edc = eina_list_data_get(eina_list_last(edf->collection_cache));
	edf->collection_cache = eina_list_remove_list(edf->collection_cache, eina_list_last(edf->collection_cache));

	ce = eina_hash_find(edf->collection, edc->part);
	_edje_collection_free(edf, edc, ce);
     }
}

void
_edje_cache_coll_flush(Edje_File *edf)
{
   while (edf->collection_cache)
     {
	Edje_Part_Collection_Directory_Entry *ce;
	Edje_Part_Collection *edc;
	Eina_List *last;

	last = eina_list_last(edf->collection_cache);
	edc = eina_list_data_get(last);
	edf->collection_cache = eina_list_remove_list(edf->collection_cache,
						      last);

	ce = eina_hash_find(edf->collection, edc->part);
	_edje_collection_free(edf, edc, ce);
     }
}

void
_edje_cache_coll_unref(Edje_File *edf, Edje_Part_Collection *edc)
{
   Edje_Part_Collection_Directory_Entry *ce;

   edc->references--;
   if (edc->references != 0) return;

   ce = eina_hash_find(edf->collection, edc->part);
   if (!ce)
     {
	ERR("Something is wrong with reference count of '%s'.", edc->part);
     }
   else if (ce->ref)
     {
	ce->ref = NULL;

	if (edf->dangling)
	  {
	     /* No need to keep the collection around if the file is dangling */
	     _edje_collection_free(edf, edc, ce);
	     _edje_cache_coll_flush(edf);
	  }
	else
	  {
	     edf->collection_cache = eina_list_prepend(edf->collection_cache, edc);
	     _edje_cache_coll_clean(edf);
	  }
     }
}

static void
_edje_cache_file_clean(void)
{
   int count;

   count = eina_list_count(_edje_file_cache);
   while ((_edje_file_cache) && (count > _edje_file_cache_size))
     {
	Eina_List *last;
	Edje_File *edf;

	last = eina_list_last(_edje_file_cache);
	edf = eina_list_data_get(last);
	_edje_file_cache = eina_list_remove_list(_edje_file_cache, last);
	_edje_file_free(edf);
	count = eina_list_count(_edje_file_cache);
     }
}

void
_edje_cache_file_unref(Edje_File *edf)
{
   edf->references--;
   if (edf->references != 0) return;

   if (edf->dangling)
     {
	_edje_file_free(edf);
	return;
     }

   eina_hash_del(_edje_file_hash, edf->path, edf);
   if (!eina_hash_population(_edje_file_hash))
     {
       eina_hash_free(_edje_file_hash);
       _edje_file_hash = NULL;
     }
   _edje_file_cache = eina_list_prepend(_edje_file_cache, edf);
   _edje_cache_file_clean();
}

void
_edje_file_cache_shutdown(void)
{
   edje_file_cache_flush();
}



/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Edje_cache_Group Cache
 *
 * @brief These functions provide an abstraction layer between the
 * application code and the interface, while allowing extremely
 * flexible dynamic layouts and animations.
 *
 * @{
 */

/**
 * @brief Set the file cache size.
 *
 * @param count The file cache size in edje file units. Default is 16.
 *
 * This function sets the file cache size. Edje keeps this cache in
 * order to prevent duplicates of edje file entries in memory. The
 * file cache size can be retrieved with edje_file_cache_get().
 *
 * @see edje_file_cache_get()
 * @see edje_file_cache_flush()
 *
 */

EAPI void
edje_file_cache_set(int count)
{
   if (count < 0) count = 0;
   _edje_file_cache_size = count;
   _edje_cache_file_clean();
}

/**
 * @brief Return the file cache size.
 *
 * @return The file cache size in edje file units. Default is 16.
 *
 * This function returns the file cache size set by
 * edje_file_cache_set().
 *
 * @see edje_file_cache_set()
 * @see edje_file_cache_flush()
 *
 */

EAPI int
edje_file_cache_get(void)
{
   return _edje_file_cache_size;
}

/**
 * @brief Clean the file cache.
 *
 * This function cleans the file cache entries, but keeps this cache's
 * size to the last value set.
 *
 * @see edje_file_cache_set()
 * @see edje_file_cache_get()
 *
 */

EAPI void
edje_file_cache_flush(void)
{
   int ps;

   ps = _edje_file_cache_size;
   _edje_file_cache_size = 0;
   _edje_cache_file_clean();
   _edje_file_cache_size = ps;
}

/**
 * @brief Set the collection cache size.
 *
 * @param count The collection cache size, in edje object units. Default is 16.
 *
 * This function sets the collection cache size. Edje keeps this cache
 * in order to prevent duplicates of edje {collection,group,part}
 * entries in memory. The collection cache size can be retrieved with
 * edje_collection_cache_get().
 *
 * @see edje_collection_cache_get()
 * @see edje_collection_cache_flush()
 *
 */

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

/**
 * @brief Return the collection cache size.
 *
 * @return The collection cache size, in edje object units. Default is 16.
 *
 * This function returns the collection cache size set by
 * edje_collection_cache_set().
 *
 * @see edje_collection_cache_set()
 * @see edje_collection_cache_flush()
 *
 */

EAPI int
edje_collection_cache_get(void)
{
   return _edje_collection_cache_size;
}

/**
 * @brief Clean the collection cache.
 *
 * This function cleans the collection cache, but keeps this cache's
 * size to the last value set.
 *
 * @see edje_collection_cache_set()
 * @see edje_collection_cache_get()
 *
 */

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

/**
 *
 * @}
 */
