/* EINA - EFL data type library
 * Copyright (C) 2012 ProFUSION embedded systems
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

#ifndef EINA_INLINE_VALUE_X_
#define EINA_INLINE_VALUE_X_

#include <string.h>
#include "eina_stringshare.h"

/* NOTE: most of value is implemented here for performance reasons */

//#define EINA_VALUE_NO_OPTIMIZE 1
#ifdef EINA_VALUE_NO_OPTIMIZE
#define EINA_VALUE_TYPE_DEFAULT(type) (0)
#else
#define EINA_VALUE_TYPE_DEFAULT(type)           \
  ((type == EINA_VALUE_TYPE_UCHAR) ||           \
   (type == EINA_VALUE_TYPE_USHORT) ||          \
   (type == EINA_VALUE_TYPE_UINT) ||            \
   (type == EINA_VALUE_TYPE_ULONG) ||           \
   (type == EINA_VALUE_TYPE_UINT64) ||          \
   (type == EINA_VALUE_TYPE_CHAR) ||            \
   (type == EINA_VALUE_TYPE_SHORT) ||           \
   (type == EINA_VALUE_TYPE_INT) ||             \
   (type == EINA_VALUE_TYPE_LONG) ||            \
   (type == EINA_VALUE_TYPE_INT64) ||           \
   (type == EINA_VALUE_TYPE_FLOAT) ||           \
   (type == EINA_VALUE_TYPE_DOUBLE) ||          \
   (type == EINA_VALUE_TYPE_STRINGSHARE) ||     \
   (type == EINA_VALUE_TYPE_STRING))
#endif

#define EINA_VALUE_TYPE_CHECK_RETURN(value)     \
  EINA_SAFETY_ON_NULL_RETURN(value);            \
  EINA_SAFETY_ON_FALSE_RETURN(eina_value_type_check(value->type))

#define EINA_VALUE_TYPE_CHECK_RETURN_VAL(value, retval)                 \
  EINA_SAFETY_ON_NULL_RETURN_VAL(value, retval);                        \
  EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(value->type), retval)

