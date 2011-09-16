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

static void
_eio_ls_xattr_heavy(void *data, Ecore_Thread *thread)
{
   Eio_File_Char_Ls *async = data;
   Eina_Iterator *it;
   const char *tmp;

   it = eina_xattr_ls(async->ls.directory);
   if (!it) return ;

   EINA_ITERATOR_FOREACH(it, tmp)
     {
        Eina_Bool filter = EINA_TRUE;

        if (async->filter_cb)
          {
             filter = async->filter_cb((void*) async->ls.common.data,
                                       &async->ls.common,
                                       tmp);
          }

        if (filter) ecore_thread_feedback(thread, eina_stringshare_add(tmp));

        if (ecore_thread_check(thread))
          break;
     }

   eina_iterator_free(it);
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
   Eina_Bool failure = EINA_FALSE;
   const char *file;
   const char *attribute;

   file = async->path;
   attribute = async->attribute;

   switch (async->op)
     {
     case EIO_XATTR_DATA:
       async->todo.xdata.xattr_size = 0;
       async->todo.xdata.xattr_data = NULL;

       async->todo.xdata.xattr_data = eina_xattr_get(file, attribute, &async->todo.xdata.xattr_size);
       if (!async->todo.xdata.xattr_data) failure = EINA_TRUE;
       break;
     case EIO_XATTR_STRING:
       async->todo.xstring.xattr_string = eina_xattr_string_get(file, attribute);
       if (!async->todo.xstring.xattr_string) failure = EINA_TRUE;
       break;
     case EIO_XATTR_DOUBLE:
       failure = !eina_xattr_double_get(file, attribute, &async->todo.xdouble.xattr_double);
       break;
     case EIO_XATTR_INT:
       failure = !eina_xattr_int_get(file, attribute, &async->todo.xint.xattr_int);
       break;
     }

   if (failure)
     ecore_thread_cancel(thread);
}

static void
_eio_file_xattr_free(Eio_File_Xattr *async)
{
   eina_stringshare_del(async->path);
   eina_stringshare_del(async->attribute);
   if (async->op == EIO_XATTR_DATA) free(async->todo.xdata.xattr_data);
   if (async->op == EIO_XATTR_STRING) free(async->todo.xstring.xattr_string);
   free(async);
}

