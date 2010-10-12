/* EIO - EFL data type library
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
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

#define _FILE_OFFSET_BITS 64

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#ifndef _MSC_VER
# include <unistd.h>
# include <libgen.h>
#endif

#ifdef HAVE_FEATURES_H
# include <features.h>
#endif
#include <ctype.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "eio_private.h"

#include "Eio.h"

static int
eio_strcmp(const void *a, const void *b)
{
   return strcmp(a, b);
}

static Eina_Bool
_eio_dir_recursiv_ls(Ecore_Thread *thread, Eio_Dir_Copy *copy, const char *target)
{
   const Eina_File_Direct_Info *info;
   Eina_Iterator *it;
   Eina_List *dirs = NULL;
   Eina_List *l;
   const char *dir;
   struct stat buffer;

   it = eina_file_direct_ls(target);
   if (!it)
     {
        eio_file_thread_error(&copy->progress.common);
        return EINA_FALSE;
     }

   EINA_ITERATOR_FOREACH(it, info)
     {
        fprintf(stderr, "entry: %s\n", info->path);
        switch (info->dirent->d_type)
          {
           case DT_UNKNOWN:
              if (stat(info->path, &buffer) != 0)
                {
                   eio_file_thread_error(&copy->progress.common);
                   goto on_error;
                }

              if (S_ISDIR(buffer.st_mode))
                dirs = eina_list_append(dirs, eina_stringshare_add(info->path));
              else
                copy->files = eina_list_append(copy->files, eina_stringshare_add(info->path));
              break;
           case DT_DIR:
              dirs = eina_list_append(dirs, eina_stringshare_add(info->path));
              break;
           default:
              copy->files = eina_list_append(copy->files, eina_stringshare_add(info->path));
              break;
          }

        if (ecore_thread_check(thread))
          goto on_error;
     }

   eina_iterator_free(it);

   EINA_LIST_FOREACH(dirs, l, dir)
     if (!_eio_dir_recursiv_ls(thread, copy, dir))
       {
          EINA_LIST_FREE(dirs, dir)
            eina_stringshare_del(dir);
          return EINA_FALSE;
       }

   copy->dirs = eina_list_merge(copy->dirs, dirs);
   return EINA_TRUE;

 on_error:
   eina_iterator_free(it);

   EINA_LIST_FREE(dirs, dir)
     eina_stringshare_del(dir);

   return EINA_FALSE;
}

static void
_eio_dir_copy_heavy(Ecore_Thread *thread, void *data)
{
   Eio_Dir_Copy *copy = data;
   const char *file = NULL;
   const char *dir = NULL;

   Eio_File_Progress file_copy;
   char target[PATH_MAX];
   struct stat buffer;

   int length_source;
   int length_dest;
   off_t count;
   off_t step;

   /* list all the content that should be copied */
   if (!_eio_dir_recursiv_ls(thread, copy, copy->progress.source))
     return ;

   /* notify main thread of the amount of work todo */
   step = 0;
   count = eina_list_count(copy->files) + eina_list_count(copy->dirs);
   eio_progress_send(thread, &copy->progress, step, count);

   /* sort the content, so we create the directory in the right order */
   copy->dirs = eina_list_sort(copy->dirs, -1, eio_strcmp);
   copy->files = eina_list_sort(copy->files, -1, eio_strcmp);

   /* prepare stuff */
   length_source = eina_stringshare_strlen(copy->progress.source);
   length_dest = eina_stringshare_strlen(copy->progress.dest);

   memcpy(&file_copy, &copy->progress, sizeof (Eio_File_Progress));
   file_copy.op = EIO_FILE_COPY;
   file_copy.source = NULL;
   file_copy.dest = NULL;

   /* create destination dir if not available */
   if (stat(copy->progress.dest, &buffer) != 0)
     {
        if (stat(copy->progress.source, &buffer) != 0)
          goto on_error;

        if (mkdir(copy->progress.dest, buffer.st_mode) != 0)
          goto on_error;
     }

   /* create all directory */
   EINA_LIST_FREE(copy->dirs, dir)
     {
        /* build target dir path */
        strcpy(target, copy->progress.dest);
        target[length_dest] = '/';
        strcpy(target + length_dest + 1, dir + length_source);

        /* stat the original file for mode info */
        /* FIXME: in some case we already did a stat call, so would be nice to reuse previous result here */
        /* FIXME: apply mode later so that readonly could be copied and property will be set correctly */
        if (stat(dir, &buffer) != 0)
          goto on_error;

        /* create the directory */
        if (mkdir(target, buffer.st_mode) != 0)
          goto on_error;

        step++;
        eio_progress_send(thread, &copy->progress, step, count);

        if (ecore_thread_check(thread))
          goto on_error;

        eina_stringshare_del(dir);
     }
   dir = NULL;

   /* Copy all files */
   EINA_LIST_FREE(copy->files, file)
     {
        /* build target dir path */
        strcpy(target, copy->progress.dest);
        target[length_dest] = '/';
        strcpy(target + length_dest + 1, file + length_source);

        file_copy.source = file;
        file_copy.dest = eina_stringshare_add(target);

        if (!eio_file_copy_do(thread, &file_copy))
          {
             copy->progress.common.error = file_copy.common.error;
             goto on_error;
          }

        step++;
        eio_progress_send(thread, &copy->progress, step, count);

        if (ecore_thread_check(thread))
          goto on_error;

        eina_stringshare_del(file_copy.dest);
        eina_stringshare_del(file);
     }
   file_copy.dest = NULL;
   file = NULL;

 on_error:
   /* cleanup the mess */
   if (file_copy.dest) eina_stringshare_del(file_copy.dest);
   if (dir) eina_stringshare_del(dir);
   if (file) eina_stringshare_del(file);

   EINA_LIST_FREE(copy->files, file)
     eina_stringshare_del(file);
   EINA_LIST_FREE(copy->dirs, dir)
     eina_stringshare_del(dir);

   if (!ecore_thread_check(thread))
     eio_progress_send(thread, &copy->progress, count, count);

   return ;
}

