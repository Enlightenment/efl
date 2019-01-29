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

/// @brief For internal use only.
/// @hideinitializer
#define EINA_VALUE_NEW(Compress_Type, Uppercase_Compress_Type, Type)    \
/**
 @brief Create a new #Eina_Value containing the passed parameter
 @param c The value to use
 @return The #Eina_Value
 @since 1.21
 */                                                                     \
static inline Eina_Value *                                              \
eina_value_##Compress_Type##_new(Type c)                                \
{                                                                       \
   Eina_Value *v;                                                       \
                                                                        \
   v = eina_value_new(EINA_VALUE_TYPE_##Uppercase_Compress_Type);       \
   if (v) eina_value_set(v, c);                                         \
   return v;                                                            \
}

EINA_VALUE_NEW(uchar, UCHAR, unsigned char);
EINA_VALUE_NEW(ushort, USHORT, unsigned short);
EINA_VALUE_NEW(uint, UINT, unsigned int);
EINA_VALUE_NEW(ulong, ULONG, unsigned long);
EINA_VALUE_NEW(uint64, UINT64, uint64_t);
EINA_VALUE_NEW(char, CHAR, char);
EINA_VALUE_NEW(short, SHORT, short);
EINA_VALUE_NEW(int, INT, int);
EINA_VALUE_NEW(long, LONG, long);
EINA_VALUE_NEW(int64, INT64, int64_t);
EINA_VALUE_NEW(float, FLOAT, float);
EINA_VALUE_NEW(double, DOUBLE, double);
EINA_VALUE_NEW(bool, BOOL, Eina_Bool);
EINA_VALUE_NEW(string, STRING, const char *);
EINA_VALUE_NEW(stringshare, STRINGSHARE, const char *);
EINA_VALUE_NEW(time, TIMESTAMP, time_t);
EINA_VALUE_NEW(error, ERROR, Eina_Error);

/// @brief For internal use only.
/// @hideinitializer
#define EINA_VALUE_INIT(Compress_Type, Uppercase_Compress_Type, Type)   \
/**
 @brief Initialize #Eina_Value containing the passed parameter
 @param c The value to use
 @return The #Eina_Value
 @since 1.21
 */                                                                     \
static inline Eina_Value                                                \
eina_value_##Compress_Type##_init(Type c)                               \
{                                                                       \
   Eina_Value v = EINA_VALUE_EMPTY;                                     \
                                                                        \
   if (eina_value_setup(&v, EINA_VALUE_TYPE_##Uppercase_Compress_Type)) \
     eina_value_set(&v, c);                                             \
   return v;                                                            \
}

EINA_VALUE_INIT(uchar, UCHAR, unsigned char);
EINA_VALUE_INIT(ushort, USHORT, unsigned short);
EINA_VALUE_INIT(uint, UINT, unsigned int);
EINA_VALUE_INIT(ulong, ULONG, unsigned long);
EINA_VALUE_INIT(uint64, UINT64, uint64_t);
EINA_VALUE_INIT(char, CHAR, char);
EINA_VALUE_INIT(short, SHORT, short);
EINA_VALUE_INIT(int, INT, int);
EINA_VALUE_INIT(long, LONG, long);
EINA_VALUE_INIT(int64, INT64, int64_t);
EINA_VALUE_INIT(float, FLOAT, float);
EINA_VALUE_INIT(double, DOUBLE, double);
EINA_VALUE_INIT(bool, BOOL, Eina_Bool);
EINA_VALUE_INIT(string, STRING, const char *);
EINA_VALUE_INIT(stringshare, STRINGSHARE, const char *);
EINA_VALUE_INIT(time, TIMESTAMP, time_t);
EINA_VALUE_INIT(error, ERROR, Eina_Error);

/// @brief For internal use only.
/// @hideinitializer
#define EINA_VALUE_GET(Compress_Type, Uppercase_Compress_Type, Type)    \
/**
 @brief Check value type and get contents.
 @param v The value to check type and get contents.
 @param c Where to store the value contents.
 @return #EINA_TRUE if type matches and fetched contents,
 #EINA_FALSE on different type or failures.
 @since 1.21
 */                                                                     \
static inline Eina_Bool                                                 \
eina_value_##Compress_Type##_get(const Eina_Value *v, Type *c)          \
{                                                                       \
   EINA_SAFETY_ON_NULL_RETURN_VAL(c, EINA_FALSE);                       \
   *c = 0;                                                              \
   if (v && v->type == EINA_VALUE_TYPE_##Uppercase_Compress_Type)       \
     return eina_value_get(v, c);                                       \
   return EINA_FALSE;                                                   \
}

EINA_VALUE_GET(uchar, UCHAR, unsigned char);
EINA_VALUE_GET(ushort, USHORT, unsigned short);
EINA_VALUE_GET(uint, UINT, unsigned int);
EINA_VALUE_GET(ulong, ULONG, unsigned long);
EINA_VALUE_GET(uint64, UINT64, uint64_t);
EINA_VALUE_GET(char, CHAR, char);
EINA_VALUE_GET(short, SHORT, short);
EINA_VALUE_GET(int, INT, int);
EINA_VALUE_GET(long, LONG, long);
EINA_VALUE_GET(int64, INT64, int64_t);
EINA_VALUE_GET(float, FLOAT, float);
EINA_VALUE_GET(double, DOUBLE, double);
EINA_VALUE_GET(bool, BOOL, Eina_Bool);
EINA_VALUE_GET(string, STRING, const char *);
EINA_VALUE_GET(stringshare, STRINGSHARE, const char *);
EINA_VALUE_GET(time, TIMESTAMP, time_t);
EINA_VALUE_GET(error, ERROR, Eina_Error);

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
