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

#ifndef EINA_FILE_COMMON_H_
# define EINA_FILE_COMMON_H_

#include "eina_file.h"
#include "eina_tmpstr.h"
#include "eina_lock.h"
#include "eina_list.h"

#define EINA_FILE_MAGIC 0xFEEDBEEF

typedef struct _Eina_File_Map Eina_File_Map;
typedef struct _Eina_Lines_Iterator Eina_Lines_Iterator;

struct _Eina_File
{
   EINA_MAGIC;
   const char *filename;

   Eina_Hash *map;
   Eina_Hash *rmap;
   void *global_map;

   Eina_Lock lock;

#ifndef _WIN32
   unsigned long long length;
   time_t mtime;
   ino_t inode;
#ifdef _STAT_VER_LINUX
   unsigned long int mtime_nsec;
#endif
#else
   ULONGLONG length;
   ULONGLONG mtime;
#endif

   int refcount;
   int global_refcount;

#ifndef _WIN32
   int fd;
#else
   HANDLE handle;
   HANDLE fm;
#endif

   Eina_List *dead_map;

   Eina_Bool shared : 1;
   Eina_Bool delete_me : 1;
   Eina_Bool global_faulty : 1;
   Eina_Bool virtual : 1;
};

struct _Eina_File_Map
{
   void *map;

   unsigned long int offset;
   unsigned long int length;

   int refcount;

   Eina_Bool hugetlb : 1;
   Eina_Bool faulty : 1;
};

struct _Eina_Lines_Iterator
{
   Eina_Iterator iterator;

   Eina_File *fp;
   const char *map;
   const char *end;

   int boundary;

   Eina_File_Line current;
};

#ifndef EINA_LOG_COLOR_DEFAULT
#define EINA_LOG_COLOR_DEFAULT EINA_COLOR_CYAN
#endif

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_file_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eina_file_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eina_file_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_file_log_dom, __VA_ARGS__)


Eina_Bool eina_file_path_relative(const char *path);
Eina_Tmpstr *eina_file_current_directory_get(const char *path, size_t len);
char *eina_file_cleanup(Eina_Tmpstr *path);
void eina_file_clean_close(Eina_File *file);
void eina_file_real_close(Eina_File *file);
void eina_file_flush(Eina_File *file, unsigned long int length);
void eina_file_common_map_free(Eina_File *file, void *map,
                               void (*free_func)(Eina_File_Map *map));


extern Eina_Hash *_eina_file_cache;
extern Eina_Lock _eina_file_lock_cache;
extern int _eina_file_log_dom;

// Common function to handle virtual file
void *eina_file_virtual_map_all(Eina_File *file);
void *eina_file_virtual_map_new(Eina_File *file,
                                unsigned long int offset, unsigned long int length);
void eina_file_virtual_map_free(Eina_File *file, void *map);

// Common hash function
unsigned int eina_file_map_key_length(const void *key);
int eina_file_map_key_cmp(const unsigned long long int *key1, int key1_length,
                          const unsigned long long int *key2, int key2_length);
int eina_file_map_key_hash(const unsigned long long int *key, int key_length);

#endif /* EINA_FILE_COMMON_H_ */