#define EINA_VALUE_TYPE_DISPATCH(type, method, no_method_err, ...)      \
  do                                                                    \
    {                                                                   \
       if (type->method)                                                \
         type->method(type, ##__VA_ARGS__);                             \
       else                                                             \
         eina_error_set(no_method_err);                                 \
    }                                                                   \
  while (0)

#define EINA_VALUE_TYPE_DISPATCH_RETURN(value, method, no_method_err, def_ret, ...) \
  do                                                                    \
    {                                                                   \
       if (type->method)                                                \
         return type->method(type, ##__VA_ARGS__);                      \
       eina_error_set(no_method_err);                                   \
       return def_ret;                                                  \
    }                                                                   \
  while (0)

static inline void *
eina_value_memory_get(const Eina_Value *value)
{
   if (value->type->value_size <= 8)
     return (void *)value->value.buf;
   return value->value.ptr;
}

static inline Eina_Bool
eina_value_setup(Eina_Value *value, const Eina_Value_Type *type)
{
   void *mem;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(type), EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(type->value_size > 0, EINA_FALSE);

   value->type = type;

   if (type->value_size <= 8)
     {
        mem = &value->value;
        memset(mem, 0, type->value_size);
     }
   else
     {
        mem = value->value.ptr = calloc(1, type->value_size);
        EINA_SAFETY_ON_NULL_RETURN_VAL(mem, EINA_FALSE);
     }

   if (EINA_VALUE_TYPE_DEFAULT(type))
     {
        eina_error_set(0);
        return EINA_TRUE;
     }

   EINA_VALUE_TYPE_DISPATCH_RETURN(type, setup,
                                   EINA_ERROR_VALUE_FAILED, EINA_FALSE, mem);
}

static inline void
eina_value_flush(Eina_Value *value)
{
   const Eina_Value_Type *type;
   void *mem;

   EINA_VALUE_TYPE_CHECK_RETURN(value);

   type = value->type;
   mem = eina_value_memory_get(value);

   if (EINA_VALUE_TYPE_DEFAULT(type))
     {
        if (type == EINA_VALUE_TYPE_STRINGSHARE)
          {
             if (value->value.ptr) eina_stringshare_del(value->value.ptr);
          }
        else if (type == EINA_VALUE_TYPE_STRING)
          {
             if (value->value.ptr) free(value->value.ptr);
          }
        else if (type->value_size > 8) free(mem);
        eina_error_set(0);
        return;
     }

   EINA_VALUE_TYPE_DISPATCH(type, flush, EINA_ERROR_VALUE_FAILED, mem);
   if (type->value_size > 8) free(mem);
   value->type = NULL;
}

static inline int
eina_value_compare(const Eina_Value *a, const Eina_Value *b)
{
   const Eina_Value_Type *type;
   void *pa, *pb;

   EINA_VALUE_TYPE_CHECK_RETURN_VAL(a, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(b, -1);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(a->type == b->type, -1);

   eina_error_set(0);
   type = a->type;
   pa = eina_value_memory_get(a);
   pb = eina_value_memory_get(b);

#ifndef EINA_VALUE_NO_OPTIMIZE
   if (type == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char *ta = pa, *tb = pb;
        if (*ta < *tb)
          return -1;
        else if (*ta > *tb)
          return 1;
        return 0;
     }
   else if (type == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short *ta = pa, *tb = pb;
        if (*ta < *tb)
          return -1;
        else if (*ta > *tb)
          return 1;
        return 0;
     }
   else if (type == EINA_VALUE_TYPE_UINT)
     {
        unsigned int *ta = pa, *tb = pb;
        if (*ta < *tb)
          return -1;
        else if (*ta > *tb)
          return 1;
        return 0;
     }
   else if (type == EINA_VALUE_TYPE_ULONG)
     {
        unsigned long *ta = pa, *tb = pb;
        if (*ta < *tb)
          return -1;
        else if (*ta > *tb)
          return 1;
        return 0;
     }
   else if (type == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t *ta = pa, *tb = pb;
        if (*ta < *tb)
          return -1;
        else if (*ta > *tb)
          return 1;
        return 0;
     }
   else if (type == EINA_VALUE_TYPE_CHAR)
     {
        char *ta = pa, *tb = pb;
        if (*ta < *tb)
          return -1;
        else if (*ta > *tb)
          return 1;
        return 0;
     }
   else if (type == EINA_VALUE_TYPE_SHORT)
     {
        short *ta = pa, *tb = pb;
        if (*ta < *tb)
          return -1;
        else if (*ta > *tb)
          return 1;
        return 0;
     }
   else if (type == EINA_VALUE_TYPE_INT)
     {
        int *ta = pa, *tb = pb;
        if (*ta < *tb)
          return -1;
        else if (*ta > *tb)
          return 1;
        return 0;
     }
   else if (type == EINA_VALUE_TYPE_LONG)
     {
        long *ta = pa, *tb = pb;
        if (*ta < *tb)
          return -1;
        else if (*ta > *tb)
          return 1;
        return 0;
     }
   else if (type == EINA_VALUE_TYPE_INT64)
     {
        int64_t *ta = pa, *tb = pb;
        if (*ta < *tb)
          return -1;
        else if (*ta > *tb)
          return 1;
        return 0;
     }
   else if (type == EINA_VALUE_TYPE_FLOAT)
     {
        float *ta = pa, *tb = pb;
        if (*ta < *tb)
          return -1;
        else if (*ta > *tb)
          return 1;
        return 0;
     }
   else if (type == EINA_VALUE_TYPE_DOUBLE)
     {
        double *ta = pa, *tb = pb;
        if (*ta < *tb)
          return -1;
        else if (*ta > *tb)
          return 1;
        return 0;
     }
   else if (type == EINA_VALUE_TYPE_STRINGSHARE ||
            type == EINA_VALUE_TYPE_STRING)
     {
        const char *sa = *(const char **)pa;
        const char *sb = *(const char **)pb;
        if (sa == sb)
          return 0;
        if (sa == NULL)
          return -1;
        if (sb == NULL)
          return 1;
        return strcmp(sa, sb);
     }
#endif

   EINA_VALUE_TYPE_DISPATCH_RETURN(type, compare, EINA_ERROR_VALUE_FAILED,
                                   EINA_FALSE, pa, pb);
}

static inline Eina_Bool
eina_value_set(Eina_Value *value, ...)
{
   va_list args;
   Eina_Bool ret;
   va_start(args, value);
   ret = eina_value_vset(value, args);
   va_end(args);
   return ret;
}

static inline Eina_Bool
eina_value_get(const Eina_Value *value, ...)
{
   va_list args;
   Eina_Bool ret;
   va_start(args, value);
   ret = eina_value_vget(value, args);
   va_end(args);
   return ret;
}

static inline Eina_Bool
eina_value_vset(Eina_Value *value, va_list args)
{
   const Eina_Value_Type *type;
   void *mem;

   EINA_VALUE_TYPE_CHECK_RETURN_VAL(value, EINA_FALSE);

   type = value->type;
   mem = eina_value_memory_get(value);
   eina_error_set(0);
#ifndef EINA_VALUE_NO_OPTIMIZE
   if (type == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char *tmem = mem;
        *tmem = va_arg(args, unsigned int); /* promoted by va_arg */
        return EINA_TRUE;
     }
   else if (type == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short *tmem = mem;
        *tmem = va_arg(args, unsigned int); /* promoted by va_arg */
        return EINA_TRUE;
     }
   else if (type == EINA_VALUE_TYPE_UINT)
     {
        unsigned int *tmem = mem;
        *tmem = va_arg(args, unsigned int);
        return EINA_TRUE;
     }
   else if (type == EINA_VALUE_TYPE_ULONG)
     {
        unsigned long *tmem = mem;
        *tmem = va_arg(args, unsigned long);
        return EINA_TRUE;
     }
   else if (type == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t *tmem = mem;
        *tmem = va_arg(args, uint64_t);
        return EINA_TRUE;
     }
   else if (type == EINA_VALUE_TYPE_CHAR)
     {
        char *tmem = mem;
        *tmem = va_arg(args, int); /* promoted by va_arg */
        return EINA_TRUE;
     }
   else if (type == EINA_VALUE_TYPE_SHORT)
     {
        short *tmem = mem;
        *tmem = va_arg(args, int); /* promoted by va_arg */
        return EINA_TRUE;
     }
   else if (type == EINA_VALUE_TYPE_INT)
     {
        int *tmem = mem;
        *tmem = va_arg(args, int);
        return EINA_TRUE;
     }
   else if (type == EINA_VALUE_TYPE_LONG)
     {
        long *tmem = mem;
        *tmem = va_arg(args, long);
        return EINA_TRUE;
     }
   else if (type == EINA_VALUE_TYPE_INT64)
     {
        int64_t *tmem = mem;
        *tmem = va_arg(args, int64_t);
        return EINA_TRUE;
     }
   else if (type == EINA_VALUE_TYPE_FLOAT)
     {
        float *tmem = mem;
        *tmem = va_arg(args, double); /* promoted by va_arg */
        return EINA_TRUE;
     }
   else if (type == EINA_VALUE_TYPE_DOUBLE)
     {
        double *tmem = mem;
        *tmem = va_arg(args, double);
        return EINA_TRUE;
     }
   else if (type == EINA_VALUE_TYPE_STRINGSHARE)
     {
        const char *str = va_arg(args, const char *);
        return eina_stringshare_replace((const char **)&value->value.ptr, str);
     }
   else if (type == EINA_VALUE_TYPE_STRING)
     {
        const char *str = va_arg(args, const char *);
        free(value->value.ptr);
        if (!str)
          value->value.ptr = NULL;
        else
          {
             value->value.ptr = strdup(str);
             if (!value->value.ptr)
               {
                  eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
                  return EINA_FALSE;
               }
          }
        return EINA_TRUE;
     }
#endif

   EINA_VALUE_TYPE_DISPATCH_RETURN(value, vset, EINA_ERROR_VALUE_FAILED,
                                   EINA_FALSE, mem, args);
}

static inline Eina_Bool
eina_value_vget(const Eina_Value *value, va_list args)
{
   const Eina_Value_Type *type;
   const void *mem;
   void *ptr;

   EINA_VALUE_TYPE_CHECK_RETURN_VAL(value, EINA_FALSE);

   type = value->type;
   mem = eina_value_memory_get(value);
   ptr = va_arg(args, void *);
   eina_error_set(0);
   if (EINA_VALUE_TYPE_DEFAULT(type))
     {
        memcpy(ptr, mem, type->value_size);
        return EINA_TRUE;
     }

   EINA_VALUE_TYPE_DISPATCH_RETURN(value, pget, EINA_ERROR_VALUE_FAILED,
                                   EINA_FALSE, mem, ptr);
}

static inline Eina_Bool
eina_value_pset(Eina_Value *value, const void *ptr)
{
   const Eina_Value_Type *type;
   void *mem;

   EINA_VALUE_TYPE_CHECK_RETURN_VAL(value, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ptr, EINA_FALSE);

   type = value->type;
   mem = eina_value_memory_get(value);
   eina_error_set(0);

   if (EINA_VALUE_TYPE_DEFAULT(type))
     {
        if (type == EINA_VALUE_TYPE_STRINGSHARE)
          {
             const char * const *str = ptr;
             return eina_stringshare_replace((const char **)&value->value.ptr,
                                             *str);
          }
        else if (type == EINA_VALUE_TYPE_STRING)
          {
             const char * const *str = ptr;
             free(value->value.ptr);
             if (!*str)
               value->value.ptr = NULL;
             else
               {
                  value->value.ptr = strdup(*str);
                  if (!value->value.ptr)
                    {
                       eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
                       return EINA_FALSE;
                    }
               }
             return EINA_TRUE;
          }
        else
          memcpy(mem, ptr, type->value_size);
        return EINA_TRUE;
     }

   EINA_VALUE_TYPE_DISPATCH_RETURN(value, pset, EINA_ERROR_VALUE_FAILED,
                                   EINA_FALSE, mem, ptr);
}

static inline Eina_Bool
eina_value_pget(const Eina_Value *value, void *ptr)
{
   const Eina_Value_Type *type;
   const void *mem;

   EINA_VALUE_TYPE_CHECK_RETURN_VAL(value, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ptr, EINA_FALSE);

   type = value->type;
   mem = eina_value_memory_get(value);
   eina_error_set(0);
   if (EINA_VALUE_TYPE_DEFAULT(type))
     {
        memcpy(ptr, mem, type->value_size);
        return EINA_TRUE;
     }

   EINA_VALUE_TYPE_DISPATCH_RETURN(value, pget, EINA_ERROR_VALUE_FAILED,
                                   EINA_FALSE, mem, ptr);
}

static inline const Eina_Value_Type *
eina_value_type_get(const Eina_Value *value)
{
   EINA_VALUE_TYPE_CHECK_RETURN_VAL(value, NULL);
   return value->type;
}

#define EINA_VALUE_TYPE_ARRAY_CHECK_RETURN_VAL(value, retval)   \
  EINA_SAFETY_ON_NULL_RETURN_VAL(value, retval);                \
  EINA_SAFETY_ON_FALSE_RETURN_VAL(value->type == EINA_VALUE_TYPE_ARRAY, retval)

static inline Eina_Bool
eina_value_array_setup(Eina_Value *value, const Eina_Value_Type *subtype, unsigned int step)
{
   Eina_Value_Array desc = {subtype, step, NULL};
   if (!eina_value_setup(value, EINA_VALUE_TYPE_ARRAY))
     return EINA_FALSE;
   if (!eina_value_pset(value, &desc))
     {
        eina_value_flush(value);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static inline unsigned int
eina_value_array_count(const Eina_Value *value)
{
   Eina_Value_Array desc;
   EINA_VALUE_TYPE_ARRAY_CHECK_RETURN_VAL(value, 0);
   if (!eina_value_pget(value, &desc))
     return 0;
   return eina_inarray_count(desc.array);
}

static inline Eina_Bool
eina_value_array_remove(Eina_Value *value, unsigned int position)
{
   Eina_Value_Array desc;
   void *mem;

   EINA_VALUE_TYPE_ARRAY_CHECK_RETURN_VAL(value, 0);
   if (!eina_value_pget(value, &desc))
     return EINA_FALSE;

   mem = eina_inarray_nth(desc.array, position);
   if (!mem)
     return EINA_FALSE;

   eina_value_type_flush(desc.subtype, mem);
   return eina_inarray_remove_at(desc.array, position);
}

static inline Eina_Bool
eina_value_array_vset(Eina_Value *value, unsigned int position, va_list args)
{
   Eina_Value_Array desc;
   void *mem;

   EINA_VALUE_TYPE_ARRAY_CHECK_RETURN_VAL(value, 0);
   if (!eina_value_pget(value, &desc))
     return EINA_FALSE;

   mem = eina_inarray_nth(desc.array, position);
   if (!mem)
     return EINA_FALSE;

   eina_value_type_flush(desc.subtype, mem);

   if (!eina_value_type_setup(desc.subtype, mem)) goto error_setup;
   if (!eina_value_type_vset(desc.subtype, mem, args)) goto error_set;
   return EINA_TRUE;

 error_set:
   eina_value_type_flush(desc.subtype, mem);
 error_setup:
   return EINA_FALSE;
}

static inline Eina_Bool
eina_value_array_vget(const Eina_Value *value, unsigned int position, va_list args)
{
   Eina_Value_Array desc;
   const void *mem;
   void *ptr;
   Eina_Bool ret;

   EINA_VALUE_TYPE_ARRAY_CHECK_RETURN_VAL(value, 0);
   if (!eina_value_pget(value, &desc))
     return EINA_FALSE;

   mem = eina_inarray_nth(desc.array, position);
   if (!mem)
     return EINA_FALSE;

   ptr = va_arg(args, void *);
   ret = eina_value_type_pget(desc.subtype, mem, ptr);
   return ret;
}

static inline Eina_Bool
eina_value_array_vinsert(Eina_Value *value, unsigned int position, va_list args)
{
   Eina_Value_Array desc;
   void *mem, *placeholder;

   EINA_VALUE_TYPE_ARRAY_CHECK_RETURN_VAL(value, 0);
   if (!eina_value_pget(value, &desc))
     return EINA_FALSE;

   placeholder = alloca(desc.subtype->value_size);
   memset(placeholder, 0, desc.subtype->value_size);
   if (!eina_inarray_insert_at(desc.array, position, placeholder))
     return EINA_FALSE;

   mem = eina_inarray_nth(desc.array, position);
   if (!mem)
     return EINA_FALSE;

   if (!eina_value_type_setup(desc.subtype, mem)) goto error_setup;
   if (!eina_value_type_vset(desc.subtype, mem, args)) goto error_set;
   return EINA_TRUE;

 error_set:
   eina_value_type_flush(desc.subtype, mem);
 error_setup:
   eina_inarray_remove_at(desc.array, position);
   return EINA_FALSE;
}

static inline Eina_Bool
eina_value_array_vappend(Eina_Value *value, va_list args)
{
   Eina_Value_Array desc;
   void *mem, *placeholder;
   int position;

   EINA_VALUE_TYPE_ARRAY_CHECK_RETURN_VAL(value, 0);
   if (!eina_value_pget(value, &desc))
     return EINA_FALSE;

   placeholder = alloca(desc.subtype->value_size);
   memset(placeholder, 0, desc.subtype->value_size);
   position = eina_inarray_append(desc.array, placeholder);
   if (position < 0)
     return EINA_FALSE;

   mem = eina_inarray_nth(desc.array, position);
   if (!mem)
     return EINA_FALSE;

   if (!eina_value_type_setup(desc.subtype, mem)) goto error_setup;
   if (!eina_value_type_vset(desc.subtype, mem, args)) goto error_set;
   return EINA_TRUE;

 error_set:
   eina_value_type_flush(desc.subtype, mem);
 error_setup:
   eina_inarray_remove_at(desc.array, position);
   return EINA_FALSE;
}

static inline Eina_Bool
eina_value_array_set(Eina_Value *value, unsigned int position, ...)
{
   va_list args;
   Eina_Bool ret;
   va_start(args, position);
   ret = eina_value_array_vset(value, position, args);
   va_end(args);
   return ret;
}

static inline Eina_Bool
eina_value_array_get(const Eina_Value *value, unsigned int position, ...)
{
   va_list args;
   Eina_Bool ret;
   va_start(args, position);
   ret = eina_value_array_vget(value, position, args);
   va_end(args);
   return ret;
}

static inline Eina_Bool
eina_value_array_insert(Eina_Value *value, unsigned int position, ...)
{
   va_list args;
   Eina_Bool ret;
   va_start(args, position);
   ret = eina_value_array_vinsert(value, position, args);
   va_end(args);
   return ret;
}

static inline Eina_Bool eina_value_array_append(Eina_Value *value, ...)
{
   va_list args;
   Eina_Bool ret;
   va_start(args, value);
   ret = eina_value_array_vappend(value, args);
   va_end(args);
   return ret;
}

static inline Eina_Bool
eina_value_array_pset(Eina_Value *value, unsigned int position, const void *ptr)
{
   Eina_Value_Array desc;
   void *mem;

   EINA_VALUE_TYPE_ARRAY_CHECK_RETURN_VAL(value, 0);
   if (!eina_value_pget(value, &desc))
     return EINA_FALSE;

   mem = eina_inarray_nth(desc.array, position);
   if (!mem)
     return EINA_FALSE;

   eina_value_type_flush(desc.subtype, mem);

   if (!eina_value_type_setup(desc.subtype, mem)) goto error_setup;
   if (!eina_value_type_pset(desc.subtype, mem, ptr)) goto error_set;
   return EINA_TRUE;

 error_set:
   eina_value_type_flush(desc.subtype, mem);
 error_setup:
   return EINA_FALSE;
}

static inline Eina_Bool
eina_value_array_pget(const Eina_Value *value, unsigned int position, void *ptr)
{
   Eina_Value_Array desc;
   const void *mem;
   Eina_Bool ret;

   EINA_VALUE_TYPE_ARRAY_CHECK_RETURN_VAL(value, 0);
   if (!eina_value_pget(value, &desc))
     return EINA_FALSE;

   mem = eina_inarray_nth(desc.array, position);
   if (!mem)
     return EINA_FALSE;

   ret = eina_value_type_pget(desc.subtype, mem, ptr);
   return ret;
}

static inline Eina_Bool
eina_value_array_pinsert(Eina_Value *value, unsigned int position, const void *ptr)
{
   Eina_Value_Array desc;
   void *mem, *placeholder;

   EINA_VALUE_TYPE_ARRAY_CHECK_RETURN_VAL(value, 0);
   if (!eina_value_pget(value, &desc))
     return EINA_FALSE;

   placeholder = alloca(desc.subtype->value_size);
   memset(placeholder, 0, desc.subtype->value_size);
   if (!eina_inarray_insert_at(desc.array, position, placeholder))
     return EINA_FALSE;

   mem = eina_inarray_nth(desc.array, position);
   if (!mem)
     return EINA_FALSE;

   if (!eina_value_type_setup(desc.subtype, mem)) goto error_setup;
   if (!eina_value_type_pset(desc.subtype, mem, ptr)) goto error_set;
   return EINA_TRUE;

 error_set:
   eina_value_type_flush(desc.subtype, mem);
 error_setup:
   eina_inarray_remove_at(desc.array, position);
   return EINA_FALSE;
}

static inline Eina_Bool
eina_value_array_pappend(Eina_Value *value, const void *ptr)
{
   Eina_Value_Array desc;
   void *mem, *placeholder;
   int position;

   EINA_VALUE_TYPE_ARRAY_CHECK_RETURN_VAL(value, 0);
   if (!eina_value_pget(value, &desc))
     return EINA_FALSE;

   placeholder = alloca(desc.subtype->value_size);
   memset(placeholder, 0, desc.subtype->value_size);
   position = eina_inarray_append(desc.array, placeholder);
   if (position < 0)
     return EINA_FALSE;

   mem = eina_inarray_nth(desc.array, position);
   if (!mem)
     return EINA_FALSE;

   if (!eina_value_type_setup(desc.subtype, mem)) goto error_setup;
   if (!eina_value_type_pset(desc.subtype, mem, ptr)) goto error_set;
   return EINA_TRUE;

 error_set:
   eina_value_type_flush(desc.subtype, mem);
 error_setup:
   eina_inarray_remove_at(desc.array, position);
   return EINA_FALSE;
}

#undef EINA_VALUE_TYPE_ARRAY_CHECK_RETURN_VAL


static inline Eina_Bool
eina_value_type_setup(const Eina_Value_Type *type, void *mem)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(type), EINA_FALSE);
   if (!type->setup)
     {
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }
   return type->setup(type, mem);
}

static inline Eina_Bool
eina_value_type_flush(const Eina_Value_Type *type, void *mem)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(type), EINA_FALSE);
   if (!type->flush)
     {
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }
   return type->flush(type, mem);
}

static inline Eina_Bool
eina_value_type_copy(const Eina_Value_Type *type, const void *src, void *dst)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(type), EINA_FALSE);
   if (!type->copy)
     {
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }
   return type->copy(type, src, dst);
}

