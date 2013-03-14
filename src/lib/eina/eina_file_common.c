/* EINA - EFL data type library
 * Copyright (C) 2013 Cedric Bail
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
# include "config.h"
#endif

#include <string.h>
#include <stdint.h>

#define COPY_BLOCKSIZE (4 * 1024 * 1024)

#include "eina_config.h"
#include "eina_private.h"

#include "eina_hash.h"
#include "eina_safety_checks.h"
#include "eina_file_common.h"

#ifdef HAVE_ESCAPE
# include <Escape.h>
#endif

Eina_Hash *_eina_file_cache = NULL;
Eina_Lock _eina_file_lock_cache;

static char *
_eina_file_escape(char *path, size_t len)
{
   char *result;
   char *p;
   char *q;

   result = path;
   p = result;
   q = result;

   if (!result)
     return NULL;

   while ((p = strchr(p, '/')))
     {
	// remove double `/'
	if (p[1] == '/')
	  {
	     memmove(p, p + 1, --len - (p - result));
	     result[len] = '\0';
	  }
	else
	  if (p[1] == '.'
	      && p[2] == '.')
	    {
	       // remove `/../'
	       if (p[3] == '/')
		 {
		    char tmp;

		    len -= p + 3 - q;
		    memmove(q, p + 3, len - (q - result));
		    result[len] = '\0';
		    p = q;

		    /* Update q correctly. */
		    tmp = *p;
		    *p = '\0';
		    q = strrchr(result, '/');
		    if (!q) q = result;
		    *p = tmp;
		 }
	       else
		 // remove '/..$'
		 if (p[3] == '\0')
		   {
		      len -= p + 2 - q;
		      result[len] = '\0';
                      break;
		   }
		 else
		   {
		      q = p;
		      ++p;
		   }
	    }
        else
          if (p[1] == '.'
              && p[2] == '/')
            {
               // remove '/./'
               len -= 2;
               memmove(p, p + 2, len - (p - result));
               result[len] = '\0';
               q = p;
               ++p;
            }
	  else
	    {
	       q = p;
	       ++p;
	    }
     }

   return result;
}

// Global API

EAPI char *
eina_file_path_sanitize(const char *path)
{
   Eina_Tmpstr *result = NULL;
   size_t len;

   if (!path) return NULL;

   len = strlen(path);

   if (eina_file_path_relative(path))
     {
       result = eina_file_current_directory_get(path, len);
       len = eina_tmpstr_strlen(result);
     }
   else
     result = path;

   return _eina_file_escape(eina_file_cleanup(result), len);
}

EAPI void
eina_file_close(Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN(file);

   eina_lock_take(&file->lock);
   file->refcount--;
   eina_lock_release(&file->lock);
   if (file->refcount != 0) return ;

   eina_lock_take(&_eina_file_lock_cache);

   eina_hash_del(_eina_file_cache, file->filename, file);
   eina_file_real_close(file);

   eina_lock_release(&_eina_file_lock_cache);
}

EAPI size_t
eina_file_size_get(Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, 0);
   return file->length;
}

EAPI time_t
eina_file_mtime_get(Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, 0);
  return file->mtime;
}

EAPI const char *
eina_file_filename_get(Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);
   return file->filename;
}

/* search '\r' and '\n' by preserving cache locality and page locality
   in doing a search inside 4K boundary.
 */
static inline const char *
_eina_fine_eol(const char *start, int boundary, const char *end)
{
   const char *cr;
   const char *lf;
   unsigned long long chunk;

   while (start < end)
     {
        chunk = start + boundary < end ? boundary : end - start;
        cr = memchr(start, '\r', chunk);
        lf = memchr(start, '\n', chunk);
        if (cr)
          {
             if (lf && lf < cr)
               return lf + 1;
             return cr + 1;
          }
        else if (lf)
           return lf + 1;

        start += chunk;
        boundary = 4096;
     }

   return end;
}

static Eina_Bool
_eina_file_map_lines_iterator_next(Eina_Lines_Iterator *it, void **data)
{
   const char *eol;
   unsigned char match;

   if (it->current.end >= it->end)
     return EINA_FALSE;

   match = *it->current.end;
   while ((*it->current.end == '\n' || *it->current.end == '\r')
          && it->current.end < it->end)
     {
        if (match == *it->current.end)
          it->current.index++;
        it->current.end++;
     }
   it->current.index++;

   if (it->current.end == it->end)
     return EINA_FALSE;

   eol = _eina_fine_eol(it->current.end,
                        it->boundary,
                        it->end);
   it->boundary = (uintptr_t) eol & 0x3FF;
   if (it->boundary == 0) it->boundary = 4096;

   it->current.start = it->current.end;

   it->current.end = eol;
   it->current.length = eol - it->current.start - 1;

   *data = &it->current;
   return EINA_TRUE;
}

static Eina_File *
_eina_file_map_lines_iterator_container(Eina_Lines_Iterator *it)
{
   return it->fp;
}

static void
_eina_file_map_lines_iterator_free(Eina_Lines_Iterator *it)
{
   eina_file_map_free(it->fp, (void*) it->map);
   eina_file_close(it->fp);

   EINA_MAGIC_SET(&it->iterator, 0);
   free(it);
}

EAPI Eina_Iterator *
eina_file_map_lines(Eina_File *file)
{
   Eina_Lines_Iterator *it;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);

   if (file->length == 0) return NULL;

   it = calloc(1, sizeof (Eina_Lines_Iterator));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->map = eina_file_map_all(file, EINA_FILE_SEQUENTIAL);
   if (!it->map)
     {
        free(it);
        return NULL;
     }

   eina_lock_take(&file->lock);
   file->refcount++;
   eina_lock_release(&file->lock);

   it->fp = file;
   it->boundary = 4096;
   it->current.start = it->map;
   it->current.end = it->current.start;
   it->current.index = 0;
   it->current.length = 0;
   it->end = it->map + it->fp->length;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_file_map_lines_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eina_file_map_lines_iterator_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_file_map_lines_iterator_free);

   return &it->iterator;
}
