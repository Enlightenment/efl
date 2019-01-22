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

/**
 * @addtogroup Eina_Value_Value_group
 * @{
 */

#ifndef EINA_INLINE_VALUE_UTIL_X_
#define EINA_INLINE_VALUE_UTIL_X_

#include <inttypes.h>
#include <math.h>
#include <time.h>
#include <string.h>

/* original names were too long, _util was removed.
 * since they were static inline, a macro is enough
 */

/**
 * Legacy compat: calls eina_value_type_size()
 * @deprecated
 * @since 1.12
 */
#define eina_value_util_type_size eina_value_type_size
/**
 * Legacy compat: calls eina_value_type_offset()
 * @deprecated
 * @since 1.12
 */
#define eina_value_util_type_offset eina_value_type_offset
/**
 * Legacy compat: calls eina_value_int_new()
 * @deprecated
 * @since 1.12
 */
#define eina_value_util_int_new eina_value_int_new
/**
 * Legacy compat: calls eina_value_double_new()
 * @deprecated
 * @since 1.12
 */
#define eina_value_util_double_new eina_value_double_new
/**
 * Legacy compat: calls eina_value_bool_new()
 * @deprecated
 * @since 1.12
 */
#define eina_value_util_bool_new eina_value_bool_new
/**
 * Legacy compat: calls eina_value_string_new()
 * @deprecated
 * @since 1.12
 */
#define eina_value_util_string_new eina_value_string_new
/**
 * Legacy compat: calls eina_value_stringshare_new()
 * @deprecated
 * @since 1.12
 */
#define eina_value_util_stringshare_new eina_value_stringshare_new
/**
 * Legacy compat: calls eina_value_time_new()
 * @deprecated
 * @since 1.12
 */
#define eina_value_util_time_new eina_value_time_new
/**
 * Legacy compat: calls eina_value_dup()
 * @deprecated
 * @since 1.12
 */
#define eina_value_util_dup eina_value_dup
/**
 * Legacy compat: calls eina_value_stringshare_copy()
 * @deprecated
 * @since 1.12
 */
#define eina_value_util_stringshare_copy eina_value_stringshare_copy
/**
 * Legacy compat: calls eina_value_string_copy()
 * @deprecated
 * @since 1.12
 */
#define eina_value_util_string_copy eina_value_string_copy

/**
 * @brief Get size of #Eina_Value_Type based on C type
 * @param type The type to get the size of
 * @return The size of the type
 * @since 1.21
 */
static inline size_t
eina_value_type_size(const Eina_Value_Type *type)
{
   if (type) return type->value_size;
   return 0;
}

/**
 * @brief Get padding of #Eina_Value_Type based on C type and base padding
 * @param type The type to get the offset of
 * @param base The existing base size
 * @return The offset of the type
 * @since 1.21
 */
static inline unsigned int
eina_value_type_offset(const Eina_Value_Type *type, unsigned int base)
{
   unsigned size, padding;
   size = eina_value_type_size(type);
   if (!(base % size))
     return base;
   padding = ( (base > size) ? (base - size) : (size - base));
   return base + padding;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param c The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_uchar_new(unsigned char c)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_UCHAR);
   if (v) eina_value_set(v, c);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param s The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_ushort_new(unsigned short s)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_USHORT);
   if (v) eina_value_set(v, s);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param i The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_uint_new(unsigned int i)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_UINT);
   if (v) eina_value_set(v, i);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param l The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_ulong_new(unsigned long l)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_ULONG);
   if (v) eina_value_set(v, l);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param i The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_uint64_new(uint64_t i)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_UINT64);
   if (v) eina_value_set(v, i);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param c The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_char_new(char c)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_CHAR);
   if (v) eina_value_set(v, c);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param s The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_short_new(short s)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_SHORT);
   if (v) eina_value_set(v, s);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param i The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_int_new(int i)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_INT);
   if (v) eina_value_set(v, i);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param l The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_long_new(long l)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_LONG);
   if (v) eina_value_set(v, l);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param i The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_int64_new(int64_t i)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_INT64);
   if (v) eina_value_set(v, i);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param f The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_float_new(float f)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_FLOAT);
   if (v) eina_value_set(v, f);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param d The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_double_new(double d)
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
 * @since 1.21
 */
static inline Eina_Value *
eina_value_bool_new(Eina_Bool b)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_BOOL);
   if (v) eina_value_set(v, b);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param str The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_string_new(const char *str)
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
 * @since 1.21
 */
