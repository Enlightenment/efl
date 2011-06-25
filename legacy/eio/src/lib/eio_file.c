/* EIO - EFL data type library
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *           Vincent "caro" Torri  <vtorri at univ-evry dot fr>
 *           Stephen "okra" Houston <UnixTitan@gmail.com>
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

/**
 * @page tutorial_file_ls eio_file_ls() tutorial
 *
 * To use eio_file_ls(), you just need to define four callbacks:
 *
 * @li The filter callback, which allow or not a file to be seen
 * by the main loop handler. This callback run in a separated thread.
 * @li The main callback, which receive in the main loop all the file
 * that are allowed by the filter. If you are updating a user interface
 * it make sense to delay the insertion a little, so you get a chance
 * to update the canvas for a bunch of file instead of one by one.
 * @li The end callback, which is called in the main loop when the
 * content of the directory has been correctly scanned and all the
 * file notified to the main loop.
 * @li The error callback, which is called if an error occured or
 * if the listing was cancelled during it's run. You can then retrieve
 * the error type as an errno error.
 *
 * Here is a simple example:
 *
 * @code
 * #include <Ecore.h>
 * #include <Eio.h>
 *
 * static Eina_Bool
 * _test_filter_cb(void *data, Eio_File *handler, const char *file)
 * {
 *    fprintf(stderr, "ACCEPTING: %s\n", file);
 *    return EINA_TRUE;
 * }
 *
 * static void
 * _test_main_cb(void *data, Eio_File *handler, const char *file)
 * {
 *    fprintf(stderr, "PROCESS: %s\n", file);
 * }
 *
 * static void
 * _test_done_cb(void *data, Eio_File *handler)
 * {
 *    printf("ls done\n");
 *    ecore_main_loop_quit();
 * }
 *
 * static void
 * _test_error_cb(void *data, Eio_File *handler, int error)
 * {
 *    fprintf(stderr, "error: [%s]\n", strerror(error));
 *    ecore_main_loop_quit();
 * }
 *
 * int
 * main(int argc, char **argv)
 * {
 *    Eio_File *cp;
 *
 *    if (argc != 2)
 *      {
 * 	fprintf(stderr, "eio_ls directory\n");
 * 	return -1;
 *      }
 *
 *    ecore_init();
 *    eio_init();
 *
 *    cp = eio_file_ls(argv[1],
 *                     _test_filter_cb,
 *                     _test_main_cb,
 *                     _test_done_cb,
 *                     _test_error_cb,
 *                     NULL);
 *
 *    ecore_main_loop_begin();
 *
 *    eio_shutdown();
 *    ecore_shutdown();
 *
 *    return 0;
 * }
 *
 * @endcode
 */

#include "eio_private.h"
#include "Eio.h"

#ifdef HAVE_XATTR
# include <sys/xattr.h>
#endif

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static void
_eio_file_heavy(void *data, Ecore_Thread *thread)
{
   Eio_File_Char_Ls *async = data;
   Eina_Iterator *ls;
   const char *file;

   ls = eina_file_ls(async->ls.directory);
   if (!ls)
     {
	eio_file_thread_error(&async->ls.common, thread);
	return ;
     }

   eio_file_container_set(&async->ls.common, eina_iterator_container_get(ls));

   EINA_ITERATOR_FOREACH(ls, file)
     {
	Eina_Bool filter = EINA_TRUE;

	if (async->filter_cb)
	  {
	     filter = async->filter_cb((void*) async->ls.common.data, &async->ls.common, file);
	  }

	if (filter)
          {
             Eio_File_Char *send;

             send = eio_char_malloc();
             if (!send) goto on_error;

             send->filename = file;
	     send->associated = async->ls.common.worker.associated;
	     async->ls.common.worker.associated = NULL;

             ecore_thread_feedback(thread, send);
          }
	else
          {
          on_error:
             eina_stringshare_del(file);

             if (async->ls.common.worker.associated)
               {
                  eina_hash_free(async->ls.common.worker.associated);
                  async->ls.common.worker.associated = NULL;
               }
          }

	if (ecore_thread_check(thread))
	  break;
     }

   eio_file_container_set(&async->ls.common, NULL);

   eina_iterator_free(ls);
}

