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
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef _WIN32
# include <evil_private.h> /* windows.h fcntl mkstemps mkdtemp */
#endif

#define COPY_BLOCKSIZE (4 * 1024 * 1024)

#include "eina_config.h"
#include "eina_private.h"

#include "eina_hash.h"
#include "eina_safety_checks.h"
#include "eina_file_common.h"
#include "eina_xattr.h"

#ifdef HAVE_ESCAPE
# include <Escape.h>
#endif

#ifndef O_BINARY
# define O_BINARY 0
#endif

#ifdef MAP_FAILED
# undef MAP_FAILED
#endif
#define MAP_FAILED ((void *)-1)

Eina_Hash *_eina_file_cache = NULL;
Eina_Lock _eina_file_lock_cache;

#if defined(EINA_SAFETY_CHECKS) && defined(EINA_MAGIC_DEBUG)
# define EINA_FILE_MAGIC_CHECK(f, ...) do { \
   if (EINA_UNLIKELY((f) == NULL)) \
     { \
       EINA_SAFETY_ERROR("safety check failed: " # f " == NULL"); \
       return __VA_ARGS__; \
     } \
   if (EINA_UNLIKELY((f)->__magic != EINA_FILE_MAGIC)) \
     { \
        EINA_MAGIC_FAIL(f, EINA_FILE_MAGIC); \
        return __VA_ARGS__; \
     } \
   } while (0)
#else
# define EINA_FILE_MAGIC_CHECK(f, ...) do {} while(0)
#endif

static Eina_Spinlock _eina_statgen_lock;
static Eina_Statgen _eina_statgen = 0;

EAPI void
eina_file_statgen_next(void)
{
   eina_spinlock_take(&_eina_statgen_lock);
   if (_eina_statgen != 0)
     {
        _eina_statgen++;
        if (_eina_statgen == 0) _eina_statgen = 1;
     }
   eina_spinlock_release(&_eina_statgen_lock);
}

EAPI Eina_Statgen
eina_file_statgen_get(void)
{
   Eina_Statgen s;
   eina_spinlock_take(&_eina_statgen_lock);
   s = _eina_statgen;
   eina_spinlock_release(&_eina_statgen_lock);
   return s;
}

EAPI void
eina_file_statgen_enable(void)
{
   eina_spinlock_take(&_eina_statgen_lock);
   if (_eina_statgen != 0) _eina_statgen = 1;
   eina_spinlock_release(&_eina_statgen_lock);
}

EAPI void
eina_file_statgen_disable(void)
{
   eina_spinlock_take(&_eina_statgen_lock);
   _eina_statgen = 0;
   eina_spinlock_release(&_eina_statgen_lock);
}

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

#ifdef _WIN32
   EVIL_PATH_SEP_WIN32_TO_UNIX(path);
#endif

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


unsigned int
eina_file_map_key_length(const void *key EINA_UNUSED)
{
   return sizeof (unsigned long int) * 2;
}

int
eina_file_map_key_cmp(const unsigned long long int *key1, int key1_length EINA_UNUSED,
                       const unsigned long long int *key2, int key2_length EINA_UNUSED)
{
   if (key1[0] == key2[0])
     {
        if (key1[1] == key2[1]) return 0;
        if (key1[1] > key2[1]) return 1;
        return -1;
     }
   if (key1[0] > key2[0]) return 1;
   return -1;
}

int
eina_file_map_key_hash(const unsigned long long int *key, int key_length EINA_UNUSED)
{
   return eina_hash_int64(&key[0], sizeof (unsigned long long int))
     ^ eina_hash_int64(&key[1], sizeof (unsigned long long int));
}

void *
eina_file_virtual_map_all(Eina_File *file)
{
   eina_lock_take(&file->lock);
   file->global_refcount++;
   eina_lock_release(&file->lock);

   return file->global_map;
}

void *
eina_file_virtual_map_new(Eina_File *file,
                          unsigned long int offset, unsigned long int length)
{
   Eina_File_Map *map;
   unsigned long int key[2];

   // offset and length has already been checked by the caller function

   key[0] = offset;
   key[1] = length;

   eina_lock_take(&file->lock);

   map = eina_hash_find(file->map, &key);
   if (!map)
     {
        map = malloc(sizeof (Eina_File_Map));
        if (!map) goto on_error;

        map->map = ((char*) file->global_map) + offset;
        map->offset = offset;
        map->length = length;
        map->refcount = 0;

        eina_hash_add(file->map, &key, map);
        eina_hash_direct_add(file->rmap, &map->map, map);
     }

   map->refcount++;

 on_error:
   eina_lock_release(&file->lock);
   return map ? map->map : NULL;
}

