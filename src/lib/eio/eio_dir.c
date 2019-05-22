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

#ifdef _WIN32
# include <evil_private.h> /* mkdir */
#endif

#include "eio_private.h"
#include "Eio.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static int
eio_strcmp(const void *a, const void *b)
{
   return strcmp(a, b);
}
static Eina_Bool
_eio_dir_recursive_progress(Eio_Dir_Copy *copy, Eio_File *handler, const Eina_File_Direct_Info *info)
{
   if (copy->filter_cb && !copy->filter_cb(&copy->progress.common.data, handler, info))
     return EINA_FALSE;

   switch (info->type)
     {
      case EINA_FILE_UNKNOWN:
         eio_file_thread_error(&copy->progress.common, handler->thread);
         return EINA_FALSE;
      case EINA_FILE_LNK:
         copy->links = eina_list_append(copy->links, eina_stringshare_add(info->path));
         break;
      case EINA_FILE_DIR:
         copy->dirs = eina_list_append(copy->dirs, eina_stringshare_add(info->path));
         break;
      default:
         copy->files = eina_list_append(copy->files, eina_stringshare_add(info->path));
         break;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eio_file_recursiv_ls(Ecore_Thread *thread,
                      Eio_File *common,
                      Eio_Filter_Direct_Cb filter_cb,
		      Eina_Iterator *(*Eina_File_Ls)(const char *target),
                      void *data,
                      const char *target)
{
   Eina_File_Direct_Info *info;
   Eina_Iterator *it = NULL;
   Eina_List *dirs = NULL;
   const char *dir;

   it = Eina_File_Ls(target);
   if (!it)
     {
        eio_file_thread_error(common, thread);
        return EINA_FALSE;
     }

   eio_file_container_set(common, eina_iterator_container_get(it));

   EINA_ITERATOR_FOREACH(it, info)
     {
        Eina_Bool filter = EINA_TRUE;
        _eio_stat_t buffer;

        switch (info->type)
          {
           case EINA_FILE_DIR:
              if (_eio_lstat(info->path, &buffer) != 0)
		continue;

#ifndef _WIN32
              if (S_ISLNK(buffer.st_mode))
                info->type = EINA_FILE_LNK;
#endif
           default:
              break;
          }

        filter = filter_cb(data, common, info);
        if (filter && info->type == EINA_FILE_DIR)
          dirs = eina_list_append(dirs, eina_stringshare_add(info->path));

        if (ecore_thread_check(thread))
          goto on_error;
     }

   eio_file_container_set(common, NULL);

   eina_iterator_free(it);
   it = NULL;

   EINA_LIST_FREE(dirs, dir)
     {
       Eina_Bool err;

       err = !_eio_file_recursiv_ls(thread, common, filter_cb, Eina_File_Ls, data, dir);

       eina_stringshare_del(dir);
       if (err) goto on_error;
     }

   return EINA_TRUE;

 on_error:
   if (it) eina_iterator_free(it);

   EINA_LIST_FREE(dirs, dir)
     eina_stringshare_del(dir);

   return EINA_FALSE;
}


static Eina_Bool
_eio_dir_recursiv_ls(Ecore_Thread *thread, Eio_Dir_Copy *copy, const char *target)
{
   if (!_eio_file_recursiv_ls(thread, &copy->progress.common,
                              (Eio_Filter_Direct_Cb) _eio_dir_recursive_progress,
			      eina_file_stat_ls,
                              copy, target))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_eio_dir_init(Ecore_Thread *thread,
              long long *step, long long *count,
              int *length_source, int *length_dest,
              Eio_Dir_Copy *order,
              Eio_File_Progress *progress)
{
   struct stat buffer;

   /* notify main thread of the amount of work todo */
   *step = 0;
   *count = eina_list_count(order->files)
     + eina_list_count(order->dirs) * 2
     + eina_list_count(order->links);
   eio_progress_send(thread, &order->progress, *step, *count);

   /* sort the content, so we create the directory in the right order */
   order->dirs = eina_list_sort(order->dirs, -1, eio_strcmp);
   order->files = eina_list_sort(order->files, -1, eio_strcmp);
   order->links = eina_list_sort(order->links, -1, eio_strcmp);

   /* prepare stuff */
   *length_source = eina_stringshare_strlen(order->progress.source);
   *length_dest = eina_stringshare_strlen(order->progress.dest);

   memcpy(progress, &order->progress, sizeof (Eio_File_Progress));
   progress->source = NULL;
   progress->dest = NULL;

   /* create destination dir if not available */
   if (stat(order->progress.dest, &buffer) != 0)
     {
        if (stat(order->progress.source, &buffer) != 0)
          {
             eio_file_thread_error(&order->progress.common, thread);
             return EINA_FALSE;
          }

        if (mkdir(order->progress.dest, buffer.st_mode) != 0)
          {
             eio_file_thread_error(&order->progress.common, thread);
             return EINA_FALSE;
          }
     }

   return EINA_TRUE;
}

static void
_eio_dir_target(Eio_Dir_Copy *order, Eina_Strbuf *target, const char *dir, int length_source, int length_dest)
{
   int length;

   length = eina_stringshare_strlen(dir);

   eina_strbuf_append_length(target, order->progress.dest, length_dest);
   eina_strbuf_append(target, "/");
   eina_strbuf_append_length(target, dir + length_source, length - length_source + 1);
}

static Eina_Bool
_eio_dir_mkdir(Ecore_Thread *thread, Eio_Dir_Copy *order,
               long long *step, long long count,
               int length_source, int length_dest)
{
   const char *dir;
   Eina_List *l;
   Eina_Strbuf *target = eina_strbuf_new();

   /* create all directory */
   EINA_LIST_FOREACH(order->dirs, l, dir)
     {
        eina_strbuf_reset(target);
        /* build target dir path */
        _eio_dir_target(order, target, dir, length_source, length_dest);

        /* create the directory (we will apply the mode later) */
        if (mkdir(eina_strbuf_string_get(target), 0777) != 0)
          {
             eio_file_thread_error(&order->progress.common, thread);
             eina_strbuf_free(target);
             return EINA_FALSE;
          }

        /* inform main thread */
        (*step)++;
        eio_progress_send(thread, &order->progress, *step, count);

        /* check for cancel request */
        if (ecore_thread_check(thread))
          {
             eina_strbuf_free(target);
             return EINA_FALSE;
          }
     }

   eina_strbuf_free(target);
   return EINA_TRUE;
}

/* no symbolic link on Windows */
#ifndef _WIN32
static Eina_Bool
_eio_dir_link(Ecore_Thread *thread, Eio_Dir_Copy *order,
              long long *step, long long count,
              int length_source, int length_dest)
{
   const char *ln;
   Eina_List *l;
   Eina_Strbuf *oldpath, *buffer;
   char *target = NULL, *newpath = NULL;
   ssize_t bsz = -1;
   struct stat st;

   oldpath = eina_strbuf_new();
   buffer = eina_strbuf_new();

   /* Build once the base of the link target */
   eina_strbuf_append_length(buffer, order->progress.dest, length_dest);
   eina_strbuf_append(buffer, "/");

   /* recreate all links */
   EINA_LIST_FOREACH(order->links, l, ln)
     {
        ssize_t length;

        eina_strbuf_reset(oldpath);

        /* build oldpath link */
        _eio_dir_target(order, oldpath, ln, length_source, length_dest);

        if (lstat(ln, &st) == -1)
          {
             goto on_error;
          }
        if (st.st_size == 0)
          {
             bsz = PATH_MAX;
             free(target);
             target = malloc(bsz);
          }
        else if(bsz < st.st_size + 1)
          {
             bsz = st.st_size +1;
             free(target);
             target = malloc(bsz);
          }

        /* read link target */
        length = readlink(ln, target, bsz);
        if (length < 0)
          goto on_error;

        if (strncmp(target, order->progress.source, length_source) == 0)
          {
             /* The link is inside the zone to copy, so rename it */
             eina_strbuf_insert_length(buffer, target + length_source,length - length_source + 1, length_dest + 1);
             newpath = target;
          }
        else
          {
             /* The link is outside the zone to copy */
             newpath = target;
          }

        /* create the link */
        if (symlink(newpath, eina_strbuf_string_get(oldpath)) != 0)
          goto on_error;

        /* inform main thread */
        (*step)++;
        eio_progress_send(thread, &order->progress, *step, count);

        /* check for cancel request */
        if (ecore_thread_check(thread))
          goto on_thread_error;
     }

   eina_strbuf_free(oldpath);
   eina_strbuf_free(buffer);
   if(target) free(target);

   return EINA_TRUE;

 on_error:
   eio_file_thread_error(&order->progress.common, thread);
 on_thread_error:
   eina_strbuf_free(oldpath);
   eina_strbuf_free(buffer);
   if(target) free(target);
   return EINA_FALSE;
}
#endif

static Eina_Bool
_eio_dir_chmod(Ecore_Thread *thread, Eio_Dir_Copy *order,
               long long *step, long long count,
               int length_source, int length_dest,
               Eina_Bool rmdir_source)
{
   const char *dir = NULL;
   Eina_Strbuf *target;
   struct stat buffer;

   target = eina_strbuf_new();

   while (order->dirs)
     {
        eina_strbuf_reset(target);

        /* destroy in reverse order so that we don't prevent change of lower dir */
        dir = eina_list_data_get(eina_list_last(order->dirs));
        order->dirs = eina_list_remove_list(order->dirs, eina_list_last(order->dirs));

        /* build target dir path */
        _eio_dir_target(order, target, dir, length_source, length_dest);

        /* FIXME: in some case we already did a stat call, so would be nice to reuse previous result here */
        /* stat the original dir for mode info */
        if (stat(dir, &buffer) != 0)
          goto on_error;

        /* set the orginal mode to the newly created dir */
        if (chmod(eina_strbuf_string_get(target), buffer.st_mode) != 0)
          goto on_error;

        /* if required destroy original directory */
        if (rmdir_source)
          {
             if (rmdir(dir) != 0)
               goto on_error;
          }

        /* inform main thread */
        (*step)++;
        eio_progress_send(thread, &order->progress, *step, count);

        /* check for cancel request */
        if (ecore_thread_check(thread))
          goto on_cancel;

        eina_stringshare_del(dir);
        dir = NULL;
     }

   eina_strbuf_free(target);
   return EINA_TRUE;

 on_error:
   eio_file_thread_error(&order->progress.common, thread);
 on_cancel:
   if (dir) eina_stringshare_del(dir);
   eina_strbuf_free(target);
   return EINA_FALSE;
}

static void
_eio_dir_copy_heavy(void *data, Ecore_Thread *thread)
{
   Eio_Dir_Copy *copy = data;
   const char *file = NULL;
   const char *dir;
   const char *ln;

   Eio_File_Progress file_copy;
   Eina_Strbuf *target;

   int length_source = 0;
   int length_dest = 0;
   long long count;
   long long step;

   /* list all the content that should be copied */
   if (!_eio_dir_recursiv_ls(thread, copy, copy->progress.source))
     return;

   target = eina_strbuf_new();

   /* init all structure needed to copy the file */
   if (!_eio_dir_init(thread, &step, &count, &length_source, &length_dest, copy, &file_copy))
     goto on_error;

   /* suboperation is a file copy */
   file_copy.op = EIO_FILE_COPY;

   /* create all directory */
   if (!_eio_dir_mkdir(thread, copy, &step, count, length_source, length_dest))
     goto on_error;

   /* copy all files */
   EINA_LIST_FREE(copy->files, file)
     {
        eina_strbuf_reset(target);

        /* build target file path */
        _eio_dir_target(copy, target, file, length_source, length_dest);

        file_copy.source = file;
        file_copy.dest = eina_stringshare_add(eina_strbuf_string_get(target));

        /* copy the file */
        if (!eio_file_copy_do(thread, &file_copy))
          {
             copy->progress.common.error = file_copy.common.error;
             goto on_error;
          }

        /* notify main thread */
        step++;
        eio_progress_send(thread, &copy->progress, step, count);

        if (ecore_thread_check(thread))
          goto on_error;

        eina_stringshare_del(file_copy.dest);
        eina_stringshare_del(file);
     }
   file_copy.dest = NULL;
   file = NULL;

   /* recreate link */
   /* no symbolic link on Windows */
#ifndef _WIN32
   if (!_eio_dir_link(thread, copy, &step, count, length_source, length_dest))
     goto on_error;
#endif

   /* set directory right back */
   if (!_eio_dir_chmod(thread, copy, &step, count, length_source, length_dest, EINA_FALSE))
     goto on_error;

 on_error:
   /* cleanup the mess */
   if (file_copy.dest) eina_stringshare_del(file_copy.dest);
   if (file) eina_stringshare_del(file);

   EINA_LIST_FREE(copy->files, file)
     eina_stringshare_del(file);
   EINA_LIST_FREE(copy->dirs, dir)
     eina_stringshare_del(dir);
   EINA_LIST_FREE(copy->links, ln)
     eina_stringshare_del(ln);

   if (!ecore_thread_check(thread))
     eio_progress_send(thread, &copy->progress, count, count);

   eina_strbuf_free(target);

   return;
}

static void
_eio_dir_copy_notify(void *data, Ecore_Thread *thread EINA_UNUSED, void *msg_data)
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
   eio_file_free(&copy->progress.common);
}

static void
_eio_dir_copy_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Dir_Copy *copy = data;

   copy->progress.common.done_cb((void*) copy->progress.common.data, &copy->progress.common);

   _eio_dir_copy_free(copy);
}

static void
_eio_dir_copy_error(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Dir_Copy *copy = data;

   eio_file_error(&copy->progress.common);

   _eio_dir_copy_free(copy);
}

static void
_eio_dir_move_heavy(void *data, Ecore_Thread *thread)
{
   Eio_Dir_Copy *move = data;
   const char *file = NULL;
   const char *dir = NULL;

   Eio_File_Progress file_move;
   Eina_Strbuf *target;

   int length_source;
   int length_dest;
   long long count;
   long long step;

   /* just try a rename, maybe we are lucky... */
   if (rename(move->progress.source, move->progress.dest) == 0)
     {
        /* we are really lucky */
        eio_progress_send(thread, &move->progress, 1, 1);
        return;
     }

   /* list all the content that should be moved */
   if (!_eio_dir_recursiv_ls(thread, move, move->progress.source))
     return;

   target = eina_strbuf_new();

   /* init all structure needed to move the file */
   if (!_eio_dir_init(thread, &step, &count, &length_source, &length_dest, move, &file_move))
     goto on_error;

   /* sub operation is a file move */
   file_move.op = EIO_FILE_MOVE;

   /* create all directory */
   if (!_eio_dir_mkdir(thread, move, &step, count, length_source, length_dest))
     goto on_error;

   /* move file around */
   EINA_LIST_FREE(move->files, file)
     {
        eina_strbuf_reset(target);

        /* build target file path */
        _eio_dir_target(move, target, file, length_source, length_dest);

        file_move.source = file;
        file_move.dest = eina_stringshare_add(eina_strbuf_string_get(target));

        /* first try to rename */
        if (rename(file_move.source, file_move.dest) < 0)
          {
             if (errno != EXDEV)
               {
                  eio_file_thread_error(&move->progress.common, thread);
                  goto on_error;
               }

             /* then try real copy */
             if (!eio_file_copy_do(thread, &file_move))
               {
                  move->progress.common.error = file_move.common.error;
                  goto on_error;
               }

             /* and unlink the original */
             if (unlink(file) != 0)
               {
                  eio_file_thread_error(&move->progress.common, thread);
                  goto on_error;
               }
          }

        step++;
        eio_progress_send(thread, &move->progress, step, count);

        if (ecore_thread_check(thread))
          goto on_error;

        eina_stringshare_del(file_move.dest);
        eina_stringshare_del(file);
     }
   file_move.dest = NULL;
   file = NULL;

   /* recreate link */
/* no symbolic link on Windows */
#ifndef _WIN32
   if (!_eio_dir_link(thread, move, &step, count, length_source, length_dest))
     goto on_error;
#endif

   /* set directory right back */
   if (!_eio_dir_chmod(thread, move, &step, count, length_source, length_dest, EINA_TRUE))
     goto on_error;

   if (rmdir(move->progress.source) != 0)
     goto on_error;

 on_error:
   /* cleanup the mess */
   if (file_move.dest) eina_stringshare_del(file_move.dest);
   if (file) eina_stringshare_del(file);

   EINA_LIST_FREE(move->files, file)
     eina_stringshare_del(file);
   EINA_LIST_FREE(move->dirs, dir)
     eina_stringshare_del(dir);

   if (!ecore_thread_check(thread))
     eio_progress_send(thread, &move->progress, count, count);

   eina_strbuf_free(target);

   return;
}

static void
_eio_dir_rmrf_heavy(void *data, Ecore_Thread *thread)
{
   Eio_Dir_Copy *rmrf = data;
   const char *file = NULL;
   const char *dir = NULL;

   long long count;
   long long step;

   /* list all the content that should be moved */
   if (!_eio_dir_recursiv_ls(thread, rmrf, rmrf->progress.source))
     return;

   /* init counter */
   step = 0;
   count = ((long long) eina_list_count(rmrf->files)) + ((long long) eina_list_count(rmrf->dirs)) + 1;

   EINA_LIST_FREE(rmrf->files, file)
     {
        if (unlink(file) != 0)
          {
             eio_file_thread_error(&rmrf->progress.common, thread);
             goto on_error;
          }

        eina_stringshare_replace(&rmrf->progress.dest, file);

        step++;
        eio_progress_send(thread, &rmrf->progress, step, count);

        if (ecore_thread_check(thread))
          goto on_error;

        eina_stringshare_del(file);
     }
   file = NULL;

   /* reverse directory listing, so the leaf would be destroyed before
      the root */
   rmrf->dirs = eina_list_reverse(rmrf->dirs);

   EINA_LIST_FREE(rmrf->dirs, dir)
     {
        if (rmdir(dir) != 0)
          {
             eio_file_thread_error(&rmrf->progress.common, thread);
             goto on_error;
          }

        eina_stringshare_replace(&rmrf->progress.dest, dir);

        step++;
        eio_progress_send(thread, &rmrf->progress, step, count);

        if (ecore_thread_check(thread))
          goto on_error;

        eina_stringshare_del(dir);
     }
   dir = NULL;

   if (rmdir(rmrf->progress.source) != 0)
     goto on_error;
   step++;

 on_error:
   if (dir) eina_stringshare_del(dir);
   if (file) eina_stringshare_del(file);

   EINA_LIST_FREE(rmrf->dirs, dir)
     eina_stringshare_del(dir);
   EINA_LIST_FREE(rmrf->files, file)
     eina_stringshare_del(file);

   if (!ecore_thread_check(thread))
     eio_progress_send(thread, &rmrf->progress, count, count);

   return;
}

static Eina_Bool
_eio_dir_stat_find_forward(Eio_File_Dir_Ls *async,
                           Eio_File *handler,
                           Eina_File_Direct_Info *info)
{
   Eina_Bool filter = EINA_TRUE;

   if (async->filter_cb)
     {
        filter = async->filter_cb((void*) async->ls.common.data, &async->ls.common, info);
     }

   if (filter)
     {
        Eio_File_Direct_Info *send_di;

        send_di = eio_direct_info_malloc();
        if (!send_di) return EINA_FALSE;

        memcpy(&send_di->info, info, sizeof (Eina_File_Direct_Info));
	send_di->associated = async->ls.common.worker.associated;
	async->ls.common.worker.associated = NULL;

        async->pack = eina_list_append(async->pack, send_di);
     }
   else if (async->ls.common.worker.associated)
     {
        eina_hash_free(async->ls.common.worker.associated);
        async->ls.common.worker.associated = NULL;
     }

   async->pack = eio_pack_send(handler->thread, async->pack, &async->start);

   return filter;
}

static void
_eio_dir_stat_find_heavy(void *data, Ecore_Thread *thread)
{
   Eio_File_Dir_Ls *async = data;

   async->ls.common.thread = thread;
   async->pack = NULL;
   async->start = ecore_time_get();

   _eio_file_recursiv_ls(thread, &async->ls.common,
                         (Eio_Filter_Direct_Cb) _eio_dir_stat_find_forward,
			 eina_file_stat_ls,
                         async, async->ls.directory);

   if (async->pack) ecore_thread_feedback(thread, async->pack);
   async->pack = NULL;
}

static void
_eio_dir_direct_find_heavy(void *data, Ecore_Thread *thread)
{
   Eio_File_Dir_Ls *async = data;

   async->ls.common.thread = thread;
   async->pack = NULL;
   async->start = ecore_time_get();

   _eio_file_recursiv_ls(thread, &async->ls.common,
                         (Eio_Filter_Direct_Cb) _eio_dir_stat_find_forward,
			 eina_file_direct_ls,
                         async, async->ls.directory);

   if (async->pack) ecore_thread_feedback(thread, async->pack);
   async->pack = NULL;
}

static void
_eio_dir_stat_done(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Ls *async = data;

   async->common.done_cb((void*) async->common.data, &async->common);

   eio_async_free(async);
}

static void
_eio_dir_stat_error(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Ls *async = data;

   eio_file_error(&async->common);

   eio_async_free(async);
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


EAPI Eio_File *
eio_dir_copy(const char *source,
             const char *dest,
             Eio_Filter_Direct_Cb filter_cb,
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

   copy = eio_common_alloc(sizeof(Eio_Dir_Copy));
   EINA_SAFETY_ON_NULL_RETURN_VAL(copy, NULL);

   copy->progress.op = EIO_DIR_COPY;
   copy->progress.progress_cb = progress_cb;
   copy->progress.source = eina_stringshare_add(source);
   copy->progress.dest = eina_stringshare_add(dest);
   copy->filter_cb = filter_cb;
   copy->files = NULL;
   copy->dirs = NULL;
   copy->links = NULL;

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

EAPI Eio_File *
eio_dir_move(const char *source,
             const char *dest,
             Eio_Filter_Direct_Cb filter_cb,
             Eio_Progress_Cb progress_cb,
             Eio_Done_Cb done_cb,
             Eio_Error_Cb error_cb,
             const void *data)
{
   Eio_Dir_Copy *move;

   EINA_SAFETY_ON_NULL_RETURN_VAL(source, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dest, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   move = eio_common_alloc(sizeof(Eio_Dir_Copy));
   EINA_SAFETY_ON_NULL_RETURN_VAL(move, NULL);

   move->progress.op = EIO_DIR_MOVE;
   move->progress.progress_cb = progress_cb;
   move->progress.source = eina_stringshare_add(source);
   move->progress.dest = eina_stringshare_add(dest);
   move->filter_cb = filter_cb;
   move->files = NULL;
   move->dirs = NULL;
   move->links = NULL;

   if (!eio_long_file_set(&move->progress.common,
                          done_cb,
                          error_cb,
                          data,
                          _eio_dir_move_heavy,
                          _eio_dir_copy_notify,
                          _eio_dir_copy_end,
                          _eio_dir_copy_error))
     return NULL;

   return &move->progress.common;
}

EAPI Eio_File *
eio_dir_unlink(const char *path,
               Eio_Filter_Direct_Cb filter_cb,
               Eio_Progress_Cb progress_cb,
               Eio_Done_Cb done_cb,
               Eio_Error_Cb error_cb,
               const void *data)
{
   Eio_Dir_Copy *rmrf;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   rmrf = eio_common_alloc(sizeof(Eio_Dir_Copy));
   EINA_SAFETY_ON_NULL_RETURN_VAL(rmrf, NULL);

   rmrf->progress.op = EIO_UNLINK;
   rmrf->progress.progress_cb = progress_cb;
   rmrf->progress.source = eina_stringshare_add(path);
   rmrf->progress.dest = NULL;
   rmrf->filter_cb = filter_cb;
   rmrf->files = NULL;
   rmrf->dirs = NULL;
   rmrf->links = NULL;

   if (!eio_long_file_set(&rmrf->progress.common,
                          done_cb,
                          error_cb,
                          data,
                          _eio_dir_rmrf_heavy,
                          _eio_dir_copy_notify,
                          _eio_dir_copy_end,
                          _eio_dir_copy_error))
     return NULL;

   return &rmrf->progress.common;
}

static Eio_File *
_eio_dir_stat_internal_ls(const char *dir,
                          Eio_Filter_Direct_Cb filter_cb,
                          Eio_Main_Direct_Cb main_cb,
                          Eio_Array_Cb main_internal_cb,
                          Eio_Done_Cb done_cb,
                          Eio_Error_Cb error_cb,
                          const void *data)
{
   Eio_File_Dir_Ls *async;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   async = eio_common_alloc(sizeof(Eio_File_Dir_Ls));
   EINA_SAFETY_ON_NULL_RETURN_VAL(async, NULL);

   /* Eio_Filter_Direct_Cb must be casted to Eio_Filter_Dir_Cb here
    * because we keep the Eio_File_Dir_Ls pointing to that variant
    * where info can be modified, but in our case it's already doing
    * stat() then it shouldn't be needed!
    */
   async->ls.directory = eina_stringshare_add(dir);
   async->filter_cb = (Eio_Filter_Dir_Cb)filter_cb;
   if (main_internal_cb)
     {
        async->main_internal_cb = main_internal_cb;
        async->ls.gather = EINA_TRUE;
     }
   else
     {
        async->main_cb = main_cb;
     }

   if (!eio_long_file_set(&async->ls.common,
                          done_cb,
                          error_cb,
                          data,
                          _eio_dir_stat_find_heavy,
                          _eio_direct_notify,
                          _eio_dir_stat_done,
                          _eio_dir_stat_error))
     return NULL;

   return &async->ls.common;
}

EAPI Eio_File *
eio_dir_stat_ls(const char *dir,
                Eio_Filter_Direct_Cb filter_cb,
                Eio_Main_Direct_Cb main_cb,
                Eio_Done_Cb done_cb,
                Eio_Error_Cb error_cb,
                const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(main_cb, NULL);

   return _eio_dir_stat_internal_ls(dir, filter_cb, main_cb, NULL, done_cb, error_cb, data);
}

Eio_File *
_eio_dir_stat_ls(const char *dir,
                 Eio_Array_Cb main_internal_cb,
                 Eio_Done_Cb done_cb,
                 Eio_Error_Cb error_cb,
                 const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(main_internal_cb, NULL);

   return _eio_dir_stat_internal_ls(dir, NULL, NULL, main_internal_cb, done_cb, error_cb, data);
}

static Eio_File *
_eio_dir_direct_internal_ls(const char *dir,
                            Eio_Filter_Dir_Cb filter_cb,
                            Eio_Main_Direct_Cb main_cb,
                            Eio_Array_Cb main_internal_cb,
                            Eio_Done_Cb done_cb,
                            Eio_Error_Cb error_cb,
                            const void *data)
{
   Eio_File_Dir_Ls *async;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   async = eio_common_alloc(sizeof(Eio_File_Dir_Ls));
   EINA_SAFETY_ON_NULL_RETURN_VAL(async, NULL);

   async->ls.directory = eina_stringshare_add(dir);
   async->filter_cb = filter_cb;
   if (main_internal_cb)
     {
        async->main_internal_cb = main_internal_cb;
        async->ls.gather = EINA_TRUE;
     }
   else
     {
        async->main_cb = main_cb;
     }

   if (!eio_long_file_set(&async->ls.common,
                          done_cb,
                          error_cb,
                          data,
                          _eio_dir_direct_find_heavy,
                          _eio_direct_notify,
                          _eio_dir_stat_done,
                          _eio_dir_stat_error))
     return NULL;

   return &async->ls.common;
}

EAPI Eio_File *
eio_dir_direct_ls(const char *dir,
		  Eio_Filter_Dir_Cb filter_cb,
		  Eio_Main_Direct_Cb main_cb,
		  Eio_Done_Cb done_cb,
		  Eio_Error_Cb error_cb,
		  const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(main_cb, NULL);

   return _eio_dir_direct_internal_ls(dir, filter_cb, main_cb, NULL, done_cb, error_cb, data);
}

Eio_File *
_eio_dir_direct_ls(const char *dir,
                   Eio_Array_Cb main_internal_cb,
                   Eio_Done_Cb done_cb,
                   Eio_Error_Cb error_cb,
                   const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(main_internal_cb, NULL);

   return _eio_dir_direct_internal_ls(dir, NULL, NULL, main_internal_cb, done_cb, error_cb, data);
}
