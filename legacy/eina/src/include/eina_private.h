/* EINA - EFL data type library
 * Copyright (C) 2008 Carsten Haitzler, Vincent Torri, Jorge Luis Zapata Muga
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

#ifndef EINA_PRIVATE_H_
#define EINA_PRIVATE_H_

#include <stdarg.h>

#include "eina_config.h"

#include "eina_magic.h"
#include "eina_iterator.h"
#include "eina_accessor.h"

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif

#ifndef MIN
# define MIN(x, y) (((x) > (y)) ? (y) : (x))
#endif

#ifndef MAX
# define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef ABS
# define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#ifndef CLAMP
# define CLAMP(x, min, max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#endif

#define READBUFSIZ 65536

/* eina magic types */
#define EINA_MAGIC_ITERATOR 0x98761233
#define EINA_MAGIC_ACCESSOR 0x98761232

#define EINA_MAGIC_STRINGSHARE 0x98761234
#define EINA_MAGIC_STRINGSHARE_NODE 0x98761235
#define EINA_MAGIC_STRINGSHARE_HEAD 0x98761236

#define EINA_MAGIC_LIST	0x98761237
#define EINA_MAGIC_LIST_ITERATOR 0x98761238
#define EINA_MAGIC_LIST_ACCESSOR 0x98761239
#define EINA_MAGIC_LIST_ACCOUNTING 0x9876123a

#define EINA_MAGIC_ARRAY 0x9876123b
#define EINA_MAGIC_ARRAY_ITERATOR 0x9876123c
#define EINA_MAGIC_ARRAY_ACCESSOR 0x9876123d

/* undef the following, we want out version */
#undef FREE
#define FREE(ptr)				\
  do {						\
     free(ptr);					\
     ptr = NULL;				\
  } while(0);

#undef IF_FREE
#define IF_FREE(ptr)				\
  do {						\
     if (ptr) {					\
	free(ptr);				\
	ptr = NULL;				\
     }						\
  } while(0);

#undef IF_FN_DEL
#define IF_FN_DEL(_fn, ptr)			\
  do {						\
     if (ptr) {					\
	_fn(ptr);				\
	ptr = NULL;				\
     }						\
  } while(0);

#define MAGIC_FREE(ptr)					\
  do {							\
     EINA_MAGIC_SET(ptr, EINA_MAGIC_NONE);		\
     FREE(ptr);						\
  } while(0);

inline void eina_print_warning(const char *function, const char *sparam);

/* convenience macros for checking pointer parameters for non-NULL */
#undef CHECK_PARAM_POINTER_RETURN
#define CHECK_PARAM_POINTER_RETURN(sparam, param, ret) \
     if (!(param)) \
	 { \
	    eina_print_warning(__FUNCTION__, sparam); \
	    return ret; \
	 }

#undef CHECK_PARAM_POINTER
#define CHECK_PARAM_POINTER(sparam, param) \
     if (!(param)) \
	 { \
	    eina_print_warning(__FUNCTION__, sparam); \
	    return; \
	 }

/* Iterator/accessor private type */
typedef Eina_Bool (*Eina_Iterator_Next_Callback)(Eina_Iterator *it, void **data);
typedef void *(*Eina_Iterator_Get_Container_Callback)(Eina_Iterator *it);
typedef void (*Eina_Iterator_Free_Callback)(Eina_Iterator *it);

#define FUNC_ITERATOR_NEXT(Function) ((Eina_Iterator_Next_Callback)Function)
#define FUNC_ITERATOR_GET_CONTAINER(Function) ((Eina_Iterator_Get_Container_Callback)Function)
#define FUNC_ITERATOR_FREE(Function) ((Eina_Iterator_Free_Callback)Function)

typedef Eina_Bool (*Eina_Accessor_Get_At_Callback)(Eina_Accessor *it, unsigned int index, void **data);
typedef void *(*Eina_Accessor_Get_Container_Callback)(Eina_Accessor *it);
typedef void (*Eina_Accessor_Free_Callback)(Eina_Accessor *it);

#define FUNC_ACCESSOR_GET_AT(Function) ((Eina_Accessor_Get_At_Callback)Function)
#define FUNC_ACCESSOR_GET_CONTAINER(Function) ((Eina_Accessor_Get_Container_Callback)Function)
#define FUNC_ACCESSOR_FREE(Function) ((Eina_Accessor_Free_Callback)Function)

struct _Eina_Iterator
{
   EINA_MAGIC;

   Eina_Iterator_Next_Callback          next;
   Eina_Iterator_Get_Container_Callback get_container;
   Eina_Iterator_Free_Callback          free;
};

struct _Eina_Accessor
{
   EINA_MAGIC;

   Eina_Accessor_Get_At_Callback        get_at;
   Eina_Accessor_Get_Container_Callback	get_container;
   Eina_Accessor_Free_Callback          free;
};

#endif /* EINA_PRIVATE_H_ */