void
eina_file_virtual_map_free(Eina_File *file, void *map)
{
   Eina_File_Map *em;

   eina_lock_take(&file->lock);

   // map could equal global_map even if length != file->length
   em = eina_hash_find(file->rmap, &map);
   if (em)
     {
        unsigned long int key[2];

        em->refcount--;

        if (em->refcount > 0) goto on_exit;

        key[0] = em->offset;
        key[1] = em->length;

        eina_hash_del(file->rmap, &map, em);
        eina_hash_del(file->map, &key, em);
     }
   else
     {
        if (file->global_map == map)
          {
             file->global_refcount--;
          }
     }

 on_exit:
   eina_lock_release(&file->lock);
}

void
eina_file_common_map_free(Eina_File *file, void *map,
                          void (*free_func)(Eina_File_Map *map))
{
   Eina_File_Map *em;
   unsigned long int key[2];
   Eina_List *l = NULL;
   Eina_Bool hashed = EINA_TRUE;

   em = eina_hash_find(file->rmap, &map);
   if (!em)
     {
        EINA_LIST_FOREACH(file->dead_map, l, em)
          if (em->map == map)
            {
               hashed = EINA_FALSE;
               break ;
            }
        if (hashed) return ;
     }

   em->refcount--;

   if (em->refcount > 0) return ;

   key[0] = em->offset;
   key[1] = em->length;

   if (hashed)
     {
        eina_hash_del(file->rmap, &map, em);
        eina_hash_del(file->map, &key, em);
     }
   else
     {
        file->dead_map = eina_list_remove_list(file->dead_map, l);
        free_func(em);
     }
}

void
eina_file_flush(Eina_File *file, unsigned long int length)
{
   Eina_File_Map *tmp;
   Eina_Iterator *it;
   Eina_List *dead_map = NULL;
   Eina_List *l;

   eina_lock_take(&file->lock);

   // File size changed
   if (file->global_map != MAP_FAILED)
     {
        // Forget global map
        tmp = malloc(sizeof (Eina_File_Map));
        if (tmp)
          {
             tmp->map = file->global_map;
             tmp->offset = 0;
             tmp->length = file->length;
             tmp->refcount = file->refcount;

             file->dead_map = eina_list_append(file->dead_map, tmp);
          }

        file->global_map = MAP_FAILED;
        file->global_refcount = 0;
     }

   it = eina_hash_iterator_data_new(file->map);
   EINA_ITERATOR_FOREACH(it, tmp)
     {
        // Add out of limit map to dead_map
        if (tmp->offset + tmp->length > length)
          dead_map = eina_list_append(dead_map, tmp);
     }
   eina_iterator_free(it);

   EINA_LIST_FOREACH(dead_map, l, tmp)
     {
        unsigned long int key[2];

        key[0] = tmp->offset;
        key[1] = tmp->length;

        eina_hash_del(file->rmap, &tmp->map, tmp);
        eina_hash_del(file->map, &key, tmp);
     }

   file->dead_map = eina_list_merge(file->dead_map, dead_map);

   eina_lock_release(&file->lock);
}

// Private to this file API
static void
_eina_file_map_close(Eina_File_Map *map)
{
   free(map);
}

// Global API

EAPI char *
eina_file_path_sanitize(const char *path)
{
   Eina_Tmpstr *result = NULL;
   char *r;
   size_t len;

   if (!path) return NULL;

   len = strlen(path);

   if (eina_file_path_relative(path))
     {
       result = eina_file_current_directory_get(path, len);
       len = eina_tmpstr_len(result);
     }
   else
     result = path;

   r = _eina_file_escape(strdup(result ? result : ""), len);
   if (result != path) eina_tmpstr_del(result);

   return r;
}

