/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* EDJE - EFL graphical design and layout library based on Evas
 * Copyright (C) 2008 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

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

#include <string.h>
#include <sys/stat.h>

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
        _edje_embryo_script_init(edc);
	free(data);
     }

   snprintf(buf, sizeof(buf), "lua_scripts/%i", id);
   data = eet_read(edf->ef, buf, &size);

   if (data)
     {
#ifdef LUA2
        _edje_lua2_script_load(edc, data, size);
#else        
	int err_code;

	//printf("lua chunk size: %d\n", size);
	_edje_lua_new_reg(_edje_lua_state_get(), -1, edc); // gets freed in 'edje_load::_edje_collectoin_free'

	if ((err_code = luaL_loadbuffer(_edje_lua_state_get(), data, size, "edje_lua_script")))
	  {
	     if (err_code == LUA_ERRSYNTAX)
	       ERR("lua load syntax error: %s", lua_tostring(_edje_lua_state_get(), -1));
	     else if (err_code == LUA_ERRMEM)
	       ERR("lua load memory allocation error: %s", lua_tostring(_edje_lua_state_get(), -1));
	  }
	if (lua_pcall(_edje_lua_state_get(), 0, 0, 0))
	  ERR("lua call error: %s", lua_tostring(_edje_lua_state_get(), -1));
#endif        
	free(data);
     }
   
   edc->part = eina_stringshare_add(coll);
   edc->references = 1;
   if (!edf->collection_hash)
     edf->collection_hash = eina_hash_string_small_new(NULL);
   eina_hash_add(edf->collection_hash, coll, edc);
   return edc;
}

static int
_edje_font_hash(Edje_File *edf)
{
   int	count = 0;

   if (!edf->font_hash)
     edf->font_hash = eina_hash_string_small_new(NULL);

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
	     eina_hash_direct_add(edf->font_hash, fnt->entry, fnt);

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
   edf->mtime = st.st_mtime;

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

   if (!edf->data_cache)
     edf->data_cache = eina_hash_string_small_new(NULL);

   EINA_LIST_FOREACH(edf->data, l, di)
     eina_hash_add(edf->data_cache, di->key, di->value);

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
   Edje_File *edf = NULL;
   Eina_List *l, *hist;
   Edje_Part_Collection *edc;
   Edje_Part *ep;
   struct stat st;

   if (stat(file, &st) != 0)
     {
	return NULL;
     }

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
	     edf = NULL;
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
		  edf = NULL;
		  goto open_new;
	       }

	     edf->references = 1;
	     _edje_file_cache = eina_list_remove_list(_edje_file_cache, l);
	     eina_hash_add(_edje_file_hash, file, edf);
	     break;
	  }
	edf = NULL;
     }

 open_new:
   if (!edf)
     {
	if (!_edje_file_hash)
	  _edje_file_hash = eina_hash_string_small_new(NULL);
	edf = _edje_file_open(file, coll, error_ret, edc_ret);
	if (!edf) return NULL;
	eina_hash_add(_edje_file_hash, file, edf);
	return edf;
     }

 open:

   if (!coll) return edf;

   if (!edf->collection_hash)
     edf->collection_hash = eina_hash_string_small_new(NULL);

   edc = eina_hash_find(edf->collection_hash, coll);
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
		  eina_hash_add(edf->collection_hash, coll, edc);
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
			    ERR("events_to loops. invalidating loop.");
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
			    ERR("clip_to loops. invalidating loop.");
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

   if (eina_hash_population(edf->collection_hash) == 0)
     {
       eina_hash_free(edf->collection_hash);
       edf->collection_hash = NULL;
     }
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
   eina_hash_del(edf->collection_hash, edc->part, edc);
   if (eina_hash_population(edf->collection_hash) == 0)
     {
       eina_hash_free(edf->collection_hash);
       edf->collection_hash = NULL;
     }
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
 * @return The file cache size.
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
 * @return The collection cache size.
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