static inline Eina_Value *
eina_value_stringshare_new(const char *str)
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
 * @since 1.21
 */
static inline Eina_Value *
eina_value_time_new(time_t t)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_TIMESTAMP);
   if (v) eina_value_set(v, t);
   return v;
}

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param err The error code
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_error_new(Eina_Error err)
{
   Eina_Value *v;

   v = eina_value_new(EINA_VALUE_TYPE_ERROR);
   if (v) eina_value_set(v, err);
   return v;
}

/**
 * @brief Initialize #Eina_Value containing the passed parameter
 * @param c The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_uchar_init(unsigned char c)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_UCHAR))
     eina_value_set(&v, c);
   return v;
}

/**
 * @brief Initialize #Eina_Value containing the passed parameter
 * @param s The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_ushort_init(unsigned short s)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_USHORT))
     eina_value_set(&v, s);
   return v;
}

/**
 * @brief Initialize #Eina_Value containing the passed parameter
 * @param i The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_uint_init(unsigned int i)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_UINT))
     eina_value_set(&v, i);
   return v;
}

/**
 * @brief Create a new #EinaInitializeining the passed parameter
 * @param l The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_ulong_init(unsigned long l)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_ULONG))
     eina_value_set(&v, l);
   return v;
}

/**
 * @brief Initialize #Eina_Value containing the passed parameter
 * @param i The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_uint64_init(uint64_t i)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_UINT64))
     eina_value_set(&v, i);
   return v;
}

/**
 * @brief Create a new #EinaInitializeining the passed parameter
 * @param c The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_char_init(char c)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_CHAR))
     eina_value_set(&v, c);
   return v;
}

/**
 * @brief Initialize #Eina_Value containing the passed parameter
 * @param s The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_short_init(short s)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_SHORT))
     eina_value_set(&v, s);
   return v;
}

/**
 * @brief Create a new #EinaInitializeining the passed parameter
 * @param i The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_int_init(int i)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_INT))
     eina_value_set(&v, i);
   return v;
}

/**
 * @brief Initialize #Eina_Value containing the passed parameter
 * @param l The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_long_init(long l)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_LONG))
     eina_value_set(&v, l);
   return v;
}

/**
 * @brief Create a new #EinaInitializeining the passed parameter
 * @param i The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_int64_init(int64_t i)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_INT64))
     eina_value_set(&v, i);
   return v;
}

/**
 * @brief Initialize #Eina_Value containing the passed parameter
 * @param f The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_float_init(float f)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_FLOAT))
     eina_value_set(&v, f);
   return v;
}

/**
 * @brief Create a new #EinaInitializeining the passed parameter
 * @param d The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_double_init(double d)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_DOUBLE))
     eina_value_set(&v, d);
   return v;
}

/**
 * @brief Initialize #Eina_Value containing the passed parameter
 * @param b The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_bool_init(Eina_Bool b)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_BOOL))
     eina_value_set(&v, b);
   return v;
}

/**
 * @brief Create a new #EinaInitializeining the passed parameter
 * @param str The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_string_init(const char *str)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_STRING))
     eina_value_set(&v, str);
   return v;
}

/**
 * @brief Initialize #Eina_Value containing the passed parameter
 * @param str The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_stringshare_init(const char *str)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_STRINGSHARE))
     eina_value_set(&v, str);
   return v;
}

/**
 * @brief Create a new #EinaInitializeining the passed parameter
 * @param t The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_time_init(time_t t)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_TIMESTAMP))
     eina_value_set(&v, t);
   return v;
}

/**
 * @brief Initialize #Eina_Value containing the passed parameter
 * @param err The error code
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_error_init(Eina_Error err)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (eina_value_setup(&v, EINA_VALUE_TYPE_ERROR))
     eina_value_set(&v, err);
   return v;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param c Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_uchar_get(const Eina_Value *v, unsigned char *c)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(c, EINA_FALSE);
   *c = 0;
   if (v && v->type == EINA_VALUE_TYPE_UCHAR)
     return eina_value_get(v, c);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param s Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_ushort_get(const Eina_Value *v, unsigned short *s)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(s, EINA_FALSE);
   *s = 0;
   if (v && v->type == EINA_VALUE_TYPE_USHORT)
     return eina_value_get(v, s);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param i Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_uint_get(const Eina_Value *v, unsigned int *i)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(i, EINA_FALSE);
   *i = 0;
   if (v && v->type == EINA_VALUE_TYPE_UINT)
     return eina_value_get(v, i);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param l Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_ulong_get(const Eina_Value *v, unsigned long *l)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(l, EINA_FALSE);
   *l = 0;
   if (v && v->type == EINA_VALUE_TYPE_ULONG)
     return eina_value_get(v, l);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param i Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_uint64_get(const Eina_Value *v, uint64_t *i)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(i, EINA_FALSE);
   *i = 0;
   if (v && v->type == EINA_VALUE_TYPE_UINT64)
     return eina_value_get(v, i);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param c Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_char_get(const Eina_Value *v, char *c)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(c, EINA_FALSE);
   *c = 0;
   if (v && v->type == EINA_VALUE_TYPE_CHAR)
     return eina_value_get(v, c);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param s Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_short_get(const Eina_Value *v, short *s)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(s, EINA_FALSE);
   *s = 0;
   if (v && v->type == EINA_VALUE_TYPE_SHORT)
     return eina_value_get(v, s);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param i Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_int_get(const Eina_Value *v, int *i)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(i, EINA_FALSE);
   *i = 0;
   if (v && v->type == EINA_VALUE_TYPE_INT)
     return eina_value_get(v, i);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param l Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_long_get(const Eina_Value *v, long *l)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(l, EINA_FALSE);
   *l = 0;
   if (v && v->type == EINA_VALUE_TYPE_LONG)
     return eina_value_get(v, l);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param i Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_int64_get(const Eina_Value *v, int64_t *i)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(i, EINA_FALSE);
   *i = 0;
   if (v && v->type == EINA_VALUE_TYPE_INT64)
     return eina_value_get(v, i);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param f Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_float_get(const Eina_Value *v, float *f)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(f, EINA_FALSE);
   *f = 0.0f;
   if (v && v->type == EINA_VALUE_TYPE_FLOAT)
     return eina_value_get(v, f);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param d Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_double_get(const Eina_Value *v, double *d)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(d, EINA_FALSE);
   *d = 0.0;
   if (v && v->type == EINA_VALUE_TYPE_DOUBLE)
     return eina_value_get(v, d);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param b Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_bool_get(const Eina_Value *v, Eina_Bool *b)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(b, EINA_FALSE);
   *b = EINA_FALSE;
   if (v && v->type == EINA_VALUE_TYPE_BOOL)
     return eina_value_get(v, b);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param str Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_string_get(const Eina_Value *v, const char **str)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   *str = NULL;
   if (v && v->type == EINA_VALUE_TYPE_STRING)
     return eina_value_get(v, str);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param str Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_stringshare_get(const Eina_Value *v, const char **str)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   *str = NULL;
   if (v && v->type == EINA_VALUE_TYPE_STRINGSHARE)
     return eina_value_get(v, str);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param t Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_time_get(const Eina_Value *v, time_t *t)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(t, EINA_FALSE);
   *t = 0;
   if (v && v->type == EINA_VALUE_TYPE_TIMESTAMP)
     return eina_value_get(v, t);
   return EINA_FALSE;
}

/**
 * @brief Check value type and get contents.
 * @param v The value to check type and get contents.
 * @param err Where to store the value contents.
 * @return #EINA_TRUE if type matches and fetched contents,
 * #EINA_FALSE on different type or failures.
 * @since 1.21
 */