static void
_eio_file_notify(void *data, Ecore_Thread *thread __UNUSED__, void *msg_data)
{
   Eio_File_Char_Ls *async = data;
   Eio_File_Char *info = msg_data;

   async->ls.common.main.associated = info->associated;

   async->main_cb((void*) async->ls.common.data,
                  &async->ls.common,
                  info->filename);

   if (async->ls.common.main.associated)
     {
        eina_hash_free(async->ls.common.main.associated);
        async->ls.common.main.associated = NULL;
     }

   eina_stringshare_del(info->filename);
   eio_char_free(info);
}

static void
_eio_file_eina_ls_heavy(Ecore_Thread *thread, Eio_File_Direct_Ls *async, Eina_Iterator *ls)
{
   const Eina_File_Direct_Info *info;

   if (!ls)
     {
	eio_file_thread_error(&async->ls.common, thread);
	return ;
     }

   eio_file_container_set(&async->ls.common, eina_iterator_container_get(ls));

   EINA_ITERATOR_FOREACH(ls, info)
     {
	Eina_Bool filter = EINA_TRUE;

	if (async->filter_cb)
	  {
	     filter = async->filter_cb((void*) async->ls.common.data, &async->ls.common, info);
	  }

	if (filter)
	  {
	     Eio_File_Direct_Info *send;

	     send = eio_direct_info_malloc();
	     if (!send) continue;

	     memcpy(&send->info, info, sizeof (Eina_File_Direct_Info));
	     send->associated = async->ls.common.worker.associated;
	     async->ls.common.worker.associated = NULL;

	     ecore_thread_feedback(thread, send);
	  }
	else if (async->ls.common.worker.associated)
	  {
             eina_hash_free(async->ls.common.worker.associated);
             async->ls.common.worker.associated = NULL;
	  }

	if (ecore_thread_check(thread))
	  break;
     }

   eio_file_container_set(&async->ls.common, NULL);

   eina_iterator_free(ls);
}

static void
_eio_file_direct_heavy(void *data, Ecore_Thread *thread)
{
   Eio_File_Direct_Ls *async = data;
   Eina_Iterator *ls;

   ls = eina_file_direct_ls(async->ls.directory);

   _eio_file_eina_ls_heavy(thread, async, ls);
}

static void
_eio_file_stat_heavy(void *data, Ecore_Thread *thread)
{
   Eio_File_Direct_Ls *async = data;
   Eina_Iterator *ls;

   ls = eina_file_stat_ls(async->ls.directory);

   _eio_file_eina_ls_heavy(thread, async, ls);
}

static void
_eio_file_direct_notify(void *data, Ecore_Thread *thread __UNUSED__, void *msg_data)
{
   Eio_File_Direct_Ls *async = data;
   Eio_File_Direct_Info *info = msg_data;

   async->ls.common.main.associated = info->associated;

   async->main_cb((void*) async->ls.common.data,
		  &async->ls.common,
		  &info->info);

   if (async->ls.common.main.associated)
     {
        eina_hash_free(async->ls.common.main.associated);
        async->ls.common.main.associated = NULL;
     }

   eio_direct_info_free(info);
}

#ifdef HAVE_XATTR
static void
_eio_file_copy_xattr(Ecore_Thread *thread __UNUSED__,
                     Eio_File_Progress *op __UNUSED__,
                     int in, int out)
{
   char *tmp;
   ssize_t length;
   ssize_t i;

   length = flistxattr(in, NULL, 0);

   if (length <= 0) return ;

   tmp = alloca(length);
   length = flistxattr(in, tmp, length);

   for (i = 0; i < length; i += strlen(tmp) + 1)
     {
        ssize_t attr_length;
        void *value;

        attr_length = fgetxattr(in, tmp, NULL, 0);
        if (!attr_length) continue ;

        value = malloc(attr_length);
        if (!value) continue ;
        attr_length = fgetxattr(in, tmp, value, attr_length);

        if (attr_length > 0)
          fsetxattr(out, tmp, value, attr_length, 0);

        free(value);
     }
}
#endif

static Eina_Bool
_eio_file_write(int fd, void *mem, ssize_t length)
{
   ssize_t count;

   if (length == 0) return EINA_TRUE;

   count = write(fd, mem, length);
   if (count != length) return EINA_FALSE;
   return EINA_TRUE;
}

