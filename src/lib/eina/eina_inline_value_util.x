/*
 * Copyright (C) 2013, 2014 Mike Blumenkrantz
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


#ifndef EINA_INLINE_VALUE_UTIL_X_
#define EINA_INLINE_VALUE_UTIL_X_

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>


/**
 * @brief Get size of #Eina_Value_Type based on C type
 * @param type The type to get the size of
 * @return The size of the type
 * @since 1.12
 */
static inline size_t
eina_value_util_type_size(const Eina_Value_Type *type)
{
   if (type == EINA_VALUE_TYPE_INT) return sizeof(int32_t);
   if (type == EINA_VALUE_TYPE_UCHAR) return sizeof(unsigned char);
   if ((type == EINA_VALUE_TYPE_STRING) || (type == EINA_VALUE_TYPE_STRINGSHARE)) return sizeof(char*);
   if (type == EINA_VALUE_TYPE_TIMESTAMP) return sizeof(time_t);
   if (type == EINA_VALUE_TYPE_ARRAY) return sizeof(Eina_Value_Array);
   if (type == EINA_VALUE_TYPE_DOUBLE) return sizeof(double);
   if (type == EINA_VALUE_TYPE_STRUCT) return sizeof(Eina_Value_Struct);
   return 0;
}

/**
 * @brief Get padding of #Eina_Value_Type based on C type and base padding
 * @param type The type to get the offset of
 * @param base The existing base size
 * @return The offset of the type
 * @since 1.12
 */
static inline unsigned int
eina_value_util_type_offset(const Eina_Value_Type *type, unsigned int base)
{
   unsigned size, padding;
   size = eina_value_util_type_size(type);
   if (!(base % size))
     return base;
   padding = abs(base - size);
   return base + padding;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param i The value to use
 * @return The #Eina_Value
 * @since 1.12
 */
static inline Eina_Value *
eina_value_util_int_new(int i)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_INT);
   if (v) eina_value_set(v, i);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param d The value to use
 * @return The #Eina_Value
 * @since 1.12
 */
static inline Eina_Value *
eina_value_util_double_new(double d)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_DOUBLE);
   if (v) eina_value_set(v, d);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param b The value to use
 * @return The #Eina_Value
 * @since 1.12
 */
static inline Eina_Value *
eina_value_util_bool_new(Eina_Bool b)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_UCHAR);
   if (v) eina_value_set(v, b);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param str The value to use
 * @return The #Eina_Value
 * @since 1.12
 */
static inline Eina_Value *
eina_value_util_string_new(const char *str)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_STRING);
   if (v) eina_value_set(v, str);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param str The value to use
 * @return The #Eina_Value
 * @since 1.12
 */
static inline Eina_Value *
eina_value_util_stringshare_new(const char *str)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_STRINGSHARE);
   if (v) eina_value_set(v, str);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param t The value to use
 * @return The #Eina_Value
 * @since 1.12
 */
static inline Eina_Value *
eina_value_util_time_new(time_t t)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_TIMESTAMP);
   if (v) eina_value_set(v, t);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param timestr The value to use
 * @return The #Eina_Value
 * @since 1.12
 */
static inline Eina_Value *
eina_value_util_time_string_new(const char *timestr)
{
   Eina_Value *v;
   struct tm tm;
   time_t t;

   if (!strptime(timestr, "%Y%m%dT%H:%M:%S", &tm)) return NULL;
   t = mktime(&tm);
   v = eina_value_new(EINA_VALUE_TYPE_TIMESTAMP);
   if (v) eina_value_set(v, t);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param val The value to use
 * @return The #Eina_Value
 * @since 1.12
 */
static inline Eina_Value *
eina_value_util_dup(const Eina_Value *val)
{
   Eina_Value *v;

   v = eina_value_new(eina_value_type_get(val));
   EINA_SAFETY_ON_NULL_RETURN_VAL(v, NULL);
   eina_value_copy(val, v);
   return v;
}

/**
 * @brief Copy the stringshare in the passed #Eina_Value
 * @param val The value to copy
 * @param str The pointer to copy the stringshare to
 * @return @c EINA_TRUE on success
 * @since 1.12
 */
static inline Eina_Bool
eina_value_util_stringshare_copy(const Eina_Value *val, Eina_Stringshare **str)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(val, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   if (!eina_value_get(val, str)) return EINA_FALSE;
   eina_stringshare_ref(*str);
   return EINA_TRUE;
}

/**
 * @brief Copy the string in the passed #Eina_Value
 * @param val The value to copy
 * @param str The pointer to copy the string to
 * @return @c EINA_TRUE on success
 * @since 1.12
 */
static inline Eina_Bool
eina_value_util_string_copy(const Eina_Value *val, char **str)
{
   char *s;
   EINA_SAFETY_ON_NULL_RETURN_VAL(val, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   if (!eina_value_get(val, &s)) return EINA_FALSE;
   *str = s ? strdup(s) : NULL;
   return EINA_TRUE;
}

#endif
