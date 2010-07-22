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

struct _Eio_List
{
   Eina_Iterator *it;
   Ecore_Thread *thread;
   const void *data;

   Eio_Done_Cb done;

   union
   {
      struct
      {
	 Eio_Filter_Direct_Cb filter;
	 Eio_Main_Direct_Cb main;
      } direct;
      struct
      {
	 Eio_Filter_Cb filter;
	 Eio_Main_Cb main;
      } str;
   } u;
};

static int _eio_count = 0;

static void
_eio_file_heavy(Ecore_Thread *thread, void *data)
{
   Eio_List *async;
   const char *file;

   async = data;

   EINA_ITERATOR_FOREACH(async->it, file)
     {
	Eina_Bool filter = EINA_TRUE;

	if (async->u.str.filter)
	  {
	     filter = async->u.str.filter(file, (void*) async->data);
	  }

	if (filter) ecore_thread_notify(thread, file);
	else eina_stringshare_del(file);
     }
}

static void
_eio_file_notify(__UNUSED__ Ecore_Thread *thread, void *msg_data, void *data)
{
   Eio_List *async;
   const char *file;

   async = data;
   file = msg_data;

   if (async->u.str.main)
     async->u.str.main(file, (void*) async->data);

   eina_stringshare_del(file);
}

static void
_eio_file_direct_heavy(Ecore_Thread *thread, void *data)
{
   Eio_List *async;
   const Eina_File_Direct_Info *info;

   async = data;

   EINA_ITERATOR_FOREACH(async->it, info)
     {
	Eina_Bool filter = EINA_TRUE;

	if (async->u.direct.filter)
	  {
	     filter = async->u.direct.filter(info, (void*) async->data);
	  }

	if (filter)
	  {
	     Eina_File_Direct_Info *send;

	     send = malloc(sizeof (Eina_File_Direct_Info));
	     if (!send) continue;

	     memcpy(send, info, sizeof (Eina_File_Direct_Info));
	     ecore_thread_notify(thread, send);
	  }
     }
}

static void
_eio_file_direct_notify(__UNUSED__ Ecore_Thread *thread, void *msg_data, void *data)
{
   Eio_List *async;
   Eina_File_Direct_Info *info;

   async = data;
   info = msg_data;

   if (async->u.direct.main)
     async->u.direct.main(info, (void*) async->data);

   free(info);
}

static void
_eio_file_end(void *data)
{
   Eio_List *async;

   async = data;

   if (async->done)
     async->done((void*) async->data);

   eina_iterator_free(async->it);
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

EAPI Eio_List *
eio_file_ls(const char *dir,
	       Eio_Filter_Cb filter,
	       Eio_Main_Cb main,
	       Eio_Done_Cb done,
	       const void *data)
{
   Eina_Iterator *ls;
   Eio_List *async = NULL;

   ls = eina_file_ls(dir);
   if (!ls) return NULL;

   async = malloc(sizeof (Eio_List));
   if (!async) goto on_error;

   async->u.str.filter = filter;
   async->u.str.main = main;
   async->done = done;
   async->data = data;
   async->it = ls;
   async->thread = ecore_long_run(_eio_file_heavy,
				  _eio_file_notify,
				  _eio_file_end,
				  _eio_file_end,
				  async,
				  EINA_FALSE);
   if (!async->thread) goto on_error;

   return async;


 on_error:
   free(async);
   eina_iterator_free(ls);
   return NULL;
}

EAPI Eio_List *
eio_file_direct_ls(const char *dir,
		      Eio_Filter_Direct_Cb filter,
		      Eio_Main_Direct_Cb main,
		      Eio_Done_Cb done,
		      const void *data)
{
   Eina_Iterator *ls;
   Eio_List *async = NULL;

   ls = eina_file_direct_ls(dir);
   if (!ls) return NULL;

   async = malloc(sizeof (Eio_List));
   if (!async) goto on_error;

   async->u.direct.filter = filter;
   async->u.direct.main = main;
   async->done = done;
   async->data = data;
   async->it = ls;
   async->thread = ecore_long_run(_eio_file_direct_heavy,
				  _eio_file_direct_notify,
				  _eio_file_end,
				  _eio_file_end,
				  async,
				  EINA_FALSE);
   if (!async->thread) goto on_error;

   return async;


 on_error:
   free(async);
   eina_iterator_free(ls);
   return NULL;
}

EAPI Eina_Bool
eio_file_cancel(Eio_List *list)
{
   return ecore_thread_cancel(list->thread);
}