#ifndef MAP_HUGETLB
# define MAP_HUGETLB 0
#endif

static Eina_Bool
_eio_file_copy_mmap(Ecore_Thread *thread, Eio_File_Progress *op, int in, int out, long long size)
{
   char *m = MAP_FAILED;
   long long i;

   for (i = 0; i < size; i += EIO_PACKET_SIZE * EIO_PACKET_COUNT)
     {
	int j;
	int k;
	int c;

	m = mmap(NULL, EIO_PACKET_SIZE * EIO_PACKET_COUNT, PROT_READ, MAP_FILE | MAP_HUGETLB | MAP_SHARED, in, i);
	if (m == MAP_FAILED)
	  goto on_error;

	madvise(m, EIO_PACKET_SIZE * EIO_PACKET_COUNT, MADV_SEQUENTIAL);

	c = size - i;
	if (c - EIO_PACKET_SIZE * EIO_PACKET_COUNT > 0)
	  c = EIO_PACKET_SIZE * EIO_PACKET_COUNT;
	else
	  c = size - i;

	for (j = EIO_PACKET_SIZE, k = 0; j < c; k = j, j += EIO_PACKET_SIZE)
	  {
	     if (!_eio_file_write(out, m + k, EIO_PACKET_SIZE))
	       goto on_error;

	     eio_progress_send(thread, op, i + j, size);
	  }

	if (!_eio_file_write(out, m + k, c - k))
	  goto on_error;

	eio_progress_send(thread, op, i + c, size);

	munmap(m, EIO_PACKET_SIZE * EIO_PACKET_COUNT);
	m = MAP_FAILED;

	if (ecore_thread_check(thread))
          goto on_error;
     }

   return EINA_TRUE;

 on_error:
   if (m != MAP_FAILED) munmap(m, EIO_PACKET_SIZE * EIO_PACKET_COUNT);
   return EINA_FALSE;
}

#ifdef EFL_HAVE_SPLICE
static int
_eio_file_copy_splice(Ecore_Thread *thread, Eio_File_Progress *op, int in, int out, long long size)
{
   int result = 0;
   long long count;
   long long i;
   int pipefd[2];

   if (pipe(pipefd) < 0)
     return -1;

   for (i = 0; i < size; i += count)
     {
	count = splice(in, 0, pipefd[1], NULL, EIO_PACKET_SIZE * EIO_PACKET_COUNT, SPLICE_F_MORE | SPLICE_F_MOVE);
	if (count < 0) goto on_error;

	count = splice(pipefd[0], NULL, out, 0, count, SPLICE_F_MORE | SPLICE_F_MOVE);
	if (count < 0) goto on_error;

	eio_progress_send(thread, op, i, size);

	if (ecore_thread_check(thread))
          goto on_error;
     }

   result = 1;

 on_error:
   if (result != 1 && (errno == EBADF || errno == EINVAL)) result = -1;
   close(pipefd[0]);
   close(pipefd[1]);

   return result;
}
#endif

static void
_eio_file_copy_heavy(void *data, Ecore_Thread *thread)
{
   Eio_File_Progress *copy = data;

   eio_file_copy_do(thread, copy);
}

static void
_eio_file_copy_notify(void *data, Ecore_Thread *thread __UNUSED__, void *msg_data)
{
   Eio_File_Progress *copy = data;

   eio_progress_cb(msg_data, copy);
}

static void
_eio_file_copy_free(Eio_File_Progress *copy)
{
   eina_stringshare_del(copy->source);
   eina_stringshare_del(copy->dest);
   free(copy);
}

static void
_eio_file_copy_end(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Progress *copy = data;

   copy->common.done_cb((void*) copy->common.data, &copy->common);

   _eio_file_copy_free(copy);
}

static void
_eio_file_copy_error(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Progress *copy = data;

   eio_file_error(&copy->common);

   _eio_file_copy_free(copy);
}

static void
_eio_file_move_free(Eio_File_Move *move)
{
   eina_stringshare_del(move->progress.source);
   eina_stringshare_del(move->progress.dest);
   free(move);
}

