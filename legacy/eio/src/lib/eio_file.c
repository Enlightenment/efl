/* EIO - EFL data type library
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *           Vincent "caro" Torri  <vtorri at univ-evry dot fr>
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

#include <Ecore.h>

#include "Eio.h"

typedef struct _Eio_File_Ls Eio_File_Ls;
typedef struct _Eio_File_Direct_Ls Eio_File_Direct_Ls;
typedef struct _Eio_File_Char_Ls Eio_File_Char_Ls;

struct _Eio_File
{
   Ecore_Thread *thread;
   const void *data;

   Eio_Done_Cb done_cb;
   Eio_Done_Cb error_cb;
};

struct _Eio_File_Ls
{
   Eio_File common;
   const char *directory;
};

struct _Eio_File_Direct_Ls
{
   Eio_File_Ls ls;

   Eio_Filter_Direct_Cb filter_cb;
   Eio_Main_Direct_Cb main_cb;
};

struct _Eio_File_Char_Ls
{
   Eio_File_Ls ls;

   Eio_Filter_Cb filter_cb;
   Eio_Main_Cb main_cb;
};

static int _eio_count = 0;

static void
_eio_file_heavy(Ecore_Thread *thread, void *data)
{
   Eio_File_Char_Ls *async;
   Eina_Iterator *ls;
   const char *file;

   async = data;

   ls = eina_file_ls(async->ls.directory);
   if (!ls)
     {
	ecore_thread_cancel(thread);
	return ;
     }

   EINA_ITERATOR_FOREACH(ls, file)
     {
	Eina_Bool filter = EINA_TRUE;

	if (async->filter_cb)
	  {
	     filter = async->filter_cb(file, (void*) async->ls.common.data);
	  }

	if (filter) ecore_thread_notify(thread, file);
	else eina_stringshare_del(file);

	if (ecore_thread_check(thread))
	  break;
     }

   eina_iterator_free(ls);
}

static void
_eio_file_notify(Ecore_Thread *thread __UNUSED__, void *msg_data, void *data)
{
   Eio_File_Char_Ls *async;
   const char *file;

   async = data;
   file = msg_data;

   if (async->main_cb)
     async->main_cb(file, (void*) async->ls.common.data);

   eina_stringshare_del(file);
}

static void
_eio_file_direct_heavy(Ecore_Thread *thread, void *data)
{
   Eio_File_Direct_Ls *async;
   Eina_Iterator *ls;
   const Eina_File_Direct_Info *info;

   async = data;

   ls = eina_file_direct_ls(async->ls.directory);
   if (!ls)
     {
	ecore_thread_cancel(thread);
	return ;
     }

   EINA_ITERATOR_FOREACH(ls, info)
     {
	Eina_Bool filter = EINA_TRUE;

	if (async->filter_cb)
	  {
	     filter = async->filter_cb(info, (void*) async->ls.common.data);
	  }

	if (filter)
	  {
	     Eina_File_Direct_Info *send;

	     send = malloc(sizeof (Eina_File_Direct_Info));
	     if (!send) continue;

	     memcpy(send, info, sizeof (Eina_File_Direct_Info));
	     ecore_thread_notify(thread, send);
	  }

	if (ecore_thread_check(thread))
	  break;
     }

   eina_iterator_free(ls);
}

static void
_eio_file_direct_notify(Ecore_Thread *thread __UNUSED__, void *msg_data, void *data)
{
   Eio_File_Direct_Ls *async;
   Eina_File_Direct_Info *info;

   async = data;
   info = msg_data;

   if (async->main_cb)
     async->main_cb(info, (void*) async->ls.common.data);

   free(info);
}

static void
_eio_file_end(void *data)
{
   Eio_File_Ls *async;

   async = data;

   if (async->common.done_cb)
     async->common.done_cb((void*) async->common.data);

   eina_stringshare_del(async->directory);
   free(async);
}

static void
_eio_file_error(void *data)
{
   Eio_File_Ls *async;

   async = data;

   if (async->common.error_cb)
     async->common.error_cb((void*) async->common.data);

   eina_stringshare_del(async->directory);
   free(async);
}

EAPI int
eio_init(void)
{
   _eio_count++;

   if (_eio_count > 1) return _eio_count;

   eina_init();
   ecore_init();

   return _eio_count;
}

EAPI int
eio_shutdown(void)
{
   _eio_count--;

   if (_eio_count > 0) return _eio_count;

   ecore_shutdown();
   eina_shutdown();
   return _eio_count;
}

/**
 * @brief List content of a directory without locking your app.
 * @param dir The directory to list.
 * @param filter_cb Callback called from another thread.
 * @param main_cb Callback called from the main loop for each accepted file.
 * @param done_cb Callback called from the main loop when the content of the directory has been listed.
 * @param error_cb Callback called from the main loop when the directory could not be opened or listing content has been canceled.
 * @return A reference to the IO operation.
 *
 * eio_file_ls run eina_file_ls in a separated thread using ecore_long_run. This prevent
 * any lock in your apps.
 */
