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

#include "eio_private.h"

#include "Eio.h"

static void
_eio_file_mkdir_done(void *data)
{
   Eio_File_Mkdir *r = data;

   if (r->common.done_cb)
     r->common.done_cb(r->common.data);

   eina_stringshare_del(r->path);
   free(r);
}

static void
_eio_file_mkdir_error(void *data)
{
   Eio_File_Mkdir *r = data;

   if (r->common.error_cb)
     r->common.error_cb(r->common.data);

   eina_stringshare_del(r->path);
   free(r);
}

static void
_eio_file_mkdir(void *data)
{
   Eio_File_Mkdir *r = data;

   if (mkdir(r->path, r->mode) != 0)
     ecore_thread_cancel(r->common.thread);
}

static void
_eio_file_unlink(void *data)
{
   Eio_File_Unlink *l = data;

   if (unlink(l->path) != 0)
     ecore_thread_cancel(l->common.thread);
}

static void
_eio_file_unlink_done(void *data)
{
   Eio_File_Unlink *l = data;

   if (l->common.done_cb)
     l->common.done_cb(l->common.data);

   eina_stringshare_del(l->path);
   free(l);
}

static void
_eio_file_unlink_error(void *data)
{
   Eio_File_Unlink *l = data;

   if (l->common.error_cb)
     l->common.error_cb(l->common.data);

   eina_stringshare_del(l->path);
   free(l);
}

/* ---- */

EAPI Eio_File *
eio_file_direct_stat(const char *path,
		     Eio_Stat_Cb done_cb,
		     Eio_Done_Cb error_cb,
		     const void *data)
{
   return NULL;
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
		Eio_Done_Cb error_cb,
		const void *data)
{
   Eio_File_Unlink *l = NULL;

   if (!path || !done_cb || !error_cb)
     return NULL;

   l = malloc(sizeof (Eio_File_Unlink));
   if (!l) return NULL;

   l->path = eina_stringshare_add(path);
   l->common.done_cb = done_cb;
   l->common.error_cb = error_cb;
   l->common.data = data;
   l->common.thread = ecore_thread_run(_eio_file_unlink,
				       _eio_file_unlink_done,
				       _eio_file_unlink_error,
				       l);
   if (!l->common.thread) goto on_error;

   return &l->common;

 on_error:
   eina_stringshare_del(l->path);
   free(l);
   return NULL;
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
	       Eio_Done_Cb error_cb,
	       const void *data)
{
   Eio_File_Mkdir *r = NULL;

   if (!path || !done_cb || !error_cb)
     return NULL;

   r = malloc(sizeof (Eio_File_Mkdir));
   if (!r) return NULL;

   r->path = eina_stringshare_add(path);
   r->mode = mode;
   r->common.done_cb = done_cb;
   r->common.error_cb = error_cb;
   r->common.data = data;
   r->common.thread = ecore_thread_run(_eio_file_mkdir,
				       _eio_file_mkdir_done,
				       _eio_file_mkdir_error,
				       r);
   if (!r->common.thread) goto on_error;

   return &r->common;

 on_error:
   eina_stringshare_del(r->path);
   free(r);
   return NULL;
}