static void
_eio_file_move_copy_progress(void *data, Eio_File *handler __UNUSED__, const Eio_Progress *info)
{
   Eio_File_Move *move = data;

   move->progress.progress_cb((void*) move->progress.common.data, &move->progress.common, info);
}

static void
_eio_file_move_unlink_done(void *data, Eio_File *handler __UNUSED__)
{
   Eio_File_Move *move = data;

   move->progress.common.done_cb((void*) move->progress.common.data, &move->progress.common);

   _eio_file_move_free(move);
}

static void
_eio_file_move_unlink_error(void *data, Eio_File *handler __UNUSED__, int error)
{
   Eio_File_Move *move = data;

   move->copy = NULL;

   move->progress.common.error = error;
   eio_file_error(&move->progress.common);

   _eio_file_move_free(move);
}

static void
_eio_file_move_copy_done(void *data, Eio_File *handler __UNUSED__)
{
   Eio_File_Move *move = data;
   Eio_File *rm;

   rm = eio_file_unlink(move->progress.source,
			_eio_file_move_unlink_done,
			_eio_file_move_unlink_error,
			move);
   if (rm) move->copy = rm;
}

static void
_eio_file_move_copy_error(void *data, Eio_File *handler __UNUSED__, int error)
{
   Eio_File_Move *move = data;

   move->progress.common.error = error;
   eio_file_error(&move->progress.common);

   _eio_file_move_free(move);
}

static void
_eio_file_move_heavy(void *data, Ecore_Thread *thread)
{
   Eio_File_Move *move = data;

   if (rename(move->progress.source, move->progress.dest) < 0)
     eio_file_thread_error(&move->progress.common, thread);
   else
     eio_progress_send(thread, &move->progress, 1, 1);
}

static void
_eio_file_move_notify(void *data, Ecore_Thread *thread __UNUSED__, void *msg_data)
{
   Eio_File_Move *move = data;

   eio_progress_cb(msg_data, &move->progress);
}

static void
_eio_file_move_end(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Move *move = data;

   move->progress.common.done_cb((void*) move->progress.common.data, &move->progress.common);

   _eio_file_move_free(move);
}

static void
_eio_file_move_error(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Move *move = data;

   if (move->copy)
     {
	eio_file_cancel(move->copy);
	return ;
     }

   if (move->progress.common.error == EXDEV)
     {
	Eio_File *eio_cp;

	eio_cp = eio_file_copy(move->progress.source, move->progress.dest,
			       move->progress.progress_cb ? _eio_file_move_copy_progress : NULL,
			       _eio_file_move_copy_done,
			       _eio_file_move_copy_error,
			       move);

	if (eio_cp)
          {
             move->copy = eio_cp;

             move->progress.common.thread = ((Eio_File_Progress*)move->copy)->common.thread;
             return ;
          }
     }

   eio_file_error(&move->progress.common);

   _eio_file_move_free(move);
}

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

void
eio_progress_cb(Eio_Progress *progress, Eio_File_Progress *op)
{
   op->progress_cb((void *) op->common.data, &op->common, progress);

   eio_progress_free(progress);
}

Eina_Bool
eio_file_copy_do(Ecore_Thread *thread, Eio_File_Progress *copy)
{
   struct stat buf;
   int result = -1;
   int in = -1;
   int out = -1;

   in = open(copy->source, O_RDONLY);
   if (in < 0)
     {
	eio_file_thread_error(&copy->common, thread);
	return EINA_FALSE;
     }

   /*
     As we need file size for progression information and both copy method
     call fstat (better than stat as it avoid race condition).
    */
   if (fstat(in, &buf) < 0)
     goto on_error;

   /* open write */
   out = open(copy->dest, O_WRONLY | O_CREAT | O_TRUNC, buf.st_mode);
   if (out < 0)
     goto on_error;

#ifdef EFL_HAVE_SPLICE
   /* fast file copy code using Linux splice API */
   result = _eio_file_copy_splice(thread, copy, in, out, buf.st_size);
   if (result == 0)
     goto on_error;
#endif

   /* classic copy method using mmap and write */
   if (result == -1 && !_eio_file_copy_mmap(thread, copy, in, out, buf.st_size))
     goto on_error;

   /* change access right to match source */
#ifdef HAVE_CHMOD
   if (fchmod(out, buf.st_mode) != 0)
     goto on_error;
#else
   if (chmod(copy->dest, buf.st_mode) != 0)
     goto on_error;
#endif

#ifdef HAVE_XATTR
   _eio_file_copy_xattr(thread, copy, in, out);
#endif

   close(out);
   close(in);

   return EINA_TRUE;

 on_error:
   eio_file_thread_error(&copy->common, thread);

   if (in >= 0) close(in);
   if (out >= 0) close(out);
   if (out >= 0)
     unlink(copy->dest);
   return EINA_FALSE;
}

