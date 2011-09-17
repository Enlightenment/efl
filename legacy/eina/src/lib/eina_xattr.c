/* EINA - EFL data type library
 * Copyright (C) 2011 Cedric Bail
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

#include <sys/types.h>
#include <string.h>
#include <math.h>

#ifdef HAVE_XATTR
# include <sys/xattr.h>
#endif

#include "eina_config.h"
#include "eina_private.h"

#include "eina_safety_checks.h"
#include "eina_xattr.h"
#include "eina_convert.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

typedef struct _Eina_Xattr_Iterator Eina_Xattr_Iterator;

struct _Eina_Xattr_Iterator
{
   Eina_Iterator iterator;

   ssize_t length;
   ssize_t offset;

   char xattr[1];
};

#ifdef HAVE_XATTR
static Eina_Bool
_eina_xattr_ls_iterator_next(Eina_Xattr_Iterator *it, void **data)
{
   if (it->offset >= it->length)
     return EINA_FALSE;

   *data = it->xattr + it->offset;
   it->offset += strlen(it->xattr + it->offset) + 1;

   return EINA_TRUE;
}

static void *
_eina_xattr_ls_iterator_container(Eina_Xattr_Iterator *it __UNUSED__)
{
   return NULL;
}

static void
_eina_xattr_ls_iterator_free(Eina_Xattr_Iterator *it)
{
   EINA_MAGIC_SET(&it->iterator, 0);
   free(it);
}
#endif

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/


EAPI Eina_Iterator *
eina_xattr_ls(const char *file)
{
   Eina_Xattr_Iterator *it;
   ssize_t length;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);

#ifdef HAVE_XATTR
   length = listxattr(file, NULL, 0);
   if (length <= 0) return NULL;

   it = calloc(1, sizeof (Eina_Xattr_Iterator) + length - 1);
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->length = listxattr(file, it->xattr, length);
   if (it->length != length)
     {
        free(it);
	return NULL;
     }

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_xattr_ls_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eina_xattr_ls_iterator_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_xattr_ls_iterator_free);

   return &it->iterator;
#else
   return NULL;
#endif
}

EAPI void *
eina_xattr_get(const char *file, const char *attribute, ssize_t *size)
{
   void *ret = NULL;
   ssize_t tmp;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(attribute, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!size, NULL);

#ifdef HAVE_XATTR
   *size = getxattr(file, attribute, NULL, 0);
   /* Size should be less than 2MB (already huge in my opinion) */
   if (!(*size > 0 && *size < 2 * 1024 * 1024))
     goto on_error;

   ret = malloc(*size);
   if (!ret) return NULL;

   tmp = getxattr(file, attribute, ret, *size);
   if (tmp != *size)
     goto on_error;

   return ret;

 on_error:
   free(ret);
#endif
   *size = 0;
   return NULL;
}

EAPI Eina_Bool
eina_xattr_set(const char *file, const char *attribute, const void *data, ssize_t length, Eina_Xattr_Flags flags)
{
   int iflags;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(attribute, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!(length > 0 && length < 2 * 1024 * 1024), EINA_FALSE);

#ifdef HAVE_XATTR
   switch (flags)
     {
     case EINA_XATTR_INSERT: iflags = 0; break;
     case EINA_XATTR_REPLACE: iflags = XATTR_REPLACE; break;
     case EINA_XATTR_CREATED: iflags = XATTR_CREATE; break;
     default:
       return EINA_FALSE;
     }

   if (setxattr(file, attribute, data, length, iflags))
     return EINA_FALSE;
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
eina_xattr_string_set(const char *file, const char *attribute, const char *data, Eina_Xattr_Flags flags)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, EINA_FALSE);

   return eina_xattr_set(file, attribute, data, strlen(data) + 1, flags);
}

EAPI char *
eina_xattr_string_get(const char *file, const char *attribute)
{
   char *tmp;
   ssize_t size;

   tmp = eina_xattr_get(file, attribute, &size);
   if (!tmp) return NULL;

   if (tmp[size - 1] != '\0')
     {
        free(tmp);
        return NULL;
     }

   return tmp;
}

EAPI Eina_Bool
eina_xattr_double_set(const char *file, const char *attribute, double value, Eina_Xattr_Flags flags)
{
   char buffer[128];

   eina_convert_dtoa(value, buffer);
   return eina_xattr_string_set(file, attribute, buffer, flags);
}

EAPI Eina_Bool
eina_xattr_double_get(const char *file, const char *attribute, double *value)
{
   char *tmp;
   long long int m = 0;
   long int e = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);

   tmp = eina_xattr_string_get(file, attribute);
   if (!tmp) return EINA_FALSE;

   if (!eina_convert_atod(tmp, strlen(tmp), &m, &e))
     {
       free(tmp);
       return EINA_FALSE;
     }

   *value = ldexp((double)m, e);
   free(tmp);

   return EINA_TRUE;
}

EAPI Eina_Bool
eina_xattr_int_set(const char *file, const char *attribute, int value, Eina_Xattr_Flags flags)
{
   char buffer[10];

   eina_convert_itoa(value, buffer);
   return eina_xattr_string_set(file, attribute, buffer, flags);
}

EAPI Eina_Bool
eina_xattr_int_get(const char *file, const char *attribute, int *value)
{
   char *tmp;
   char *eos;
   Eina_Bool result;

   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);

   tmp = eina_xattr_string_get(file, attribute);
   if (!tmp) return EINA_FALSE;

   *value = (int) strtol(tmp, &eos, 10);
   result = (*eos == '\0');
   free(tmp);

   return result;
}
