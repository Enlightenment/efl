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

#include "eio_private.h"
#include "Eio.h"

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
   Eina_List *pack = NULL;
   double start;

   ls = eina_file_ls(async->ls.directory);
   if (!ls)
     {
	eio_file_thread_error(&async->ls.common, thread);
	return;
     }

   eio_file_container_set(&async->ls.common, eina_iterator_container_get(ls));

   start = ecore_time_get();

   EINA_ITERATOR_FOREACH(ls, file)
     {
	Eina_Bool filter = EINA_TRUE;

	if (async->filter_cb)
	  {
	     filter = async->filter_cb((void*) async->ls.common.data, &async->ls.common, file);
	  }

	if (filter)
          {
             Eio_File_Char *send_fc;

             send_fc = eio_char_malloc();
             if (!send_fc) goto on_error;

             send_fc->filename = file;
	     send_fc->associated = async->ls.common.worker.associated;
	     async->ls.common.worker.associated = NULL;

	     pack = eina_list_append(pack, send_fc);
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

        pack = eio_pack_send(thread, pack, &start);

	if (ecore_thread_check(thread))
	  break;
     }

   if (pack) ecore_thread_feedback(thread, pack);

   eio_file_container_set(&async->ls.common, NULL);

   eina_iterator_free(ls);
}

static void
_eio_file_notify(void *data, Ecore_Thread *thread EINA_UNUSED, void *msg_data)
{
   Eio_File_Char_Ls *async = data;
   Eina_List *pack = msg_data;
   Eio_File_Char *info;

   EINA_LIST_FREE(pack, info)
     {
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
}

static void
_eio_file_eina_ls_heavy(Ecore_Thread *thread, Eio_File_Direct_Ls *async, Eina_Iterator *ls)
{
   const Eina_File_Direct_Info *info;
   Eina_List *pack = NULL;
   double start;

   if (!ls)
     {
	eio_file_thread_error(&async->ls.common, thread);
	return;
     }

   eio_file_container_set(&async->ls.common, eina_iterator_container_get(ls));

   start = ecore_time_get();

   EINA_ITERATOR_FOREACH(ls, info)
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
	     if (!send_di) continue;

	     memcpy(&send_di->info, info, sizeof (Eina_File_Direct_Info));
	     send_di->associated = async->ls.common.worker.associated;
	     async->ls.common.worker.associated = NULL;

             pack = eina_list_append(pack, send_di);
	  }
	else if (async->ls.common.worker.associated)
	  {
             eina_hash_free(async->ls.common.worker.associated);
             async->ls.common.worker.associated = NULL;
	  }

        pack = eio_pack_send(thread, pack, &start);

	if (ecore_thread_check(thread))
	  break;
     }

   if (pack) ecore_thread_feedback(thread, pack);

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
_eio_file_direct_notify(void *data, Ecore_Thread *thread EINA_UNUSED, void *msg_data)
{
   Eio_File_Direct_Ls *async = data;
   Eina_List *pack = msg_data;
   Eio_File_Direct_Info *info;

   EINA_LIST_FREE(pack, info)
     {
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
}

#ifndef MAP_HUGETLB
# define MAP_HUGETLB 0
#endif

static void
_eio_file_copy_heavy(void *data, Ecore_Thread *thread)
{
   Eio_File_Progress *copy = data;

   eio_file_copy_do(thread, copy);
}

static void
_eio_file_copy_notify(void *data, Ecore_Thread *thread EINA_UNUSED, void *msg_data)
{
   Eio_File_Progress *copy = data;

   eio_progress_cb(msg_data, copy);
}

static void
_eio_file_copy_free(Eio_File_Progress *copy)
{
   eina_stringshare_del(copy->source);
   eina_stringshare_del(copy->dest);
   eio_file_free(&copy->common);
}

static void
_eio_file_copy_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Progress *copy = data;

   copy->common.done_cb((void*) copy->common.data, &copy->common);

   _eio_file_copy_free(copy);
}

static void
_eio_file_copy_error(void *data, Ecore_Thread *thread EINA_UNUSED)
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
   eio_file_free(&move->progress.common);
}

static void
_eio_file_move_copy_progress(void *data, Eio_File *handler EINA_UNUSED, const Eio_Progress *info)
{
   Eio_File_Move *move = data;

   move->progress.progress_cb((void*) move->progress.common.data, &move->progress.common, info);
}

