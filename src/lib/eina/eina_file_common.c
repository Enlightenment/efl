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
#ifdef HAVE_DIRENT_H
# include <dirent.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef HAVE_EVIL
# include <Evil.h>
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

#ifdef MAP_FAILED
# undef MAP_FAILED
#endif
#define MAP_FAILED ((void *)-1)

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


unsigned int
eina_file_map_key_length(const void *key EINA_UNUSED)
{
   return sizeof (unsigned long int) * 2;
}

int
eina_file_map_key_cmp(const unsigned long long int *key1, int key1_length EINA_UNUSED,
                       const unsigned long long int *key2, int key2_length EINA_UNUSED)
{
   if (key1[0] - key2[0] == 0) return key1[1] - key2[1];
   return key1[0] - key2[0];
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

EAPI Eina_File *
eina_file_virtualize(const char *virtual_name, const void *data, unsigned long long length, Eina_Bool copy)
{
   Eina_File *file;
   Eina_Nano_Time tp;
   long int ti;
   const char *tmpname = "/dev/mem/virtual\\/%16x";
   int slen;

   // Generate an almost uniq filename based on current nsec time.
   if (_eina_time_get(&tp)) return NULL;
   ti = _eina_time_convert(&tp);

   slen = virtual_name ? strlen(virtual_name) + 1 : strlen(tmpname) + 17;

   file = malloc(sizeof (Eina_File) +
                 slen +
                 (copy ? length : 0));
   if (!file) return NULL;

   memset(file, 0, sizeof(Eina_File));
   file->filename = (char*) (file + 1);
   if (virtual_name)
     strcpy((char*) file->filename, virtual_name);
   else
     sprintf((char*) file->filename, tmpname, ti);

   eina_lock_new(&file->lock);
   file->mtime = ti / 1000;
   file->length = length;
#ifdef _STAT_VER_LINUX
   file->mtime_nsec = ti;
#endif
   file->refcount = 1;
#ifndef _WIN32
   file->fd = -1;
#else
   file->handle = NULL;
   file->fm = NULL;
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
        file->global_map = (void*)(file->filename +
                                   strlen(file->filename) + 1);
        memcpy((char*) file->global_map, data, length);
     }
   else
     {
        file->global_map = (void*) data;
     }

   return file;
}

EAPI Eina_Bool
eina_file_virtual(Eina_File *file)
{
   if (file) return file->virtual;
   return EINA_FALSE;
}

EAPI Eina_File *
eina_file_dup(const Eina_File *f)
{
   Eina_File *file = (Eina_File*) f;

   if (file)
     {
        eina_lock_take(&file->lock);
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
   unsigned int length;
   unsigned int key;

   EINA_SAFETY_ON_NULL_RETURN(file);

   eina_lock_take(&_eina_file_lock_cache);

   eina_lock_take(&file->lock);
   file->refcount--;
   if (file->refcount == 0) leave = EINA_FALSE;
   eina_lock_release(&file->lock);
   if (leave) goto end;

   length = strlen(file->filename) + 1;
   key = eina_hash_djb2(file->filename, length);
   if (eina_hash_find_by_hash(_eina_file_cache,
                              file->filename, length, key) == file)
     {
        eina_hash_del_by_key_hash(_eina_file_cache,
                                  file->filename, length, key);
     }

   eina_file_clean_close(file);
 end:
   eina_lock_release(&_eina_file_lock_cache);
}

EAPI size_t
eina_file_size_get(const Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, 0);
   return file->length;
}

EAPI time_t
eina_file_mtime_get(const Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, 0);
   return file->mtime;
}

EAPI const char *
eina_file_filename_get(const Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);
   return file->filename;
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

   eol = _eina_find_eol(it->current.end,
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

   s = open(src, O_RDONLY);
   EINA_SAFETY_ON_TRUE_RETURN_VAL (s < 0, EINA_FALSE);

   success = (fstat(s, &st) == 0);
   EINA_SAFETY_ON_FALSE_GOTO(success, end);

   d = open(dst, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
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
   const char *tmpdir = NULL;
   const char *XXXXXX = NULL;
   int fd, len;
   mode_t old_umask;

#ifndef HAVE_EVIL
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     tmpdir = getenv("TMPDIR");
   if (!tmpdir) tmpdir = "/tmp";
#else
   tmpdir = (char *)evil_tmpdir_get();
#endif /* ! HAVE_EVIL */

   len = snprintf(buffer, PATH_MAX, "%s/%s", tmpdir, templatename);

   /* 
    * Make sure temp file is created with secure permissions,
    * http://man7.org/linux/man-pages/man3/mkstemp.3.html#NOTES
    */
   old_umask = umask(S_IRWXG|S_IRWXO);
   if ((XXXXXX = strstr(buffer, "XXXXXX.")) != NULL)
     {
        int suffixlen = buffer + len - XXXXXX - 6;
        fd = mkstemps(buffer, suffixlen);
     }
   else
     fd = mkstemp(buffer);
   umask(old_umask);

   if (path) *path = eina_tmpstr_add(buffer);
   if (fd < 0)
     return -1;

   return fd;
}

EAPI Eina_Bool
eina_file_mkdtemp(const char *templatename, Eina_Tmpstr **path)
{
   char buffer[PATH_MAX];
   const char *tmpdir = NULL;
   char *tmpdirname;

#ifndef HAVE_EVIL
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     tmpdir = getenv("TMPDIR");
   if (!tmpdir) tmpdir = "/tmp";
#else
   tmpdir = (char *)evil_tmpdir_get();
#endif /* ! HAVE_EVIL */

   snprintf(buffer, PATH_MAX, "%s/%s", tmpdir, templatename);

   tmpdirname = mkdtemp(buffer);
   if (path) *path = eina_tmpstr_add(buffer);
   if (tmpdirname == NULL)
     return EINA_FALSE;

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

   _eina_file_cache = eina_hash_string_djb2_new(NULL);
   if (!_eina_file_cache)
     {
        ERR("Could not create cache.");
        eina_log_domain_unregister(_eina_file_log_dom);
        _eina_file_log_dom = -1;
        return EINA_FALSE;
     }

   eina_lock_new(&_eina_file_lock_cache);

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

   eina_lock_free(&_eina_file_lock_cache);

   eina_log_domain_unregister(_eina_file_log_dom);
   _eina_file_log_dom = -1;
   return EINA_TRUE;
}