void
eio_async_end(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Ls *async = data;

   async->common.done_cb((void*) async->common.data, &async->common);

   eina_stringshare_del(async->directory);
   free(async);
}

void
eio_async_error(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Ls *async = data;

   eio_file_error(&async->common);

   eina_stringshare_del(async->directory);
   free(async);
}

/**
 * @endcond
 */


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Eio_Group Eio Reference API
 *
 * @{
 */

/**
 * @brief List content of a directory without locking your app.
 * @param dir The directory to list.
 * @param filter_cb Callback called from another thread.
 * @param main_cb Callback called from the main loop for each accepted file.
 * @param done_cb Callback called from the main loop when the content of the directory has been listed.
 * @param error_cb Callback called from the main loop when the directory could not be opened or listing content has been canceled.
 * @param data Data passed to callback and not modified at all by eio_file_ls.
 * @return A reference to the IO operation.
 *
 * eio_file_ls run eina_file_ls in a separated thread using ecore_thread_feedback_run. This prevent
 * any lock in your apps.
 */
EAPI Eio_File *
eio_file_ls(const char *dir,
	    Eio_Filter_Cb filter_cb,
	    Eio_Main_Cb main_cb,
	    Eio_Done_Cb done_cb,
	    Eio_Error_Cb error_cb,
	    const void *data)
{
   Eio_File_Char_Ls *async;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(main_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   async = malloc(sizeof (Eio_File_Char_Ls));
   EINA_SAFETY_ON_NULL_RETURN_VAL(async, NULL);

   async->filter_cb = filter_cb;
   async->main_cb = main_cb;
   async->ls.directory = eina_stringshare_add(dir);

   if (!eio_long_file_set(&async->ls.common,
			  done_cb,
			  error_cb,
			  data,
			  _eio_file_heavy,
			  _eio_file_notify,
			  eio_async_end,
			  eio_async_error))
     return NULL;

   return &async->ls.common;
}

/**
 * @brief List content of a directory without locking your app.
 * @param dir The directory to list.
 * @param filter_cb Callback called from another thread.
 * @param main_cb Callback called from the main loop for each accepted file.
 * @param done_cb Callback called from the main loop when the content of the directory has been listed.
 * @param error_cb Callback called from the main loop when the directory could not be opened or listing content has been canceled.
 * @param data Data passed to callback and not modified at all by eio_file_direct_ls.
 * @return A reference to the IO operation.
 *
 * eio_file_direct_ls run eina_file_direct_ls in a separated thread using
 * ecore_thread_feedback_run. This prevent any lock in your apps.
 */
EAPI Eio_File *
eio_file_direct_ls(const char *dir,
		   Eio_Filter_Direct_Cb filter_cb,
		   Eio_Main_Direct_Cb main_cb,
		   Eio_Done_Cb done_cb,
		   Eio_Error_Cb error_cb,
		   const void *data)
{
   Eio_File_Direct_Ls *async;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(main_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   async = malloc(sizeof(Eio_File_Direct_Ls));
   EINA_SAFETY_ON_NULL_RETURN_VAL(async, NULL);

   async->filter_cb = filter_cb;
   async->main_cb = main_cb;
   async->ls.directory = eina_stringshare_add(dir);

   if (!eio_long_file_set(&async->ls.common,
			  done_cb,
			  error_cb,
			  data,
			  _eio_file_direct_heavy,
			  _eio_file_direct_notify,
			  eio_async_end,
			  eio_async_error))
     return NULL;

   return &async->ls.common;
}

/**
 * @brief List content of a directory without locking your app.
 * @param dir The directory to list.
 * @param filter_cb Callback called from another thread.
 * @param main_cb Callback called from the main loop for each accepted file.
 * @param done_cb Callback called from the main loop when the content of the directory has been listed.
 * @param error_cb Callback called from the main loop when the directory could not be opened or listing content has been canceled.
 * @param data Data passed to callback and not modified at all by eio_file_stat_ls.
 * @return A reference to the IO operation.
 *
 * eio_file_stat_ls() run eina_file_stat_ls() in a separated thread using
 * ecore_thread_feedback_run. This prevent any lock in your apps.
 */
EAPI Eio_File *
eio_file_stat_ls(const char *dir,
                 Eio_Filter_Direct_Cb filter_cb,
                 Eio_Main_Direct_Cb main_cb,
                 Eio_Done_Cb done_cb,
                 Eio_Error_Cb error_cb,
                 const void *data)
{
   Eio_File_Direct_Ls *async;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(main_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   async = malloc(sizeof(Eio_File_Direct_Ls));
   EINA_SAFETY_ON_NULL_RETURN_VAL(async, NULL);

   async->filter_cb = filter_cb;
   async->main_cb = main_cb;
   async->ls.directory = eina_stringshare_add(dir);

   if (!eio_long_file_set(&async->ls.common,
			  done_cb,
			  error_cb,
			  data,
			  _eio_file_stat_heavy,
			  _eio_file_direct_notify,
			  eio_async_end,
			  eio_async_error))
     return NULL;

   return &async->ls.common;
}

/**
 * @brief Cancel any Eio_File.
 * @param ls The asynchronous IO operation to cancel.
 * @return EINA_FALSE if the destruction is delayed, EINA_TRUE if it's done.
 *
 * This will cancel any kind of IO operation and cleanup the mess. This means
 * that it could take time to cancel an IO.
 */
EAPI Eina_Bool
eio_file_cancel(Eio_File *ls)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ls, EINA_FALSE);
   return ecore_thread_cancel(ls->thread);
}

/**
 * @brief Check if an Eio_File operation has been cancelled.
 * @param ls The asynchronous IO operation to check.
 * @return EINA_TRUE if it was canceled, EINA_FALSE other wise.
 *
 * In case of an error it also return EINA_TRUE.
 */
EAPI Eina_Bool
eio_file_check(Eio_File *ls)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ls, EINA_TRUE);
   return ecore_thread_check(ls->thread);
}