EAPI Eina_File *
eina_file_virtualize(const char *virtual_name, const void *data, unsigned long long length, Eina_Bool copy)
{
   Eina_File *file;
   Eina_Nano_Time tp;
   long int ti;
   const char *tmpname = "/dev/mem/virtual\\/%16x";
   size_t slen, head_padded;

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

   // Generate an almost uniq filename based on current nsec time.
   if (_eina_time_get(&tp)) return NULL;
   ti = _eina_time_convert(&tp);

   slen = virtual_name ? strlen(virtual_name) + 1 : strlen(tmpname) + 17;
   // align copied data at end of file struct to 16 bytes...
   head_padded = 16 * ((sizeof(Eina_File) + slen + 15) / 16);

   file = malloc(head_padded + (copy ? length : 0));
   if (!file) return NULL;

   memset(file, 0, sizeof(Eina_File));
   EINA_MAGIC_SET(file, EINA_FILE_MAGIC);
   file->filename = (char *)(file + 1);
   if (virtual_name)
     file->filename = eina_stringshare_add(virtual_name);
   else
     file->filename = eina_stringshare_printf(tmpname, ti);

   eina_lock_recursive_new(&file->lock);
   file->mtime = ti / 1000;
   file->length = length;
#ifdef _STAT_VER_LINUX
   file->mtime_nsec = ti;
#endif
   file->refcount = 1;
#ifndef _WIN32
   file->fd = -1;
#else
   file->handle = INVALID_HANDLE_VALUE;
#endif
   file->virtual = EINA_TRUE;
   file->map = eina_hash_new(EINA_KEY_LENGTH(eina_file_map_key_length),
                             EINA_KEY_CMP(eina_file_map_key_cmp),
                             EINA_KEY_HASH(eina_file_map_key_hash),
                             EINA_FREE_CB(_eina_file_map_close),
                             3);
   file->rmap = eina_hash_pointer_new(NULL);

   if (copy)
     {
        file->copied = EINA_TRUE;
        file->global_map = ((char *)file) + head_padded;
        memcpy((char *)file->global_map, data, length);
     }
   else
     {
        file->global_map = (void *)data;
     }

   return file;
}

EAPI Eina_Bool
eina_file_virtual(Eina_File *file)
{
   if (!file) return EINA_FALSE;
   EINA_FILE_MAGIC_CHECK(file, EINA_FALSE);
   return file->virtual;
}

EAPI Eina_File *
eina_file_dup(const Eina_File *f)
{
   Eina_File *file = (Eina_File*) f;

   if (file)
     {
        EINA_FILE_MAGIC_CHECK(f, NULL);
        eina_lock_take(&file->lock);

        // For ease of use and safety of the API, if you dup a virtualized file, we prefer to make a copy
        if (file->virtual && !file->copied)
          {
             Eina_File *r;

             r = eina_file_virtualize(file->filename, file->global_map, file->length, EINA_TRUE);
             eina_lock_release(&file->lock);

             return r;
          }
        file->refcount++;
        eina_lock_release(&file->lock);
     }
   return file;
}

void
eina_file_clean_close(Eina_File *file)
{
   // Generic destruction of the file
   eina_hash_free(file->rmap); file->rmap = NULL;
   eina_hash_free(file->map); file->map = NULL;
   eina_stringshare_del(file->filename);

   // Backend specific file resource close
   eina_file_real_close(file);

   // Final death
   EINA_MAGIC_SET(file, 0);
   free(file);
}

EAPI void
eina_file_close(Eina_File *file)
{
   Eina_Bool leave = EINA_TRUE;
   unsigned int key;

   if (!file) return ;
   EINA_FILE_MAGIC_CHECK(file);

   eina_lock_take(&_eina_file_lock_cache);

   eina_lock_take(&file->lock);
   file->refcount--;
   if (file->refcount == 0) leave = EINA_FALSE;
   eina_lock_release(&file->lock);
   if (leave) goto end;

   key = eina_hash_superfast((void*) &file->filename, sizeof (void*));
   if (eina_hash_find_by_hash(_eina_file_cache,
                              file->filename, 0, key) == file)
     {
        eina_hash_del_by_key_hash(_eina_file_cache,
                                  file->filename, 0, key);
     }

   eina_file_clean_close(file);
 end:
   eina_lock_release(&_eina_file_lock_cache);
}

EAPI size_t
eina_file_size_get(const Eina_File *file)
{
   EINA_FILE_MAGIC_CHECK(file, 0);
   return file->length;
}

EAPI time_t
eina_file_mtime_get(const Eina_File *file)
{
   EINA_FILE_MAGIC_CHECK(file, 0);
   return file->mtime;
}

EAPI const char *
eina_file_filename_get(const Eina_File *file)
{
   EINA_FILE_MAGIC_CHECK(file, NULL);
   return file->filename;
}