EAPI Eio_File *
eio_file_ls(const char *dir,
	    Eio_Filter_Cb filter_cb,
	    Eio_Main_Cb main_cb,
	    Eio_Done_Cb done_cb,
	    Eio_Done_Cb error_cb,
	    const void *data)
{
   Eio_File_Char_Ls *async = NULL;

   async = malloc(sizeof (Eio_File_Char_Ls));
   if (!async) goto on_error;

   async->filter_cb = filter_cb;
   async->main_cb = main_cb;
   async->ls.directory = eina_stringshare_add(dir);
   async->ls.common.done_cb = done_cb;
   async->ls.common.error_cb = error_cb;
   async->ls.common.data = data;
   async->ls.common.thread = ecore_long_run(_eio_file_heavy,
					    _eio_file_notify,
					    _eio_file_end,
					    _eio_file_error,
					    async,
					    EINA_FALSE);
   if (!async->ls.common.thread) goto on_error;

   return &async->ls.common;

 on_error:
   free(async);
   return NULL;
}

/**
 * @brief List content of a directory without locking your app.
 * @param dir The directory to list.
 * @param filter_cb Callback called from another thread.
 * @param main_cb Callback called from the main loop for each accepted file.
 * @param done_cb Callback called from the main loop when the content of the directory has been listed.
 * @param error_cb Callback called from the main loop when the directory could not be opened or listing content has been canceled.
 * @return A reference to the IO operation.
 *
 * eio_file_direct_ls run eina_file_direct_ls in a separated thread using
 * ecore_long_run. This prevent any lock in your apps.
 */
EAPI Eio_File *
eio_file_direct_ls(const char *dir,
		   Eio_Filter_Direct_Cb filter_cb,
		   Eio_Main_Direct_Cb main_cb,
		   Eio_Done_Cb done_cb,
		   Eio_Done_Cb error_cb,
		   const void *data)
{
   Eio_File_Direct_Ls *async = NULL;

   async = malloc(sizeof (Eio_File_Direct_Ls));
   if (!async) goto on_error;

   async->filter_cb = filter_cb;
   async->main_cb = main_cb;
   async->ls.directory = eina_stringshare_add(dir);
   async->ls.common.done_cb = done_cb;
   async->ls.common.error_cb = error_cb;
   async->ls.common.data = data;
   async->ls.common.thread = ecore_long_run(_eio_file_direct_heavy,
					 _eio_file_direct_notify,
					 _eio_file_end,
					 _eio_file_error,
					 async,
					 EINA_FALSE);
   if (!async->ls.common.thread) goto on_error;

   return &async->ls.common;


 on_error:
   free(async);
   return NULL;
}

EAPI Eina_Bool
eio_file_cancel(Eio_File *ls)
{
   return ecore_thread_cancel(ls->thread);
}


