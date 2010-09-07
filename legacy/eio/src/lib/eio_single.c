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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>

#include "eio_private.h"

#include "Eio.h"

void
eio_file_error(Eio_File *common)
{
   if (common->error_cb)
     common->error_cb(common->error, common->data);
}

void
eio_file_thread_error(Eio_File *common)
{
   common->error = errno;
   ecore_thread_cancel(common->thread);
}

Eina_Bool
eio_long_file_set(Eio_File *common,
		  Eio_Done_Cb done_cb,
		  Eio_Error_Cb error_cb,
		  const void *data,
		  Ecore_Thread_Heavy_Cb heavy_cb,
		  Ecore_Thread_Notify_Cb notify_cb,
		  Ecore_Cb end_cb,
		  Ecore_Cb cancel_cb)
{
   Ecore_Thread *thread;

   common->done_cb = done_cb;
   common->error_cb = error_cb;
   common->data = data;
   common->error = 0;

   /* Be aware that ecore_thread_run could call cancel_cb if something goes wrong. */
   thread = ecore_long_run(heavy_cb,
			   notify_cb,
			   end_cb,
			   cancel_cb,
			   common,
			   EINA_TRUE);

   if (!thread) return EINA_FALSE;
   common->thread = thread;
   return EINA_TRUE;
}

Eina_Bool
eio_file_set(Eio_File *common,
	     Eio_Done_Cb done_cb,
	     Eio_Error_Cb error_cb,
	     const void *data,
	     Ecore_Cb job_cb,
	     Ecore_Cb end_cb,
	     Ecore_Cb cancel_cb)
{
   Ecore_Thread *thread;

   common->done_cb = done_cb;
   common->error_cb = error_cb;
   common->data = data;
   common->error = 0;

   /* Be aware that ecore_thread_run could call cancel_cb if something goes wrong. */
   thread = ecore_thread_run(job_cb,
			     end_cb,
			     cancel_cb,
			     common);

   if (!thread) return EINA_FALSE;
   common->thread = thread;
   return EINA_TRUE;
}

/* --- */

static void
_eio_file_mkdir(void *data)
{
   Eio_File_Mkdir *m = data;

   if (mkdir(m->path, m->mode) != 0)
     eio_file_thread_error(&m->common);
}

static void
_eio_mkdir_free(Eio_File_Mkdir *m)
{
   eina_stringshare_del(m->path);
   free(m);
}

static void
_eio_file_mkdir_done(void *data)
{
   Eio_File_Mkdir *m = data;

   if (m->common.done_cb)
     m->common.done_cb(m->common.data);

   _eio_mkdir_free(m);
}

static void
_eio_file_mkdir_error(void *data)
{
   Eio_File_Mkdir *m = data;

   eio_file_error(&m->common);
   _eio_mkdir_free(m);
}

static void
_eio_file_unlink(void *data)
{
   Eio_File_Unlink *l = data;

   if (unlink(l->path) != 0)
     eio_file_thread_error(&l->common);
}

static void
_eio_unlink_free(Eio_File_Unlink *l)
{
   eina_stringshare_del(l->path);
   free(l);
}

static void
_eio_file_unlink_done(void *data)
{
   Eio_File_Unlink *l = data;

   if (l->common.done_cb)
     l->common.done_cb(l->common.data);

   _eio_unlink_free(l);
}

static void
_eio_file_unlink_error(void *data)
{
   Eio_File_Unlink *l = data;

   eio_file_error(&l->common);
   _eio_unlink_free(l);
}

static void
_eio_file_stat(void *data)
{
   Eio_File_Stat *s = data;

   if (stat(s->path, &s->buffer) != 0)
     eio_file_thread_error(&s->common);
}

static void
_eio_stat_free(Eio_File_Stat *s)
{
   eina_stringshare_del(s->path);
   free(s);
}

static void
_eio_file_stat_done(void *data)
{
   Eio_File_Stat *s = data;

   if (s->done_cb)
     s->done_cb(s->common.data, &s->buffer);

   _eio_stat_free(s);
}

static void
_eio_file_stat_error(void *data)
{
   Eio_File_Stat *s = data;

   eio_file_error(&s->common);
   _eio_stat_free(s);
}

/* ---- */

/**
 * @brief Stat a file/directory.
 * @param done_cb Callback called from the main loop when stat was successfully called..
 * @param error_cb Callback called from the main loop when stat failed or has been canceled.
 * @return A reference to the IO operation.
 *
 * eio_file_direct_stat basically call stat in another thread. This prevent any lock in your apps.
 */
EAPI Eio_File *
eio_file_direct_stat(const char *path,
		     Eio_Stat_Cb done_cb,
		     Eio_Error_Cb error_cb,
		     const void *data)
{
   Eio_File_Stat *s = NULL;

   if (!path || !done_cb || !error_cb)
     return NULL;

   s = malloc(sizeof (Eio_File_Stat));
   if (!s) return NULL;

   s->path = eina_stringshare_add(path);
   s->done_cb = done_cb;

   if (!eio_file_set(&s->common,
		      NULL,
		      error_cb,
		      data,
		      _eio_file_stat,
		      _eio_file_stat_done,
		      _eio_file_stat_error))
     return NULL;

   return &s->common;
}

/**
 * @brief Unlink a file/directory.
 * @param mode The permission to set, follow (mode & ~umask & 0777).
 * @param done_cb Callback called from the main loop when the directory has been created.
 * @param error_cb Callback called from the main loop when the directory failed to be created or has been canceled.
 * @return A reference to the IO operation.
 *
 * eio_file_unlink basically call unlink in another thread. This prevent any lock in your apps.
 */
EAPI Eio_File *
eio_file_unlink(const char *path,
		Eio_Done_Cb done_cb,
		Eio_Error_Cb error_cb,
		const void *data)
{
   Eio_File_Unlink *l = NULL;

   if (!path || !done_cb || !error_cb)
     return NULL;

   l = malloc(sizeof (Eio_File_Unlink));
   if (!l) return NULL;

   l->path = eina_stringshare_add(path);

   if (!eio_file_set(&l->common,
		      done_cb,
		      error_cb,
		      data,
		      _eio_file_unlink,
		      _eio_file_unlink_done,
		      _eio_file_unlink_error))
     return NULL;

   return &l->common;
}

/**
 * @brief Create a new directory.
 * @param path The directory path to create.
 * @param mode The permission to set, follow (mode & ~umask & 0777).
 * @param done_cb Callback called from the main loop when the directory has been created.
 * @param error_cb Callback called from the main loop when the directory failed to be created or has been canceled.
 * @return A reference to the IO operation.
 *
 * eio_file_mkdir basically call mkdir in another thread. This prevent any lock in your apps.
 */
EAPI Eio_File *
eio_file_mkdir(const char *path,
	       mode_t mode,
	       Eio_Done_Cb done_cb,
	       Eio_Error_Cb error_cb,
	       const void *data)
{
   Eio_File_Mkdir *r = NULL;

   if (!path || !done_cb || !error_cb)
     return NULL;

   r = malloc(sizeof (Eio_File_Mkdir));
   if (!r) return NULL;

   r->path = eina_stringshare_add(path);
   r->mode = mode;

   if (!eio_file_set(&r->common,
		     done_cb,
		     error_cb,
		      data,
		     _eio_file_mkdir,
		     _eio_file_mkdir_done,
		     _eio_file_mkdir_error))
     return NULL;

   return &r->common;
}