/**
 * @brief Return the container during EIO operation
 * @param ls The asynchronous IO operation to retrieve container from.
 * @return NULL if not available, a DIRP if it is.
 *
 * This is only available and make sense in the thread callback, not in
 * the mainloop.
 */
EAPI void *
eio_file_container_get(Eio_File *ls)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ls, EINA_FALSE);
   return ls->container;
}

/**
 * @brief Associate data with the current filtered file.
 * @param ls The Eio_File ls request currently calling the filter callback.
 * @param key The key to associate data to.
 * @param data The data to associate the data to.
 * @param free_cb The function to call to free the associated data, @p free will be called if not specified.
 * @return EINA_TRUE if insertion was fine.
 *
 * This function could only be safely called from within the filter callback.
 * If you don't need to copy the key around you can use @ref eio_file_associate_direct_add
 */
EAPI Eina_Bool
eio_file_associate_add(Eio_File *ls,
                       const char *key,
                       const void *data, Eina_Free_Cb free_cb)
{
   /* FIXME: Check if we are in the right worker thred */
   if (!ls->worker.associated)
     ls->worker.associated = eina_hash_string_small_new(eio_associate_free);

   return eina_hash_add(ls->worker.associated,
                        key,
                        eio_associate_malloc(data, free_cb));
}

/**
 * @brief Associate data with the current filtered file.
 * @param ls The Eio_File ls request currently calling the filter callback.
 * @param key The key to associate data to (will not be copied, and the pointer will be used as long as the file is not notified).
 * @param data The data to associate the data to.
 * @param free_cb The function to call to free the associated data, @p free will be called if not specified.
 * @return EINA_TRUE if insertion was fine.
 *
 * This function could only be safely called from within the filter callback.
 * If you need eio to make a proper copy of the @p key to be safe use
 * @ref eio_file_associate_add instead.
 */