static inline Eina_Bool
eina_value_error_get(const Eina_Value *v, Eina_Error *err)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(err, EINA_FALSE);
   *err = 0;
   if (v && v->type == EINA_VALUE_TYPE_ERROR)
     return eina_value_get(v, err);
   return EINA_FALSE;
}

/// @brief For internal use only.
/// @hideinitializer
#define EINA_VALUE_CONVERT(Compress_Type, Uppercase_Compress_Type, Type) \
/**
 @brief Check value type and convert contents.
 @param v The value to check type and convert contents.
 @param c Where to store the value contents.
 @return #EINA_TRUE if type matches and fetched contents,
 #EINA_FALSE on different type or failures.
 @since 1.22
 */                                                                     \
static inline Eina_Bool                                                 \
eina_value_##Compress_Type##_convert(const Eina_Value *v, Type *c)      \
{                                                                       \
   Eina_Value dst = EINA_VALUE_EMPTY;                                   \
   Eina_Bool r = EINA_FALSE;                                            \
                                                                        \
   EINA_SAFETY_ON_NULL_RETURN_VAL(c, EINA_FALSE);                       \
   EINA_SAFETY_ON_NULL_RETURN_VAL(v, EINA_FALSE);                       \
                                                                        \
   /* Try no conversion first */                                        \
   if (eina_value_##Compress_Type##_get(v, c)) return EINA_TRUE;        \
                                                                        \
   if (!eina_value_setup(&dst, EINA_VALUE_TYPE_##Uppercase_Compress_Type)) return EINA_FALSE; \
   if (!eina_value_convert(v, &dst)) goto on_error;                     \
   if (!eina_value_##Compress_Type##_get(&dst, c)) goto on_error;       \
   r = EINA_TRUE;                                                       \
                                                                        \
 on_error:                                                              \
   eina_value_flush(&dst);                                              \
   return r;                                                            \
}

EINA_VALUE_CONVERT(uchar, UCHAR, unsigned char);
EINA_VALUE_CONVERT(ushort, USHORT, unsigned short);
EINA_VALUE_CONVERT(uint, UINT, unsigned int);
EINA_VALUE_CONVERT(ulong, ULONG, unsigned long);
EINA_VALUE_CONVERT(uint64, UINT64, uint64_t);
EINA_VALUE_CONVERT(char, CHAR, char);
EINA_VALUE_CONVERT(short, SHORT, short);
EINA_VALUE_CONVERT(int, INT, int);
EINA_VALUE_CONVERT(long, LONG, long);
EINA_VALUE_CONVERT(int64, INT64, int64_t);
EINA_VALUE_CONVERT(float, FLOAT, float);
EINA_VALUE_CONVERT(double, DOUBLE, double);
EINA_VALUE_CONVERT(bool, BOOL, Eina_Bool);
EINA_VALUE_CONVERT(string, STRING, const char *);
EINA_VALUE_CONVERT(stringshare, STRINGSHARE, const char *);
EINA_VALUE_CONVERT(time, TIMESTAMP, time_t);
EINA_VALUE_CONVERT(error, ERROR, Eina_Error);

/**
 * @brief Create a new #Eina_Value containing the passed parameter
 * @param val The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value *
eina_value_dup(const Eina_Value *val)
{
   Eina_Value *v;

   v = eina_value_new(eina_value_type_get(val));
   EINA_SAFETY_ON_NULL_RETURN_VAL(v, NULL);
   if (!eina_value_copy(val, v))
     {
        eina_value_setup(v, EINA_VALUE_TYPE_ERROR);
        eina_value_set(v, EINA_ERROR_VALUE_FAILED);
     }
   return v;
}

/**
 * @brief Return a reference to #Eina_Value containing a copy of the passed parameter
 * @param val The value to use
 * @return The #Eina_Value
 * @since 1.21
 */
static inline Eina_Value
eina_value_reference_copy(const Eina_Value *val)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   if (!eina_value_setup(&v, eina_value_type_get(val)) ||
       !eina_value_copy(val, &v))
     {
        eina_value_setup(&v, EINA_VALUE_TYPE_ERROR);
        eina_value_set(&v, EINA_ERROR_VALUE_FAILED);
     }
   return v;
}

/**
 * @brief Copy the stringshare in the passed #Eina_Value
 * @param val The value to copy
 * @param str The pointer to copy the stringshare to
 * @return @c EINA_TRUE on success
 * @since 1.21
 */
static inline Eina_Bool
eina_value_stringshare_copy(const Eina_Value *val, Eina_Stringshare **str)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(val, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   *str = NULL;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(val->type == EINA_VALUE_TYPE_STRINGSHARE, EINA_FALSE);
   if (!eina_value_get(val, str)) return EINA_FALSE;
   eina_stringshare_ref(*str);
   return EINA_TRUE;
}

/**
 * @brief Copy the string in the passed #Eina_Value
 * @param val The value to copy
 * @param str The pointer to copy the string to
 * @return @c EINA_TRUE on success
 * @since 1.21
 */
static inline Eina_Bool
eina_value_string_copy(const Eina_Value *val, char **str)
{
   char *s;
   EINA_SAFETY_ON_NULL_RETURN_VAL(val, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   *str = NULL;
   EINA_SAFETY_ON_FALSE_RETURN_VAL((val->type == EINA_VALUE_TYPE_STRINGSHARE || val->type == EINA_VALUE_TYPE_STRING), EINA_FALSE);
   if (!eina_value_get(val, &s)) return EINA_FALSE;
   *str = s ? strdup(s) : NULL;
   return EINA_TRUE;
}

#endif

/**
 * @}
 */
