/* EIO - EFL data type library
 * Copyright (C) 2011 Enlightenment Developers:
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

#ifdef HAVE_XATTR
static void
_eio_ls_xattr_heavy(void *data, Ecore_Thread *thread)
{
   Eio_File_Char_Ls *async = data;
   char *tmp;
   ssize_t length;
   ssize_t i;

   length = listxattr(async->ls.directory, NULL, 0);

   if (length <= 0) return ;

   tmp = alloca(length);
   length = listxattr(async->ls.directory, tmp, length);

   for (i = 0; i < length; i += strlen(tmp) + 1)
     {
        const char *xattr;
        Eina_Bool filter = EINA_TRUE;

        xattr = eina_stringshare_add(tmp);

        if (async->filter_cb)
          {
             filter = async->filter_cb((void*) async->ls.common.data,
                                       &async->ls.common,
                                       xattr);
          }

        if (filter) ecore_thread_feedback(thread, xattr);
        else eina_stringshare_del(xattr);

        if (ecore_thread_check(thread))
          break;
     }
}

static void
_eio_ls_xattr_notify(void *data, Ecore_Thread *thread __UNUSED__, void *msg_data)
{
   Eio_File_Char_Ls *async = data;
   const char *xattr = msg_data;

   async->main_cb((void*) async->ls.common.data, &async->ls.common, xattr);
}

static void
_eio_file_xattr_get(void *data, Ecore_Thread *thread)
{
   Eio_File_Xattr *async = data;
   ssize_t sz;

   async->xattr_size = 0;
   async->xattr_data = NULL;

   sz = getxattr(async->path, async->attribute, NULL, 0);
   if (sz > 0 && sz < 2 * 1024 * 1024) /* sz should be smaller than 2MB (already huge in my opinion) */
     {
        async->xattr_data = malloc(sz);
        async->xattr_size = (unsigned int) sz;

        sz = getxattr(async->path, async->attribute, async->xattr_data, async->xattr_size);
        if (sz != async->xattr_size)
          {
             free(async->xattr_data);
             async->xattr_data = NULL;
             async->xattr_size = 0;

             ecore_thread_cancel(thread);
          }
     }
   else
     ecore_thread_cancel(thread);
}

static void
_eio_file_xattr_free(Eio_File_Xattr *async)
{
   eina_stringshare_del(async->path);
   eina_stringshare_del(async->attribute);
   free(async->xattr_data);
   free(async);
}

static void
_eio_file_xattr_get_done(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Xattr *async = data;

   if (async->done_cb)
     async->done_cb((void *) async->common.data, &async->common, async->xattr_data, async->xattr_size);

   _eio_file_xattr_free(async);
}

static void
_eio_file_xattr_get_error(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Xattr *async = data;

   eio_file_error(&async->common);
   _eio_file_xattr_free(async);
}

static void
_eio_file_xattr_set(void *data, Ecore_Thread *thread)
{
   Eio_File_Xattr *async = data;

   if (setxattr(async->path, async->attribute, async->xattr_data, async->xattr_size, async->flags))
     eio_file_thread_error(&async->common, thread);
   async->xattr_data = NULL;
}

static void
_eio_file_xattr_set_done(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Xattr *async = data;

   if (async->common.done_cb)
     async->common.done_cb((void*) async->common.data, &async->common);

   _eio_file_xattr_free(async);
}

static void
_eio_file_xattr_set_error(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Xattr *async = data;

   eio_file_error(&async->common);
   _eio_file_xattr_free(async);
}

#endif

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

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