static void
_eio_file_xattr_get_done(void *data, Ecore_Thread *thread __UNUSED__)
{
   Eio_File_Xattr *async = data;

   switch (async->op)
     {
     case EIO_XATTR_DATA:
       if (async->todo.xdata.done_cb)
	 async->todo.xdata.done_cb((void *) async->common.data, &async->common, async->todo.xdata.xattr_data, async->todo.xdata.xattr_size);
       break;
     case EIO_XATTR_STRING:
       if (async->todo.xstring.done_cb)
	 async->todo.xstring.done_cb((void *) async->common.data, &async->common, async->todo.xstring.xattr_string);
       break;
     case EIO_XATTR_DOUBLE:
       if (async->todo.xdouble.done_cb)
	 async->todo.xdouble.done_cb((void *) async->common.data, &async->common, async->todo.xdouble.xattr_double);
       break;
     case EIO_XATTR_INT:
       if (async->todo.xint.done_cb)
	 async->todo.xint.done_cb((void *) async->common.data, &async->common, async->todo.xint.xattr_int);
       break;
     }

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
   const char *file;
   const char *attribute;
   Eina_Xattr_Flags flags;
   Eina_Bool failure = EINA_FALSE;

   file = async->path;
   attribute = async->attribute;
   flags = async->flags;

   switch (async->op)
     {
     case EIO_XATTR_DATA:
       failure = !eina_xattr_set(file, attribute, async->todo.xdata.xattr_data, async->todo.xdata.xattr_size, flags);
       break;
     case EIO_XATTR_STRING:
       failure = !eina_xattr_string_set(file, attribute, async->todo.xstring.xattr_string, flags);
       break;
     case EIO_XATTR_DOUBLE:
       failure = !eina_xattr_double_set(file, attribute, async->todo.xdouble.xattr_double, flags);
       break;
     case EIO_XATTR_INT:
       failure = !eina_xattr_int_set(file, attribute, async->todo.xint.xattr_int, flags);
       break;
     }

   if (failure) eio_file_thread_error(&async->common, thread);
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

static Eio_File *
_eio_file_xattr_setup_get(Eio_File_Xattr *async,
			  const char *path,
			  const char *attribute,
			  Eio_Error_Cb error_cb,
			  const void *data)
{
   async->path = eina_stringshare_add(path);
   async->attribute = eina_stringshare_add(attribute);

   if (!eio_file_set(&async->common,
                     NULL,
                     error_cb,
                     data,
                     _eio_file_xattr_get,
                     _eio_file_xattr_get_done,
                     _eio_file_xattr_get_error))
     return NULL;

   return &async->common;
}

static Eio_File *
_eio_file_xattr_setup_set(Eio_File_Xattr *async,
			  const char *path,
			  const char *attribute,
			  Eina_Xattr_Flags flags,
			  Eio_Done_Cb done_cb,
			  Eio_Error_Cb error_cb,
			  const void *data)
{
   async->path = eina_stringshare_add(path);
   async->attribute = eina_stringshare_add(attribute);
   async->flags = flags;

   if (!eio_file_set(&async->common,
                     done_cb,
                     error_cb,
                     data,
                     _eio_file_xattr_set,
                     _eio_file_xattr_set_done,
                     _eio_file_xattr_set_error))
     return NULL;

   return &async->common;
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
		   Eio_Done_Data_Cb done_cb,
		   Eio_Error_Cb error_cb,
                   const void *data)
{
   Eio_File_Xattr *async;

   if (!path || !attribute || !done_cb || !error_cb)
     return NULL;

   async = malloc(sizeof (Eio_File_Xattr));
   if (!async) return NULL;

   async->op = EIO_XATTR_DATA;
   async->todo.xdata.done_cb = done_cb;

   return _eio_file_xattr_setup_get(async, path, attribute, error_cb, data);
}

/**
 * @brief Retrieve a string extended attribute of a file/directory.
 * @param path The path to retrieve the extended attribute from.
 * @param attribute The name of the attribute to retrieve.
 * @param done_cb Callback called from the main loop when getxattr succeeded.
 * @param error_cb Callback called from the main loop when getxattr failed or has been canceled.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_string_get call eina_xattr_string_get from another thread. This prevent lock in your apps.
 */
EAPI Eio_File *
eio_file_xattr_string_get(const char *path,
			  const char *attribute,
			  Eio_Done_String_Cb done_cb,
			  Eio_Error_Cb error_cb,
			  const void *data)
{
   Eio_File_Xattr *async;

   if (!path || !attribute || !done_cb || !error_cb)
     return NULL;

   async = malloc(sizeof (Eio_File_Xattr));
   if (!async) return NULL;

   async->op = EIO_XATTR_STRING;
   async->todo.xstring.done_cb = done_cb;

   return _eio_file_xattr_setup_get(async, path, attribute, error_cb, data);
}

/**
 * @brief Retrieve a extended attribute of a file/directory.
 * @param path The path to retrieve the extended attribute from.
 * @param attribute The name of the attribute to retrieve.
 * @param done_cb Callback called from the main loop when getxattr succeeded.
 * @param error_cb Callback called from the main loop when getxattr failed or has been canceled.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_double_get call eina_xattr_double_get from another thread. This prevent lock in your apps.
 */
EAPI Eio_File *
eio_file_xattr_double_get(const char *path,
			  const char *attribute,
			  Eio_Done_Double_Cb done_cb,
			  Eio_Error_Cb error_cb,
			  const void *data)
{
   Eio_File_Xattr *async;

   if (!path || !attribute || !done_cb || !error_cb)
     return NULL;

   async = malloc(sizeof (Eio_File_Xattr));
   if (!async) return NULL;

   async->op = EIO_XATTR_DOUBLE;
   async->todo.xdouble.done_cb = done_cb;

   return _eio_file_xattr_setup_get(async, path, attribute, error_cb, data);
}

/**
 * @brief Retrieve a extended attribute of a file/directory.
 * @param path The path to retrieve the extended attribute from.
 * @param attribute The name of the attribute to retrieve.
 * @param done_cb Callback called from the main loop when getxattr succeeded.
 * @param error_cb Callback called from the main loop when getxattr failed or has been canceled.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_int_get call eina_xattr_int_get from another thread. This prevent lock in your apps.
 */
EAPI Eio_File *
eio_file_xattr_int_get(const char *path,
		       const char *attribute,
		       Eio_Done_Int_Cb done_cb,
		       Eio_Error_Cb error_cb,
		       const void *data)
{
   Eio_File_Xattr *async;

   if (!path || !attribute || !done_cb || !error_cb)
     return NULL;

   async = malloc(sizeof (Eio_File_Xattr));
   if (!async) return NULL;

   async->op = EIO_XATTR_INT;
   async->todo.xint.done_cb = done_cb;

   return _eio_file_xattr_setup_get(async, path, attribute, error_cb, data);
}

/**
 * @brief Define an extented attribute on a file/directory.
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
                   Eina_Xattr_Flags flags,
                   Eio_Done_Cb done_cb,
                   Eio_Error_Cb error_cb,
                   const void *data)
{
   Eio_File_Xattr *async;

   if (!path || !attribute || !done_cb || !xattr_data || !xattr_size || !error_cb)
     return NULL;

   async = malloc(sizeof (Eio_File_Xattr) + xattr_size);
   if (!async) return NULL;

   async->op = EIO_XATTR_DATA;
   async->todo.xdata.xattr_size = xattr_size;
   async->todo.xdata.xattr_data = (char*) (async + 1);
   memcpy(async->todo.xdata.xattr_data, xattr_data, xattr_size);

   return _eio_file_xattr_setup_set(async, path, attribute, flags, done_cb, error_cb, data);
}

/**
 * @brief Define a string extented attribute on a file/directory.
 * @param path The path to set the attribute on.
 * @param attribute The name of the attribute to define.
 * @param xattr_string The string to link the attribute with.
 * @param done_cb The callback called from the main loop when setxattr succeeded.
 * @param error_cb The callback called from the main loop when setxattr failed.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_string_set call eina_xattr_string_set from another thread. This prevent lock in your apps. If
 * the writing succeeded, the done_cb will be called even if a cancel was requested, but came to late.
 */
EAPI Eio_File *
eio_file_xattr_string_set(const char *path,
			  const char *attribute,
			  const char *xattr_string,
			  Eina_Xattr_Flags flags,
			  Eio_Done_Cb done_cb,
			  Eio_Error_Cb error_cb,
			  const void *data)
{
   Eio_File_Xattr *async;
   int length;

   if (!path || !attribute || !done_cb || !xattr_string || !error_cb)
     return NULL;

   async = malloc(sizeof (Eio_File_Xattr));
   if (!async) return NULL;

   length = strlen(xattr_string) + 1;

   async->op = EIO_XATTR_STRING;
   async->todo.xstring.xattr_string = malloc(length);
   if (!async->todo.xstring.xattr_string)
     {
       free(async);
       return NULL;
     }
   memcpy(async->todo.xstring.xattr_string, xattr_string, length);

   return _eio_file_xattr_setup_set(async, path, attribute, flags, done_cb, error_cb, data);
}

/**
 * @brief Define an extented attribute on a file/directory.
 * @param path The path to set the attribute on.
 * @param attribute The name of the attribute to define.
 * @param xattr_double The value to link the attribute with.
 * @param done_cb The callback called from the main loop when setxattr succeeded.
 * @param error_cb The callback called from the main loop when setxattr failed.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_double_set call eina_xattr_double_set from another thread. This prevent lock in your apps. If
 * the writing succeeded, the done_cb will be called even if a cancel was requested, but came to late.
 */
EAPI Eio_File *
eio_file_xattr_double_set(const char *path,
			  const char *attribute,
			  double xattr_double,
			  Eina_Xattr_Flags flags,
			  Eio_Done_Cb done_cb,
			  Eio_Error_Cb error_cb,
			  const void *data)
{
   Eio_File_Xattr *async;

   if (!path || !attribute || !done_cb || !error_cb)
     return NULL;

   async = malloc(sizeof (Eio_File_Xattr));
   if (!async) return NULL;

   async->op = EIO_XATTR_DOUBLE;
   async->todo.xdouble.xattr_double = xattr_double;

   return _eio_file_xattr_setup_set(async, path, attribute, flags, done_cb, error_cb, data);
}

/**
 * @brief Define an extented attribute on a file/directory.
 * @param path The path to set the attribute on.
 * @param attribute The name of the attribute to define.
 * @param xattr_int The value to link the attribute with.
 * @param done_cb The callback called from the main loop when setxattr succeeded.
 * @param error_cb The callback called from the main loop when setxattr failed.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_int_set call eina_xattr_int_set from another thread. This prevent lock in your apps. If
 * the writing succeeded, the done_cb will be called even if a cancel was requested, but came to late.
 */
EAPI Eio_File *
eio_file_xattr_int_set(const char *path,
		       const char *attribute,
		       int xattr_int,
		       Eina_Xattr_Flags flags,
		       Eio_Done_Cb done_cb,
		       Eio_Error_Cb error_cb,
		       const void *data)
{
   Eio_File_Xattr *async;

   if (!path || !attribute || !done_cb || !error_cb)
     return NULL;

   async = malloc(sizeof (Eio_File_Xattr));
   if (!async) return NULL;

   async->op = EIO_XATTR_INT;
   async->todo.xint.xattr_int = xattr_int;

   return _eio_file_xattr_setup_set(async, path, attribute, flags, done_cb, error_cb, data);
}

/**
 * @}
 */
