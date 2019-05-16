/* EINA - EFL data type library
 * Copyright (C) 2015 Carsten Haitzler
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

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef _WIN32
#include <evil_private.h> /* realpath */
#endif

#include "eina_debug_private.h"

extern Eina_Spinlock _eina_debug_lock;

static unsigned int _table_num = 0;
static unsigned int _table_size = 0;
static const char **_table = NULL;

// a very simple "fast lookup" of a filename to a path. we expect this table
// of lookups to remain very small as it most likely includes just the
// application executable itself as this was run from $PATH or using
// a relative path relative to cwd of shell at time of exec. this is really
// the only likely content, but just in case, handle more. it is much faster
// than going through systemcalls like realpath() every time (well this libc
// function will at least be a system call or use system calls to do its work)
static const char *
_eina_debug_file_lookup(const char *fname)
{
   unsigned int n;

   if (!_table) return NULL;
   for (n = 0; _table[n]; n += 2)
     {
        if (!strcmp(_table[n], fname)) return _table[n + 1];
     }
   return NULL;
}

// record a new filename -> path entry in our table. the table really is just
// odd/even strings like fname, path, fname2, path2, fnamr3, path3, ...
// and we are unlikely to have much more than 1 entry here. see above
static const char *
_eina_debug_file_store(const char *fname, const char *file)
{
   static const char **table2;

   _table_num += 2;
   if (_table_num >= _table_size)
     {
        _table_size += 32;
        table2 = _eina_debug_chunk_realloc(_table_size * sizeof(const char *));
        if (!table2) return NULL;
        _table = table2;
     }
   _table[_table_num - 2] = _eina_debug_chunk_strdup(fname);
   _table[_table_num - 1] = _eina_debug_chunk_strdup(file);
   return _table[_table_num - 1];
}

// do a "fast lookup" of a filename to a file path for debug output. this
// relies on caching to avoid system calls and assumes that once we know
// the full path of a given filename, we can know its full path reliably.
// if we can't we'd be in trouble anyway as the filename and path lookup
// failure is due maybe to a deleted file or renamed file and then we are
// going to have a bad day either way.
const char *
_eina_debug_file_get(const char *fname)
{
   char buf[4096];
   const char *file;
   static const char **path = NULL;
   static char *pathstrs = NULL;

   // no filename provided
   if ((!fname) || (!fname[0])) return NULL;
   // it's a full path so return as-is
   if (fname[0] == '/') return fname;
   // first look in cache for filename -> full path lookup and if
   // there, return that (yes - assuming filesystem paths doesn't change
   // which is unlikely as they were set up at star most likely)
   eina_spinlock_take(&_eina_debug_lock);
   file = _eina_debug_file_lookup(fname);
   eina_spinlock_release(&_eina_debug_lock);
   if (file) return file;

   // store PATH permanently - yes. if it changes runtime it will break.
   // for speed reasons we need to assume it won't change. store path broken
   // down into an array of ptrs to strings with NULL ptr at the end. this
   // will only execute once as an "init" for a breoken up path so it should
   // not matter speed-wise
   eina_spinlock_take(&_eina_debug_lock);
   if (!path)
     {
        unsigned int n;
        char *p1, *p2;
        const char *p;
        const char *pathstr = getenv("PATH");

        if (!pathstr)
          {
             eina_spinlock_release(&_eina_debug_lock);
             return NULL;
          }
        // dup the entire env as we will rpelace : with 0 bytes to break str
        pathstrs = _eina_debug_chunk_strdup(pathstr);
        for (n = 0, p = pathstr; *p;)
          {
             n++;
             p = strchr(p, ':');
             if (!p) break;
             p++;
          }
        path = _eina_debug_chunk_push(sizeof(const char *) * (n + 1));
        for (n = 0, p1 = pathstrs; *p1; n++)
          {
             path[n] = p1;
             p2 = strchr(p1, ':');
             if (!p2) break;
             *p2 = 0;
             p1 = p2 + 1;
          }
        path[n] = NULL;
     }
   eina_spinlock_release(&_eina_debug_lock);

   // a relative path - resolve with realpath. due to the cache store above
   // we shouldn't have to do this very often
   if ((!strncmp(fname, "./", 2)) || (!strncmp(fname, "../", 3)))
     { // relative path
        if (realpath(fname, buf)) file = buf;
        else file = NULL;
     }
   // search in $PATH for the file then - this should also be very rare as
   // we will store and cache results permanently
   else if (path)
     {
        struct stat st;
        unsigned int n;

        for (n = 0; path[n]; n++)
          {
             snprintf(buf, sizeof(buf), "%s/%s", path[n], fname);
             if (stat(buf, &st) == 0)
               {
                  file = buf;
                  break;
               }
          }
     }
   // if it's found - store it in cache for later
   if (file)
     {
        eina_spinlock_take(&_eina_debug_lock);
        file = _eina_debug_file_store(fname, file);
        eina_spinlock_release(&_eina_debug_lock);
     }
   return file;
}