EAPI Eio_File *
eio_file_xattr(const char *path,
	       Eio_Filter_Cb filter_cb,
	       Eio_Main_Cb main_cb,
	       Eio_Done_Cb done_cb,
	       Eio_Error_Cb error_cb,
	       const void *data)
{
#ifdef HAVE_XATTR
  Eio_File_Char_Ls *async;

  EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
  EINA_SAFETY_ON_NULL_RETURN_VAL(main_cb, NULL);
  EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
  EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

  async = malloc(sizeof (Eio_File_Char_Ls));
  EINA_SAFETY_ON_NULL_RETURN_VAL(async, NULL);

  async->filter_cb = filter_cb;
  async->main_cb = main_cb;
  async->ls.directory = eina_stringshare_add(path);

  if (!eio_long_file_set(&async->ls.common,
                         done_cb,
                         error_cb,
                         data,
                         _eio_ls_xattr_heavy,
                         _eio_ls_xattr_notify,
                         eio_async_end,
                         eio_async_error))
    return NULL;

  return &async->ls.common;
#else
  return NULL;
#endif
}

/**
 * @brief Retrieve the extended attribute of a file/directory.
 * @param path The path to retrieve the extended attribute from.
 * @param attribute The name of the attribute to retrieve.
 * @param done_cb Callback called from the main loop when getxattr succeeded.
 * @param error_cb Callback called from the main loop when getxattr failed or has been canceled.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_get call getxattr from another thread. This prevent lock in your apps.
 */

EAPI Eio_File *
eio_file_xattr_get(const char *path,
		   const char *attribute,
		   Eio_Done_Length_Cb done_cb,
		   Eio_Error_Cb error_cb,
                   const void *data)
{
#ifdef HAVE_XATTR
   Eio_File_Xattr *async;

   if (!path || !attribute || !done_cb || !error_cb)
     return NULL;

   async = malloc(sizeof (Eio_File_Xattr));
   if (!async) return NULL;

   async->path = eina_stringshare_add(path);
   async->attribute = eina_stringshare_add(attribute);
   async->done_cb = done_cb;

   if (!eio_file_set(&async->common,
                     NULL,
                     error_cb,
                     data,
                     _eio_file_xattr_get,
                     _eio_file_xattr_get_done,
                     _eio_file_xattr_get_error))
     return NULL;

   return &async->common;
#else
   return NULL;
#endif
}

/**
 * @brief Define the extented attribute on a file/directory.
 * @param path The path to set the attribute on.
 * @param attribute The name of the attribute to define.
 * @param xattr_data The data to link the attribute with.
 * @param xattr_size The size of the data to set.
 * @param done_cb The callback called from the main loop when setxattr succeeded.
 * @param error_cb The callback called from the main loop when setxattr failed.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_set call setxattr from another thread. This prevent lock in your apps. If
 * the writing succeeded, the done_cb will be called even if a cancel was requested, but came to late.
 */
EAPI Eio_File *
eio_file_xattr_set(const char *path,
                   const char *attribute,
                   const char *xattr_data,
                   unsigned int xattr_size,
                   Eio_Xattr_Flags flags,
                   Eio_Done_Cb done_cb,
                   Eio_Error_Cb error_cb,
                   const void *data)
{
#ifdef HAVE_XATTR
   Eio_File_Xattr *async;
   int iflags;

   if (!path || !attribute || !done_cb || !xattr_data || !xattr_size || !error_cb)
     return NULL;

   switch (flags)
     {
      case EIO_XATTR_INSERT: iflags = 0; break;
      case EIO_XATTR_REPLACE: iflags = XATTR_REPLACE; break;
      case EIO_XATTR_CREATED: iflags = XATTR_CREATE; break;
      default:
         return NULL;
     }

   async = malloc(sizeof (Eio_File_Xattr) + xattr_size);
   if (!async) return NULL;

   async->path = eina_stringshare_add(path);
   async->attribute = eina_stringshare_add(attribute);
   async->xattr_size = xattr_size;
   async->xattr_data = (char*) (async + 1);
   memcpy(async->xattr_data, xattr_data, xattr_size);
   async->flags = iflags;

   if (!eio_file_set(&async->common,
                     done_cb,
                     error_cb,
                     data,
                     _eio_file_xattr_set,
                     _eio_file_xattr_set_done,
                     _eio_file_xattr_set_error))
     return NULL;

   return &async->common;
#else
   return NULL;
#endif
}

/**
 * @}
 */