EAPI Eina_Bool
eio_file_associate_direct_add(Eio_File *ls,
                              const char *key,
                              const void *data, Eina_Free_Cb free_cb)
{
   /* FIXME: Check if we are in the right worker thred */
   if (!ls->worker.associated)
     ls->worker.associated = eina_hash_string_small_new(eio_associate_free);

   return eina_hash_direct_add(ls->worker.associated,
                               key,
                               eio_associate_malloc(data, free_cb));
}

/**
 * @brief Get the data associated during the filter callback inside the main loop
 * @param ls The Eio_File ls request currently calling the notify callback.
 * @param key The key pointing to the data to retrieve.
 * @return the data associated with the key or @p NULL if not found.
 */
EAPI void *
eio_file_associate_find(Eio_File *ls, const char *key)
{
   Eio_File_Associate *search;

   if (!ls->main.associated)
     return NULL;

   search = eina_hash_find(ls->main.associated, key);
   if (!search) return NULL;
   return search->data;
}

/**
 * @brief Copy a file asynchronously
 * @param source Should be the name of the file to copy the data from.
 * @param dest Should be the name of the file to copy the data to.
 * @param progress_cb Callback called to know the progress of the copy.
 * @param done_cb Callback called when the copy is done.
 * @param error_cb Callback called when something goes wrong.
 * @param data Private data given to callback.
 *
 * This function will copy a file from source to dest. It will try to use splice
 * if possible, if not it will fallback to mmap/write. It will try to preserve
 * access right, but not user/group identification.
 */
EAPI Eio_File *
eio_file_copy(const char *source,
	      const char *dest,
	      Eio_Progress_Cb progress_cb,
	      Eio_Done_Cb done_cb,
	      Eio_Error_Cb error_cb,
	      const void *data)
{
   Eio_File_Progress *copy;

   EINA_SAFETY_ON_NULL_RETURN_VAL(source, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dest, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   copy = malloc(sizeof(Eio_File_Progress));
   EINA_SAFETY_ON_NULL_RETURN_VAL(copy, NULL);

   copy->op = EIO_FILE_COPY;
   copy->progress_cb = progress_cb;
   copy->source = eina_stringshare_add(source);
   copy->dest = eina_stringshare_add(dest);

   if (!eio_long_file_set(&copy->common,
			  done_cb,
			  error_cb,
			  data,
			  _eio_file_copy_heavy,
			  _eio_file_copy_notify,
			  _eio_file_copy_end,
			  _eio_file_copy_error))
     return NULL;

   return &copy->common;
}

/**
 * @brief Move a file asynchronously
 * @param source Should be the name of the file to move the data from.
 * @param dest Should be the name of the file to move the data to.
 * @param progress_cb Callback called to know the progress of the move.
 * @param done_cb Callback called when the move is done.
 * @param error_cb Callback called when something goes wrong.
 * @param data Private data given to callback.
 *
 * This function will copy a file from source to dest. It will try to use splice
 * if possible, if not it will fallback to mmap/write. It will try to preserve
 * access right, but not user/group identification.

 */
EAPI Eio_File *
eio_file_move(const char *source,
	      const char *dest,
	      Eio_Progress_Cb progress_cb,
	      Eio_Done_Cb done_cb,
	      Eio_Error_Cb error_cb,
	      const void *data)
{
   Eio_File_Move *move;

   EINA_SAFETY_ON_NULL_RETURN_VAL(source, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dest, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   move = malloc(sizeof(Eio_File_Move));
   EINA_SAFETY_ON_NULL_RETURN_VAL(move, NULL);

   move->progress.op = EIO_FILE_MOVE;
   move->progress.progress_cb = progress_cb;
   move->progress.source = eina_stringshare_add(source);
   move->progress.dest = eina_stringshare_add(dest);
   move->copy = NULL;

   if (!eio_long_file_set(&move->progress.common,
			  done_cb,
			  error_cb,
			  data,
			  _eio_file_move_heavy,
			  _eio_file_move_notify,
			  _eio_file_move_end,
			  _eio_file_move_error))
     return NULL;

   return &move->progress.common;
}

/**
 * @}
 */
