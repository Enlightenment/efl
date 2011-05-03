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
 * @}
 */