static void
_eio_file_move_unlink_done(void *data, Eio_File *handler EINA_UNUSED)
{
   Eio_File_Move *move = data;

   move->progress.common.done_cb((void*) move->progress.common.data, &move->progress.common);

   _eio_file_move_free(move);
}

static void
_eio_file_move_unlink_error(void *data, Eio_File *handler EINA_UNUSED, int error)
{
   Eio_File_Move *move = data;

   move->copy = NULL;

   move->progress.common.error = error;
   eio_file_error(&move->progress.common);

   _eio_file_move_free(move);
}

static void
_eio_file_move_copy_done(void *data, Eio_File *handler EINA_UNUSED)
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
_eio_file_move_copy_error(void *data, Eio_File *handler EINA_UNUSED, int error)
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
_eio_file_move_notify(void *data, Ecore_Thread *thread EINA_UNUSED, void *msg_data)
{
   Eio_File_Move *move = data;

   eio_progress_cb(msg_data, &move->progress);
}

static void
_eio_file_move_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Move *move = data;

   move->progress.common.done_cb((void*) move->progress.common.data, &move->progress.common);

   _eio_file_move_free(move);
}

static void
_eio_file_move_error(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Move *move = data;

   if (move->copy)
     {
	eio_file_cancel(move->copy);
	return;
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
             return;
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

static Eina_Bool
_eio_file_copy_progress(void *data, unsigned long long done, unsigned long long total)
{
   void **ctx = data;
   Ecore_Thread *thread = ctx[0];
   Eio_File_Progress *copy = ctx[1];

   eio_progress_send(thread, copy, done, total);

   return !ecore_thread_check(thread);
}

Eina_Bool
eio_file_copy_do(Ecore_Thread *thread, Eio_File_Progress *copy)
{
   void *ctx[2] = {thread, copy};
   Eina_Bool ret = eina_file_copy(copy->source, copy->dest,
                                  (EINA_FILE_COPY_PERMISSION |
                                   EINA_FILE_COPY_XATTR),
                                  _eio_file_copy_progress,
                                  ctx);

   if (!ret)
     {
        eio_file_thread_error(&copy->common, thread);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

void
eio_async_free(Eio_File_Ls *async)
{
   eina_stringshare_del(async->directory);
   eio_file_free(&async->common);
}

void
eio_async_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Ls *async = data;

   async->common.done_cb((void*) async->common.data, &async->common);

   eio_async_free(async);
}

void
eio_async_error(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_File_Ls *async = data;

   eio_file_error(&async->common);

   eio_async_free(async);
}

/**
 * @endcond
 */


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

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

EAPI Eina_Bool
eio_file_cancel(Eio_File *ls)
{
   if (!ls) return EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(ls, EINA_FALSE);
   return ecore_thread_cancel(ls->thread);
}

EAPI Eina_Bool
eio_file_check(Eio_File *ls)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ls, EINA_TRUE);
   return ecore_thread_check(ls->thread);
}

EAPI void *
eio_file_container_get(Eio_File *ls)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ls, NULL);
   return ls->container;
}

EAPI Eina_Bool
eio_file_associate_add(Eio_File *ls,
                       const char *key,
                       const void *data, Eina_Free_Cb free_cb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ls, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, EINA_FALSE);
   /* FIXME: Check if we are in the right worker thred */
   if (!ls->worker.associated)
     ls->worker.associated = eina_hash_string_small_new(eio_associate_free);

   return eina_hash_add(ls->worker.associated,
                        key,
                        eio_associate_malloc(data, free_cb));
}

EAPI Eina_Bool
eio_file_associate_direct_add(Eio_File *ls,
                              const char *key,
                              const void *data, Eina_Free_Cb free_cb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ls, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, EINA_FALSE);
   /* FIXME: Check if we are in the right worker thred */
   if (!ls->worker.associated)
     ls->worker.associated = eina_hash_string_small_new(eio_associate_free);

   return eina_hash_direct_add(ls->worker.associated,
                               key,
                               eio_associate_malloc(data, free_cb));
}

EAPI void *
eio_file_associate_find(Eio_File *ls, const char *key)
{
   Eio_File_Associate *search;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ls, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(key, NULL);
   if (!ls->main.associated)
     return NULL;

   search = eina_hash_find(ls->main.associated, key);
   if (!search) return NULL;
   return search->data;
}

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