Eina_Stringshare *
eina_file_sanitize(const char *path)
{
   char *filename;
   Eina_Stringshare *ss;

   filename = eina_file_path_sanitize(path);
   if (!filename) return NULL;

   ss = eina_stringshare_add(filename);
   free(filename);
   return ss;
}

/* search '\r' and '\n' by preserving cache locality and page locality
   in doing a search inside 4K boundary.
 */
static inline const char *
_eina_find_eol(const char *start, int boundary, const char *end)
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
               return lf;
             return cr;
          }
        else if (lf)
           return lf;

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
   if (it->current.index > 0)
     it->current.end++;
   while (it->current.end < it->end &&
          (*it->current.end == '\n' || *it->current.end == '\r'))
     {
        if (match == *it->current.end)
          break;
        it->current.end++;
     }
   it->current.index++;

   if (it->current.end == it->end)
     return EINA_FALSE;

   eol = _eina_find_eol(it->current.end,
                        it->boundary,
                        it->end);
   it->boundary = (uintptr_t) eol & 0x3FF;
   if (it->boundary == 0) it->boundary = 4096;

   it->current.start = it->current.end;

   it->current.end = eol;
   it->current.length = eol - it->current.start;

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

   EINA_FILE_MAGIC_CHECK(file, NULL);

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