static inline int
eina_value_type_compare(const Eina_Value_Type *type, const void *a, void *b)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(type), EINA_FALSE);
   if (!type->compare)
     {
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }
   return type->compare(type, a, b);
}

static inline Eina_Bool
eina_value_type_convert_to(const Eina_Value_Type *type, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(type), EINA_FALSE);
   if (!type->convert_to)
     {
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }
   return type->convert_to(type, convert, type_mem, convert_mem);
}

static inline Eina_Bool
eina_value_type_convert_from(const Eina_Value_Type *type, const Eina_Value_Type *convert, void *type_mem, const void *convert_mem)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(type), EINA_FALSE);
   if (!type->convert_from)
     {
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }
   return type->convert_from(type, convert, type_mem, convert_mem);
}

static inline Eina_Bool
eina_value_type_vset(const Eina_Value_Type *type, void *mem, va_list args)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(type), EINA_FALSE);
   if (!type->vset)
     {
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }
   return type->vset(type, mem, args);
}

static inline Eina_Bool
eina_value_type_pset(const Eina_Value_Type *type, void *mem, const void *ptr)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(type), EINA_FALSE);
   if (!type->pset)
     {
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }
   return type->pset(type, mem, ptr);
}

static inline Eina_Bool
eina_value_type_pget(const Eina_Value_Type *type, const void *mem, void *ptr)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(type), EINA_FALSE);
   if (!type->pget)
     {
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }
   return type->pget(type, mem, ptr);
}

#undef EINA_VALUE_TYPE_DEFAULT
#undef EINA_VALUE_TYPE_CHECK_RETURN
#undef EINA_VALUE_TYPE_CHECK_RETURN_VAL
#undef EINA_VALUE_TYPE_DISPATCH
#undef EINA_VALUE_TYPE_DISPATCH_RETURN
#endif