static void
_eio_dir_copy_notify(Ecore_Thread *thread __UNUSED__, void *msg_data, void *data)
{
   Eio_Dir_Copy *copy = data;
   Eio_Progress *progress = msg_data;

   eio_progress_cb(progress, &copy->progress);
}

static void
_eio_dir_copy_free(Eio_Dir_Copy *copy)
{
   eina_stringshare_del(copy->progress.source);
   eina_stringshare_del(copy->progress.dest);
   free(copy);
}

static void
_eio_dir_copy_end(void *data)
{
   Eio_Dir_Copy *copy = data;

   copy->progress.common.done_cb((void*) copy->progress.common.data);

   _eio_dir_copy_free(copy);
}

static void
_eio_dir_copy_error(void *data)
{
   Eio_Dir_Copy *copy = data;

   eio_file_error(&copy->progress.common);

   _eio_dir_copy_free(copy);
}


/**
 * @addtogroup Eio_Group Asynchronous Inout/Output library
 *
 * @{
 */

/**
 * @brief Copy a directory and it's content asynchronously
 * @param source Should be the name of the directory to copy the data from.
 * @param dest Should be the name of the directory to copy the data to.
 * @param progress_cb Callback called to know the progress of the copy.
 * @param done_cb Callback called when the copy is done.
 * @param error_cb Callback called when something goes wrong.
 * @param data Private data given to callback.
 *
 * This function will copy a directory and all it's content from source to dest.
 * It will try to use splice if possible, if not it will fallback to mmap/write.
 * It will try to preserve access right, but not user/group identification.
 */
EAPI Eio_File *
eio_dir_copy(const char *source,
             const char *dest,
             Eio_Progress_Cb progress_cb,
             Eio_Done_Cb done_cb,
             Eio_Error_Cb error_cb,
             const void *data)
{
   Eio_Dir_Copy *copy;

   EINA_SAFETY_ON_NULL_RETURN_VAL(source, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dest, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   copy = malloc(sizeof(Eio_Dir_Copy));
   EINA_SAFETY_ON_NULL_RETURN_VAL(copy, NULL);

   copy->progress.op = EIO_DIR_COPY;
   copy->progress.progress_cb = progress_cb;
   copy->progress.source = eina_stringshare_add(source);
   copy->progress.dest = eina_stringshare_add(dest);
   copy->files = NULL;

   if (!eio_long_file_set(&copy->progress.common,
                          done_cb,
                          error_cb,
                          data,
                          _eio_dir_copy_heavy,
                          _eio_dir_copy_notify,
                          _eio_dir_copy_end,
                          _eio_dir_copy_error))
     return NULL;

   return &copy->progress.common;
}

/**
 * @}
 */