static Eina_Bool
_eina_file_copy_write_internal(int fd, char *buf, size_t size)
{
   size_t done = 0;
   while (done < size)
     {
        ssize_t w = write(fd, buf + done, size - done);
        if (w >= 0)
          done += w;
        else if ((errno != EAGAIN) && (errno != EINTR))
          {
             ERR("Error writing destination file during copy: %s",
                 strerror(errno));
             return EINA_FALSE;
          }
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_file_copy_read_internal(int fd, char *buf, off_t bufsize, ssize_t *readsize)
{
   while (1)
     {
        ssize_t r = read(fd, buf, bufsize);
        if (r == 0)
          {
             ERR("Premature end of source file during copy.");
             return EINA_FALSE;
          }
        else if (r < 0)
          {
             if ((errno != EAGAIN) && (errno != EINTR))
               {
                  ERR("Error reading source file during copy: %s",
                      strerror(errno));
                  return EINA_FALSE;
               }
          }
        else
          {
             *readsize = r;
             return EINA_TRUE;
          }
     }
}

#ifdef HAVE_SPLICE
static Eina_Bool
_eina_file_copy_write_splice_internal(int fd, int pipefd, size_t size)
{
   size_t done = 0;
   while (done < size)
     {
        ssize_t w = splice(pipefd, NULL, fd, NULL, size - done, SPLICE_F_MORE);
        if (w >= 0)
          done += w;
        else if (errno == EINVAL)
          {
             INF("Splicing is not supported for destination file");
             return EINA_FALSE;
          }
        else if ((errno != EAGAIN) && (errno != EINTR))
          {
             ERR("Error splicing to destination file during copy: %s",
                 strerror(errno));
             return EINA_FALSE;
          }
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_file_copy_read_splice_internal(int fd, int pipefd, off_t bufsize, ssize_t *readsize)
{
   while (1)
     {
        ssize_t r = splice(fd, NULL, pipefd, NULL, bufsize, SPLICE_F_MORE);
        if (r == 0)
          {
             ERR("Premature end of source file during splice.");
             return EINA_FALSE;
          }
        else if (r < 0)
          {
             if (errno == EINVAL)
               {
                  INF("Splicing is not supported for source file");
                  return EINA_FALSE;
               }
             else if ((errno != EAGAIN) && (errno != EINTR))
               {
                  ERR("Error splicing from source file during copy: %s",
                      strerror(errno));
                  return EINA_FALSE;
               }
          }
        else
          {
             *readsize = r;
             return EINA_TRUE;
          }
     }
}
#endif

static Eina_Bool
_eina_file_copy_splice_internal(int s, int d, off_t total, Eina_File_Copy_Progress cb, const void *cb_data, Eina_Bool *splice_unsupported)
{
#ifdef HAVE_SPLICE
   off_t bufsize = COPY_BLOCKSIZE;
   off_t done;
   Eina_Bool ret;
   int pipefd[2];

   *splice_unsupported = EINA_TRUE;

   if (pipe(pipefd) < 0) return EINA_FALSE;

   done = 0;
   ret = EINA_TRUE;
   while (done < total)
     {
        size_t todo;
        ssize_t r;

        if (done + bufsize < total)
          todo = bufsize;
        else
          todo = total - done;

        ret = _eina_file_copy_read_splice_internal(s, pipefd[1], todo, &r);
        if (!ret) break;

        ret = _eina_file_copy_write_splice_internal(d, pipefd[0], r);
        if (!ret) break;

        *splice_unsupported = EINA_FALSE;
        done += r;

        if (cb)
          {
             ret = cb((void *)cb_data, done, total);
             if (!ret) break;
          }
     }

   close(pipefd[0]);
   close(pipefd[1]);

   return ret;
#endif
   *splice_unsupported = EINA_TRUE;
   return EINA_FALSE;
   (void)s;
   (void)d;
   (void)total;
   (void)cb;
   (void)cb_data;
}

static Eina_Bool
_eina_file_copy_internal(int s, int d, off_t total, Eina_File_Copy_Progress cb, const void *cb_data)
{
   void *buf = NULL;
   off_t bufsize = COPY_BLOCKSIZE;
   off_t done;
   Eina_Bool ret, splice_unsupported;

   ret = _eina_file_copy_splice_internal(s, d, total, cb, cb_data,
                                         &splice_unsupported);
   if (ret)
     return EINA_TRUE;
   else if (!splice_unsupported) /* splice works, but copy failed anyway */
     return EINA_FALSE;

   /* make sure splice didn't change the position */
   lseek(s, 0, SEEK_SET);
   lseek(d, 0, SEEK_SET);

   while ((bufsize > 0) && ((buf = malloc(bufsize)) == NULL))
     bufsize /= 128;

   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, EINA_FALSE);

   done = 0;
   ret = EINA_TRUE;
   while (done < total)
     {
        size_t todo;
        ssize_t r;

        if (done + bufsize < total)
          todo = bufsize;
        else
          todo = total - done;

        ret = _eina_file_copy_read_internal(s, buf, todo, &r);
        if (!ret) break;

        ret = _eina_file_copy_write_internal(d, buf, r);
        if (!ret) break;

        done += r;

        if (cb)
          {
             ret = cb((void *)cb_data, done, total);
             if (!ret) break;
          }
     }

   free(buf);
   return ret;
}

EAPI Eina_Bool
eina_file_copy(const char *src, const char *dst, Eina_File_Copy_Flags flags, Eina_File_Copy_Progress cb, const void *cb_data)
{
   struct stat st;
   int s, d = -1;
   Eina_Bool success;

   EINA_SAFETY_ON_NULL_RETURN_VAL(src, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dst, EINA_FALSE);

   s = open(src, O_RDONLY | O_BINARY);
   EINA_SAFETY_ON_TRUE_RETURN_VAL (s < 0, EINA_FALSE);

   success = (fstat(s, &st) == 0);
   EINA_SAFETY_ON_FALSE_GOTO(success, end);

   d = open(dst, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
   EINA_SAFETY_ON_TRUE_GOTO(d < 0, end);

   success = _eina_file_copy_internal(s, d, st.st_size, cb, cb_data);
   if (success)
     {
#ifdef HAVE_FCHMOD
        if (flags & EINA_FILE_COPY_PERMISSION)
          fchmod(d, st.st_mode);
#endif
        if (flags & EINA_FILE_COPY_XATTR)
          eina_xattr_fd_copy(s, d);
     }

 end:
   if (d >= 0) close(d);
   else success = EINA_FALSE;
   close(s);

   if (!success)
     unlink(dst);

   return success;
}

EAPI int
eina_file_mkstemp(const char *templatename, Eina_Tmpstr **path)
{
   char buffer[PATH_MAX];
   const char *XXXXXX = NULL, *sep;
   int fd, len;
#ifndef _WIN32
   mode_t old_umask;
#endif

   EINA_SAFETY_ON_NULL_RETURN_VAL(templatename, -1);

   sep = strchr(templatename, '/');
#ifdef _WIN32
   if (!sep) sep = strchr(templatename, '\\');
#endif
   if (sep)
     {
        len = eina_strlcpy(buffer, templatename, sizeof(buffer));
     }
   else
     {
        len = eina_file_path_join(buffer, sizeof(buffer),
                                  eina_environment_tmp_get(), templatename);
     }

   /*
    * Unix:
    * Make sure temp file is created with secure permissions,
    * http://man7.org/linux/man-pages/man3/mkstemp.3.html#NOTES
    *
    * Windows:
    * no secure permissions anyway and the umask use below makes
    * the file read-only.
    */
#ifndef _WIN32
   old_umask = umask(S_IRWXG|S_IRWXO);
#endif
   if ((XXXXXX = strstr(buffer, "XXXXXX.")) != NULL)
     {
        int suffixlen = buffer + len - XXXXXX - 6;
        fd = mkstemps(buffer, suffixlen);
     }
   else
     fd = mkstemp(buffer);
#ifndef _WIN32
   umask(old_umask);
#endif

   if (fd < 0)
     {
        if (path) *path = NULL;
        return -1;
     }

   if (path) *path = eina_tmpstr_add(buffer);
   return fd;
}

EAPI Eina_Bool
eina_file_mkdtemp(const char *templatename, Eina_Tmpstr **path)
{
   char buffer[PATH_MAX];
   char *tmpdirname, *sep;

   EINA_SAFETY_ON_NULL_RETURN_VAL(templatename, EINA_FALSE);

   sep = strchr(templatename, '/');
#ifdef _WIN32
   if (!sep) sep = strchr(templatename, '\\');
#endif
   if (sep)
     {
        eina_strlcpy(buffer, templatename, sizeof(buffer));
     }
   else
     {
        eina_file_path_join(buffer, sizeof(buffer),
                            eina_environment_tmp_get(), templatename);
     }

   tmpdirname = mkdtemp(buffer);
   if (tmpdirname == NULL)
     {
        if (path) *path = NULL;
        return EINA_FALSE;
     }

   if (path) *path = eina_tmpstr_add(tmpdirname);
   return EINA_TRUE;
}

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

Eina_Bool
eina_file_init(void)
{
   _eina_file_log_dom = eina_log_domain_register("eina_file",
                                                 EINA_LOG_COLOR_DEFAULT);
   if (_eina_file_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_file");
        return EINA_FALSE;
     }

   _eina_file_cache = eina_hash_stringshared_new(NULL);
   if (!_eina_file_cache)
     {
        ERR("Could not create cache.");
        eina_log_domain_unregister(_eina_file_log_dom);
        _eina_file_log_dom = -1;
        return EINA_FALSE;
     }

   eina_spinlock_new(&_eina_statgen_lock);
   eina_spinlock_take(&_eina_statgen_lock);
   if (getenv("EINA_STATGEN")) _eina_statgen = 1;
   eina_spinlock_release(&_eina_statgen_lock);
   eina_lock_recursive_new(&_eina_file_lock_cache);
   eina_magic_string_set(EINA_FILE_MAGIC, "Eina_File");

   return EINA_TRUE;
}

Eina_Bool
eina_file_shutdown(void)
{
   if (eina_hash_population(_eina_file_cache) > 0)
     {
        Eina_Iterator *it;
        const char *key;

        it = eina_hash_iterator_key_new(_eina_file_cache);
        EINA_ITERATOR_FOREACH(it, key)
          {
             Eina_File *f = eina_hash_find(_eina_file_cache, key);
             ERR("File [%s] still open %i times !", key, f->refcount);
          }
        eina_iterator_free(it);
     }

   eina_hash_free(_eina_file_cache);
   _eina_file_cache = NULL;

   eina_lock_free(&_eina_file_lock_cache);

   eina_log_domain_unregister(_eina_file_log_dom);
   _eina_file_log_dom = -1;
   eina_spinlock_free(&_eina_statgen_lock);
   return EINA_TRUE;
}

EAPI Eina_Bool
eina_file_close_on_exec(int fd, Eina_Bool on)
{
#ifdef HAVE_FCNTL
   int flags;

   flags = fcntl(fd, F_GETFD);
   if (flags < 0)
     {
        int errno_backup = errno;
        ERR("%#x = fcntl(%d, F_GETFD): %s", flags, fd, strerror(errno));
        errno = errno_backup;
        return EINA_FALSE;
     }

   if (on)
     flags |= FD_CLOEXEC;
   else
     flags &= (~flags);

   if (fcntl(fd, F_SETFD, flags) == -1)
     {
        int errno_backup = errno;
        ERR("fcntl(%d, F_SETFD, %#x): %s", fd, flags, strerror(errno));
        errno = errno_backup;
        return EINA_FALSE;
     }
   return EINA_TRUE;
#else
   static Eina_Bool statement = EINA_FALSE;

   if (!statement)
     ERR("fcntl is not available on your platform. fd may leak when using exec.");
   statement = EINA_TRUE;
   return EINA_TRUE;
#endif
}
