/* EINA - EFL data type library
 * Copyright (C) 2012 ProFUSION embedded systems
 *
 * This library is a free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_VALUE_H_
#define EINA_VALUE_H_

#include <stdarg.h>

#include "eina_types.h"
#include "eina_fp.h" /* defines int64_t and uint64_t */
#include "eina_inarray.h"
#include "eina_list.h"
#include "eina_hash.h"

/**
 * @internal
 * @defgroup Eina_Value_Group Generic Value Storage
 * @ingroup Eina_Containers_Group
 *
 * @since 1.2
 *
 * @brief Abstracts generic data storage and access to it in an extensible
 *        and efficient way.
 *
 * @remarks It comes with pre-defined types for numbers, array, list, hash,
 *          blob, and structs. It is able to convert between data types,
 *          including string.
 *
 *          It is meant for simple data types, providing uniform access and
 *          release functions, useful to exchange data by preserving their
 *          types. For more complex hierarchical data, with properties and
 *          children, reference counting, inheritance, and interfaces are also included.
 *
 * @{
 */


/**
 * @typedef Eina_Value
 * @brief The structure type that stores generic values.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
typedef struct _Eina_Value Eina_Value;

/**
 * @typedef Eina_Value_Type
 * @brief The strcuture type that describes the data contained by the value.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
typedef struct _Eina_Value_Type Eina_Value_Type;

/**
 * @typedef Eina_Value_Union
 * @brief Union of all known value types.
 *
 * @remarks This is only used to specify the minimum payload memory for #Eina_Value.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
typedef union _Eina_Value_Union Eina_Value_Union;

/**
 * @internal
 * @union _Eina_Value_Union
 * @brief All possible value types.
 *
 * @remarks This is only used to specify the minimum payload memory for #Eina_Value.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
union _Eina_Value_Union
{
   unsigned char  buf[8];    /**< Just hold 8-bytes, more goes into ptr */
   void          *ptr;       /**< Used as a generic pointer */
   uint64_t      _guarantee; /**< Guarantees 8-byte alignment */
};

/**
 * @var EINA_VALUE_TYPE_UCHAR
 * @brief Manages unsigned char type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_UCHAR;

/**
 * @var EINA_VALUE_TYPE_USHORT
 * @brief Manages unsigned short type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_USHORT;

/**
 * @var EINA_VALUE_TYPE_UINT
 * @brief Manages unsigned int type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_UINT;

/**
 * @var EINA_VALUE_TYPE_ULONG
 * @brief Manages unsigned long type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_ULONG;

/**
 * @var EINA_VALUE_TYPE_TIMESTAMP
 * @brief Manages unsigned long type used for timestamps.
 * @remarks This is identical in function to @c EINA_VALUE_TYPE_ULONG.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_TIMESTAMP;

/**
 * @var EINA_VALUE_TYPE_UINT64
 * @brief Manages unsigned integer of 64 bits type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_UINT64;

/**
 * @var EINA_VALUE_TYPE_CHAR
 * @brief Manages char type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_CHAR;

/**
 * @var EINA_VALUE_TYPE_SHORT
 * @brief Manages short type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_SHORT;

/**
 * @var EINA_VALUE_TYPE_INT
 * @brief Manages int type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_INT;

/**
 * @var EINA_VALUE_TYPE_LONG
 * @brief Manages long type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_LONG;

/**
 * @var EINA_VALUE_TYPE_INT64
 * @brief Manages integer of 64 bits type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_INT64;

/**
 * @var EINA_VALUE_TYPE_FLOAT
 * @brief Manages float type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_FLOAT;

/**
 * @var EINA_VALUE_TYPE_DOUBLE
 * @brief Manages double type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_DOUBLE;

/**
 * @var EINA_VALUE_TYPE_STRINGSHARE
 * @brief Manages stringshared string type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_STRINGSHARE;

/**
 * @var EINA_VALUE_TYPE_STRING
 * @brief Manages string type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_STRING;


/**
 * @var EINA_VALUE_TYPE_ARRAY
 *
 * @brief Manages array type. Use the value get/set for arrays:
 *  @li eina_value_array_get() and eina_value_array_set()
 *  @li eina_value_array_vget() and eina_value_array_vset()
 *  @li eina_value_array_pget() and eina_value_array_pset()
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks eina_value_set() takes an #Eina_Value_Array where just @c subtype
 *          and @c step are used. If there is an @c array, it is copied
 *          (including each item) and its contents must be properly
 *          configurable as @c subtype expects. eina_value_pset() takes a
 *          pointer to an #Eina_Value_Array.  For your convenience, use
 *          eina_value_array_setup().
 *
 * @remarks eina_value_get() and eina_value_pget() take a pointer
 *          to #Eina_Value_Array, it's an exact copy of the current structure in
 *          use by value, no copies are done.
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_ARRAY;

/**
 * @var EINA_VALUE_TYPE_LIST
 *
 * @brief Manages list type. Use the value get/set for lists:
 *  @li eina_value_list_get() and eina_value_list_set()
 *  @li eina_value_list_vget() and eina_value_list_vset()
 *  @li eina_value_list_pget() and eina_value_list_pset()
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks eina_value_set() takes an #Eina_Value_List where just @c subtype is
 *          used. If there is a @c list, it is copied (including each
 *          item) and its contents must be properly configurable as @c
 *          subtype expects. eina_value_pset() takes a pointer to an #Eina_Value_List.
 *          For your convenience, use eina_value_list_setup().
 *
 * @remarks eina_value_get() and eina_value_pget() take a pointer to #Eina_Value_List,
 *          it's an exact copy of the current structure in use by value, no copies are
 *          done.
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_LIST;

/**
 * @var EINA_VALUE_TYPE_HASH
 *
 * @brief Manages hash type. Use the value get/set for hashes:
 *  @li eina_value_hash_get() and eina_value_hash_set()
 *  @li eina_value_hash_vget() and eina_value_hash_vset()
 *  @li eina_value_hash_pget() and eina_value_hash_pset()
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks eina_value_set() takes an #Eina_Value_Hash where just @c subtype
 *          and @c buckets_power_size are used. If there is an @c hash, it is
 *          copied (including each item) and its contents must be
 *          properly configurable as @c subtype expects. eina_value_pset()
 *          takes a pointer to an #Eina_Value_Hash.  For your convenience, use
 *          eina_value_hash_setup().
 *
 * @remarks eina_value_get() and eina_value_pget() take a pointer to #Eina_Value_Hash,
 *          it's an exact copy of the current structure in use by value, no copies are
 *          done.
 *
 * @remarks Be aware that hash data is always an allocated memory of size
 *          defined by @c subtype->value_size. If your @c subtype is an
 *          integer, add as data malloc(sizeof(int)). If your @c subtype
 *          is a string, add as data malloc(sizeof(char*)) and this data
 *          value must point to strdup(string).
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_HASH;

/**
 * @var EINA_VALUE_TYPE_TIMEVAL
 * @brief Manages 'struct timeval' type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks eina_value_set() takes a "struct timeval" from sys/time.h.
 *          eina_value_pset() takes a pointer to "struct timeval".
 *
 * @remarks eina_value_get() and eina_value_pget() take a pointer to "struct
 *          timeval" and it's an exact copy of the value.
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_TIMEVAL;

/**
 * @var EINA_VALUE_TYPE_BLOB
 * @brief Manages blob of bytes type, see @ref Eina_Value_Blob
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks eina_value_set() takes an #Eina_Value_Blob
 *          eina_value_pset() takes a pointer to #Eina_Value_Blob.
 *
 * @remarks eina_value_get() and eina_value_pget() take a pointer to #Eina_Value_Blob
 *          and it's an exact copy of value, no allocations are made.
 *
 * @remarks Memory is untouched unless you provide an @c ops (operations) pointer.
 *
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_BLOB;

/**
 * @var EINA_VALUE_TYPE_STRUCT
 *
 * @brief Manages struct type. Use the value get/set for structs:
 *  @li eina_value_struct_get() and eina_value_struct_set()
 *  @li eina_value_struct_vget() and eina_value_struct_vset()
 *  @li eina_value_struct_pget() and eina_value_struct_pset()
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks eina_value_set() takes an #Eina_Value_Struct where just @c desc is
 *          used. If there is a @c memory, it is copied (including each
 *          member) and its contents must be properly configurable as @c desc
 *          expects. eina_value_pset() takes a pointer to an #Eina_Value_Struct. For
 *          your convenience, use eina_value_struct_setup().
 *
 * @remarks eina_value_get() and eina_value_pget() take a pointer
 *          to #Eina_Value_Struct, it's an exact copy of the current structure in
 *          use by value, no copies are done.
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_STRUCT;

/**
 * @var EINA_ERROR_VALUE_FAILED
 * @brief Error identifier corresponding to a value check failure.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
EAPI extern int EINA_ERROR_VALUE_FAILED;

/**
 * @internal
 * @defgroup Eina_Value_Value_Group Generic Value management
 *
 * @{
 */

/**
 * @internal
 * @struct _Eina_Value
 * @brief The structure type defining the contents of a value.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
struct _Eina_Value
{
   const Eina_Value_Type *type; /**< How to access values */
   Eina_Value_Union value; /**< To be accessed with type descriptor */
};

/**
 * @brief Creates a generic value storage.
 *
 * @details This creates a new generic value storage. The members are managed using
 *          the description specified by @a type.
 *
 *          Some types may specify more operations:
 *          eg. #EINA_VALUE_TYPE_ARRAY uses eina_value_array_set(),
 *          eina_value_array_get(), and so on.
 *
 *          On failure, @c NULL is returned and either #EINA_ERROR_OUT_OF_MEMORY
 *          or #EINA_ERROR_VALUE_FAILED is set.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks This call creates from mempool and then uses
 *          eina_value_setup(). Consider using eina_value_flush() and
 *          eina_value_setup() instead to avoid memory allocations.
 *
 * @param[in] type The type used to manage this value
 * @return The new value, otherwise @c NULL on failure
 *
 * @see eina_value_free()
 *
 */
EAPI Eina_Value *eina_value_new(const Eina_Value_Type *type) EINA_ARG_NONNULL(1) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Frees a value and its data.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The value object
 *
 * @see eina_value_flush()
 */
EAPI void eina_value_free(Eina_Value *value) EINA_ARG_NONNULL(1);


/**
 * @brief Initializes a generic value storage.
 *
 * @details This initializes an existing generic value storage. The members are managed using the
 *          description specified by @a type.
 *
 *          Some types may specify more operations, as an example #EINA_VALUE_TYPE_ARRAY
 *          uses eina_value_array_set(), eina_value_array_get(), and so on.
 *
 *          On failure, @c EINA_FALSE is returned and #EINA_ERROR_OUT_OF_MEMORY
 *          or #EINA_ERROR_VALUE_FAILED is set.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks Existing contents are ignored. If the value is previously used, then
 *          use eina_value_flush() first.
 *
 * @param[in] value The value object
 * @param[in] type The type used to manage this value
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_flush()
 */
static inline Eina_Bool eina_value_setup(Eina_Value *value,
                                         const Eina_Value_Type *type) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Creates a generic value storage.
 *
 * @details This releases all the resources associated with an #Eina_Value. The
 *          value must be already set with eina_value_setup() or
 *          eina_value_new().
 *
 *          After this call returns, the contents of the value are undefined,
 *          but the value can be reused by calling eina_value_setup() again.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The value object
 *
 * @see eina_value_setup()
 * @see eina_value_free()
 *
 */
static inline void eina_value_flush(Eina_Value *value) EINA_ARG_NONNULL(1);

/**
 * @brief Copies a generic value storage.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 *
 * @remarks The @a copy object is considered uninitialized and its existing
 *          contents are overwritten (just as if eina_value_flush() is called on
 *          it).
 *
 *          The copy happens by calling eina_value_setup() on @a copy, followed
 *          by getting the contents of @a value and setting it to @a copy.
 *
 * @param[in] value The source value object
 * @param[in] copy The destination value object
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_value_copy(const Eina_Value *value,
                               Eina_Value *copy) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Compares a generic value storage.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] a The left side of comparison
 * @param[in] b The right side of comparison
 * @return Less than zero if a < b, greater than zero if a > b, zero
 *         if a == b
 */
static inline int eina_value_compare(const Eina_Value *a,
                                     const Eina_Value *b) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Sets the generic value.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The variable argument is dependent on the chosen type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char
 * @li EINA_VALUE_TYPE_USHORT: unsigned short
 * @li EINA_VALUE_TYPE_UINT: unsigned int
 * @li EINA_VALUE_TYPE_ULONG: unsigned long
 * @li EINA_VALUE_TYPE_UINT64: uint64_t
 * @li EINA_VALUE_TYPE_CHAR: char
 * @li EINA_VALUE_TYPE_SHORT: short
 * @li EINA_VALUE_TYPE_INT: int
 * @li EINA_VALUE_TYPE_LONG: long
 * @li EINA_VALUE_TYPE_INT64: int64_t
 * @li EINA_VALUE_TYPE_FLOAT: float
 * @li EINA_VALUE_TYPE_DOUBLE: double
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char *
 * @li EINA_VALUE_TYPE_STRING: const char *
 * @li EINA_VALUE_TYPE_ARRAY: Eina_Value_Array
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct
 *
 * @code
 *     Eina_Value *value = eina_value_new(EINA_VALUE_TYPE_INT);
 *     int x = 567;
 *     eina_value_set(value, 1234);
 *     eina_value_set(value, x);
 *
 *     eina_value_flush(value);
 *
 *     eina_value_setup(value, EINA_VALUE_TYPE_STRING);
 *     eina_value_set(value, "hello world!");
 *
 *     eina_value_free(value);
 * @endcode
 *
 * @remarks For array member see eina_value_array_set().
 * @remarks For list member see eina_value_list_set().
 * @remarks For hash member see eina_value_hash_set().
 *
 * @param[in] value The source value object
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_get()
 * @see eina_value_vset()
 * @see eina_value_pset()
 *
 */
static inline Eina_Bool eina_value_set(Eina_Value *value,
                                       ...) EINA_ARG_NONNULL(1);

/**
 * @brief Gets a generic value.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in the variable argument parameter, the
 *          actual value is type-dependent, but usually it is what is
 *          stored inside the object. There shouldn't be any memory allocation,
 *          thus the contents should @b not be freed.
 *
 * @remarks The variable argument is dependent on the chosen type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_ARRAY: Eina_Value_Array*
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @code
 *     Eina_Value *value = eina_value_new(EINA_VALUE_TYPE_INT);
 *     int x;
 *     const char *s;
 *
 *     eina_value_set(value, 1234);
 *     eina_value_get(value, &x);
 *
 *     eina_value_flush(value);
 *
 *     eina_value_setup(value, EINA_VALUE_TYPE_STRING);
 *     eina_value_set(value, "hello world!");
 *     eina_value_get(value, &s);
 *
 *     eina_value_free(value);
 * @endcode
 *
 * @remarks For array member see eina_value_array_get().
 * @remarks For list member see eina_value_list_get().
 * @remarks For hash member see eina_value_hash_get().
 *
 * @param[in] value The source value object
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_set()
 * @see eina_value_vset()
 * @see eina_value_pset()
 */
static inline Eina_Bool eina_value_get(const Eina_Value *value,
                                       ...) EINA_ARG_NONNULL(1);

/**
 * @brief Sets a generic value.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks For array member see eina_value_array_vset().
 * @remarks For list member see eina_value_list_vset().
 * @remarks For hash member see eina_value_hash_vset().
 *
 * @param[in] value The source value object
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_vget()
 * @see eina_value_set()
 * @see eina_value_pset()
 *
 */
static inline Eina_Bool eina_value_vset(Eina_Value *value,
                                        va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Gets a generic value.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in the variable argument parameter, the
 *          actual value is type-dependent, but usually it is what is
 *          stored inside the object. There shouldn't be any memory allocation,
 *          thus the contents should @b not be freed.
 *
 * @remarks For array member see eina_value_array_vget().
 * @remarks For list member see eina_value_list_vget().
 * @remarks For hash member see eina_value_hash_vget().
 *
 * @param[in] value The source value object
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_vset()
 * @see eina_value_get()
 * @see eina_value_pget()
 */
static inline Eina_Bool eina_value_vget(const Eina_Value *value,
                                        va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Sets a generic value for a pointer.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_ARRAY: Eina_Value_Array*
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @remarks The pointer contents are written using the size defined by the
 *          type. It can be larger than void* or uint64_t.
 *
 * @code
 *     Eina_Value *value = eina_value_new(EINA_VALUE_TYPE_INT);
 *     int x = 567;
 *     const char *s = "hello world!";
 *
 *     eina_value_pset(value, &x);
 *
 *     eina_value_flush(value);
 *
 *     eina_value_setup(value, EINA_VALUE_TYPE_STRING);
 *     eina_value_pset(value, &s);
 *
 *     eina_value_free(value);
 * @endcode
 *
 * @remarks For array member see eina_value_array_pset().
 * @remarks For list member see eina_value_list_pset().
 * @remarks For hash member see eina_value_hash_pset().
 *
 * @param[in] value The source value object
 * @param[in] ptr A pointer to specify the contents
 * @return @c EINA_TRUE on success, otherwise EINA_FALSE
 *
 * @see eina_value_pget()
 * @see eina_value_set()
 * @see eina_value_vset()
 */
static inline Eina_Bool eina_value_pset(Eina_Value *value,
                                        const void *ptr) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Gets a generic value from a pointer.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned as pointer contents, the actual value is
 *          type-dependent, but usually it is what is stored inside the
 *          object. There shouldn't be any memory allocation, thus the contents
 *          should @b not be freed.
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_ARRAY: Eina_Value_Array*
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 *
 * @code
 *     Eina_Value *value = eina_value_new(EINA_VALUE_TYPE_INT);
 *     int x;
 *     const char *s;
 *
 *     eina_value_set(value, 1234);
 *     eina_value_pget(value, &x);
 *
 *     eina_value_flush(value);
 *
 *     eina_value_setup(value, EINA_VALUE_TYPE_STRING);
 *     eina_value_set(value, "hello world!");
 *     eina_value_pget(value, &s);
 *
 *     eina_value_free(value);
 * @endcode
 *
 * @remarks For array member see eina_value_array_get().
 * @remarks For list member see eina_value_list_get().
 * @remarks For hash member see eina_value_hash_get().
 *
 * @param[in] value The source value object
 * @param[in] ptr A pointer to receive the contents
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_set()
 * @see eina_value_vset()
 * @see eina_value_pset()
 *
 */
static inline Eina_Bool eina_value_pget(const Eina_Value *value,
                                        void *ptr) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Converts one value type to another.
 *
 * @details This converts one value to another by first trying the @a value type
 *          using the @c convert_to() function. If unsuccessful, it tries using the @c convert_from()
 *          function with the @a convert type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks Conversion functions are type defined, and the basic types can convert
 *          between themselves, but conversion is strict. That is, when
 *          converting from a negative value to an unsigned type, it fails. It
 *          also fails on value overflow.
 *
 * @remarks It is recommended that all types implement at least convert to
 *          string, used by eina_value_to_string().
 *
 * @remarks Both objects must have eina_value_setup() called on them beforehand.
 *
 * @param[in] value The source value object.
 * @param[in] convert The destination value object.
 * @return @c EINA_TRUE if converted, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_value_convert(const Eina_Value *value,
                                  Eina_Value *convert) EINA_ARG_NONNULL(1, 2);


/**
 * @brief Converts a value to the string type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The value object
 * @return The newly allocated memory, otherwise @c NULL on failure
 *
 * @see eina_value_convert()
 */
EAPI char *eina_value_to_string(const Eina_Value *value) EINA_ARG_NONNULL(1);

/**
 * @brief Queries a value type.
 *
 * @details This checks whether the value type is valid and returns that value type. A type is invalid if
 *          it does not exist or if it is using a different version field.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The value object
 * @return The type instance, otherwise @c NULL if the type is invalid
 *
 * @see eina_value_type_check()
 *
 */
static inline const Eina_Value_Type *eina_value_type_get(const Eina_Value *value) EINA_PURE EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @}
 */


/**
 * @internal
 * @defgroup Eina_Value_Array_Group Generic Value Array management
 *
 * @{
 */


/**
 * @typedef Eina_Value_Array
 * @brief The structure type defining the value type for #EINA_VALUE_TYPE_ARRAY.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @see #_Eina_Value_Array explains fields.
 */
typedef struct _Eina_Value_Array Eina_Value_Array;

/**
 * @internal
 * @struct _Eina_Value_Array
 * The structure type used to store the array and its subtype.
 * @since 1.2
 *
 * @since_tizen 2.3
 */
struct _Eina_Value_Array
{
   const Eina_Value_Type *subtype; /**< How to allocate and access items */
   unsigned int step; /**< How to grow the members array */
   Eina_Inarray *array; /**< The array that holds data, members are of subtype->value_size bytes */
};

/**
 * @brief Creates a generic value storage of type array.
 *
 * @details This creates a new generic value storage of type array. The members are
 *          managed using the description specified by @a subtype.
 *
 *          On failure, @c NULL is returned and #EINA_ERROR_OUT_OF_MEMORY
 *          or #EINA_ERROR_VALUE_FAILED is set.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks This creates from the mempool and then uses
 *          eina_value_array_setup().
 *
 * @param subtype The subtype to manage these array members
 * @param step The step value used to make the members array grow
 * @return The new value, otherwise @c NULL on failure
 *
 * @see eina_value_free()
 * @see eina_value_array_setup()
 */
EAPI Eina_Value *eina_value_array_new(const Eina_Value_Type *subtype,
                                      unsigned int step) EINA_ARG_NONNULL(1);

/**
 * @brief Initializes a generic value storage of type array.
 *
 * @details This initializes a new generic value storage of type array with the given
 *          @a subtype.
 *
 *          This is same as calling eina_value_set()
 *          with #EINA_VALUE_TYPE_ARRAY followed by eina_value_pset() with
 *          the #Eina_Value_Array description configured.
 *
 *          On failure, @c EINA_FALSE is returned and #EINA_ERROR_OUT_OF_MEMORY
 *          or #EINA_ERROR_VALUE_FAILED is set.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks Existing contents are ignored. If the value is previously used, then
 *          use eina_value_flush() first.
 *
 * @param[in] value The value object
 * @param[in] subtype how to manage array members.
 * @param[in] step The step value used to make the members array grow
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_flush()
 *
 */
static inline Eina_Bool eina_value_array_setup(Eina_Value *value,
                                               const Eina_Value_Type *subtype,
                                               unsigned int step) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Queries the number of elements in a value of array type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The value object
 * @return The number of child elements
 */
static inline unsigned int eina_value_array_count(const Eina_Value *value);

/**
 * @brief Removes the element at the given position in a value of array type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The value object
 * @param[in] position The index of the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
static inline Eina_Bool eina_value_array_remove(Eina_Value *value,
                                                unsigned int position) EINA_ARG_NONNULL(1);

/**
 * @brief Sets a generic value for an array member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The variable argument is dependent on the chosen subtype. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char
 * @li EINA_VALUE_TYPE_USHORT: unsigned short
 * @li EINA_VALUE_TYPE_UINT: unsigned int
 * @li EINA_VALUE_TYPE_ULONG: unsigned long
 * @li EINA_VALUE_TYPE_UINT64: uint64_t
 * @li EINA_VALUE_TYPE_CHAR: char
 * @li EINA_VALUE_TYPE_SHORT: short
 * @li EINA_VALUE_TYPE_INT: int
 * @li EINA_VALUE_TYPE_LONG: long
 * @li EINA_VALUE_TYPE_INT64: int64_t
 * @li EINA_VALUE_TYPE_FLOAT: float
 * @li EINA_VALUE_TYPE_DOUBLE: double
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char *
 * @li EINA_VALUE_TYPE_STRING: const char *
 * @li EINA_VALUE_TYPE_ARRAY: Eina_Value_Array
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct
 *
 * @code
 *     Eina_Value *value = eina_value_array_new(EINA_VALUE_TYPE_INT, 0);
 *     int x;
 *
 *     eina_value_array_append(value, 1234);
 *     eina_value_array_set(value, 0, 5678);
 *     eina_value_array_get(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_array_get()
 * @see eina_value_array_vset()
 * @see eina_value_array_pset()
 * @see eina_value_array_insert()
 * @see eina_value_array_vinsert()
 * @see eina_value_array_pinsert()
 * @see eina_value_array_append()
 * @see eina_value_array_vappend()
 * @see eina_value_array_pappend()
 *
 */
static inline Eina_Bool eina_value_array_set(Eina_Value *value,
                                             unsigned int position,
                                             ...) EINA_ARG_NONNULL(1);

/**
 * @brief Gets a generic value from an array member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in the variable argument parameter, the
 *          actual value is type-dependent, but usually it is what is
 *          stored inside the object. There shouldn't be any memory allocation;
 *          thus the contents should @b not be freed.
 *
 * @remarks The variable argument is dependent on the chosen subtype. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_ARRAY: Eina_Value_Array*
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @code
 *     Eina_Value *value = eina_value_array_new(EINA_VALUE_TYPE_INT, 0);
 *     int x;
 *
 *     eina_value_array_append(value, 1234);
 *     eina_value_array_get(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_array_set()
 * @see eina_value_array_vset()
 * @see eina_value_array_pset()
 *
 */
static inline Eina_Bool eina_value_array_get(const Eina_Value *value,
                                             unsigned int position,
                                             ...) EINA_ARG_NONNULL(1);

/**
 * @brief Inserts a generic value at an array member position.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The variable argument is dependent on the chosen subtype. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char
 * @li EINA_VALUE_TYPE_USHORT: unsigned short
 * @li EINA_VALUE_TYPE_UINT: unsigned int
 * @li EINA_VALUE_TYPE_ULONG: unsigned long
 * @li EINA_VALUE_TYPE_UINT64: uint64_t
 * @li EINA_VALUE_TYPE_CHAR: char
 * @li EINA_VALUE_TYPE_SHORT: short
 * @li EINA_VALUE_TYPE_INT: int
 * @li EINA_VALUE_TYPE_LONG: long
 * @li EINA_VALUE_TYPE_INT64: int64_t
 * @li EINA_VALUE_TYPE_FLOAT: float
 * @li EINA_VALUE_TYPE_DOUBLE: double
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char *
 * @li EINA_VALUE_TYPE_STRING: const char *
 * @li EINA_VALUE_TYPE_ARRAY: Eina_Value_Array
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct
 *
 * @code
 *     Eina_Value *value = eina_value_array_new(EINA_VALUE_TYPE_INT, 0);
 *     int x;
 *
 *     eina_value_array_insert(value, 0, 1234);
 *     eina_value_array_get(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_array_set()
 * @see eina_value_array_get()
 * @see eina_value_array_vset()
 * @see eina_value_array_pset()
 * @see eina_value_array_vinsert()
 * @see eina_value_array_pinsert()
 * @see eina_value_array_append()
 * @see eina_value_array_vappend()
 * @see eina_value_array_pappend()
 *
 */
static inline Eina_Bool eina_value_array_insert(Eina_Value *value,
                                                unsigned int position,
                                                ...) EINA_ARG_NONNULL(1);


/**
 * @brief Appends a generic value to an array.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The variable argument is dependent on the chosen subtype. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char
 * @li EINA_VALUE_TYPE_USHORT: unsigned short
 * @li EINA_VALUE_TYPE_UINT: unsigned int
 * @li EINA_VALUE_TYPE_ULONG: unsigned long
 * @li EINA_VALUE_TYPE_UINT64: uint64_t
 * @li EINA_VALUE_TYPE_CHAR: char
 * @li EINA_VALUE_TYPE_SHORT: short
 * @li EINA_VALUE_TYPE_INT: int
 * @li EINA_VALUE_TYPE_LONG: long
 * @li EINA_VALUE_TYPE_INT64: int64_t
 * @li EINA_VALUE_TYPE_FLOAT: float
 * @li EINA_VALUE_TYPE_DOUBLE: double
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char *
 * @li EINA_VALUE_TYPE_STRING: const char *
 * @li EINA_VALUE_TYPE_ARRAY: Eina_Value_Array
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct
 *
 * @code
 *     Eina_Value *value = eina_value_array_new(EINA_VALUE_TYPE_INT, 0);
 *     int x;
 *
 *     eina_value_array_append(value, 1234);
 *     eina_value_array_get(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_array_set()
 * @see eina_value_array_get()
 * @see eina_value_array_vset()
 * @see eina_value_array_pset()
 * @see eina_value_array_vinsert()
 * @see eina_value_array_pinsert()
 * @see eina_value_array_append()
 * @see eina_value_array_vappend()
 * @see eina_value_array_pappend()
 *
 */
static inline Eina_Bool eina_value_array_append(Eina_Value *value,
                                                ...) EINA_ARG_NONNULL(1);

/**
 * @brief Sets a generic value for an array member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 * @see eina_value_array_set()
 * @see eina_value_array_get()
 * @see eina_value_array_pset()
 * @see eina_value_array_insert()
 * @see eina_value_array_vinsert()
 * @see eina_value_array_pinsert()
 * @see eina_value_array_append()
 * @see eina_value_array_vappend()
 * @see eina_value_array_pappend()
 *
 */
static inline Eina_Bool eina_value_array_vset(Eina_Value *value,
                                              unsigned int position,
                                              va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Gets a generic value from an array member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in the variable argument parameter, the
 *          actual value is type-dependent, but usually it is what is
 *          stored inside the object. There shouldn't be any memory allocation,
 *          thus the contents should @b not be freed.
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_array_vset()
 * @see eina_value_array_get()
 * @see eina_value_array_pget()
 *
 */
static inline Eina_Bool eina_value_array_vget(const Eina_Value *value,
                                              unsigned int position,
                                              va_list args) EINA_ARG_NONNULL(1);
/**
 * @brief Inserts a generic value at an array member position.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 * @see eina_value_array_set()
 * @see eina_value_array_get()
 * @see eina_value_array_vset()
 * @see eina_value_array_pset()
 * @see eina_value_array_insert()
 * @see eina_value_array_pinsert()
 * @see eina_value_array_append()
 * @see eina_value_array_vappend()
 * @see eina_value_array_pappend()
 */
static inline Eina_Bool eina_value_array_vinsert(Eina_Value *value,
                                                unsigned int position,
                                                va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Appends a generic value to an array.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The source value object
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 * @see eina_value_array_set()
 * @see eina_value_array_get()
 * @see eina_value_array_vget()
 * @see eina_value_array_pset()
 * @see eina_value_array_insert()
 * @see eina_value_array_vinsert()
 * @see eina_value_array_pinsert()
 * @see eina_value_array_append()
 * @see eina_value_array_pappend()
 */
static inline Eina_Bool eina_value_array_vappend(Eina_Value *value,
                                                 va_list args) EINA_ARG_NONNULL(1);


/**
 * @brief Sets a generic value for an array member from a pointer.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_ARRAY: Eina_Value_Array*
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @remarks The pointer contents are written using the size defined by
 *          type. It can be larger than void* or uint64_t.
 *
 * @code
 *     Eina_Value *value = eina_value_array_new(EINA_VALUE_TYPE_INT, 0);
 *     int x = 1234;
 *
 *     eina_value_array_append(value, 1234);
 *     eina_value_array_pset(value, 0, &x);
 *     eina_value_array_pget(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @param[in] ptr A pointer to specify the contents.
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_array_set()
 * @see eina_value_array_get()
 * @see eina_value_array_vset()
 * @see eina_value_array_insert()
 * @see eina_value_array_vinsert()
 * @see eina_value_array_pinsert()
 * @see eina_value_array_append()
 * @see eina_value_array_vappend()
 * @see eina_value_array_pappend()
 *
 */
static inline Eina_Bool eina_value_array_pset(Eina_Value *value,
                                              unsigned int position,
                                              const void *ptr) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Gets a generic value into a pointer from an array member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned as pointer contents, the actual value is
 *          type-dependent, but usually it is what is stored inside the
 *          object. There shouldn't be any memory allocation, thus the contents
 *          should @b not be freed.
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_ARRAY: Eina_Value_Array*
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @code
 *     Eina_Value *value = eina_value_array_new(EINA_VALUE_TYPE_INT, 0);
 *     int x;
 *
 *     eina_value_array_append(value, 1234);
 *     eina_value_array_pget(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @param[in] ptr A pointer to receive the contents
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_array_set()
 * @see eina_value_array_vset()
 * @see eina_value_array_pset()
 *
 */
static inline Eina_Bool eina_value_array_pget(const Eina_Value *value,
                                              unsigned int position,
                                              void *ptr) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Inserts a generic value at an array member position from a pointer.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_ARRAY: Eina_Value_Array*
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @remarks The pointer contents are written using the size defined by
 *          type. It can be larger than void* or uint64_t.
 *
 * @code
 *     Eina_Value *value = eina_value_array_new(EINA_VALUE_TYPE_INT, 0);
 *     int x = 1234;
 *
 *     eina_value_array_pinsert(value, 0, &x);
 *     eina_value_array_pget(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @param[in] ptr A pointer to specify the contents
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_array_set()
 * @see eina_value_array_get()
 * @see eina_value_array_vset()
 * @see eina_value_array_insert()
 * @see eina_value_array_vinsert()
 * @see eina_value_array_pinsert()
 * @see eina_value_array_append()
 * @see eina_value_array_vappend()
 * @see eina_value_array_pappend()
 *
 */
static inline Eina_Bool eina_value_array_pinsert(Eina_Value *value,
                                                 unsigned int position,
                                                 const void *ptr) EINA_ARG_NONNULL(1);

/**
 * @brief Appends a generic value to an array from a pointer.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_ARRAY: Eina_Value_Array*
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @remarks The pointer contents are written using the size defined by
 *          type. It can be larger than void* or uint64_t.
 *
 * @code
 *     Eina_Value *value = eina_value_array_new(EINA_VALUE_TYPE_INT, 0);
 *     int x = 1234;
 *
 *     eina_value_array_pappend(value, &x);
 *     eina_value_array_pget(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] ptr A pointer to specify the contents
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_array_set()
 * @see eina_value_array_get()
 * @see eina_value_array_vset()
 * @see eina_value_array_insert()
 * @see eina_value_array_vinsert()
 * @see eina_value_array_pinsert()
 * @see eina_value_array_append()
 * @see eina_value_array_vappend()
 * @see eina_value_array_pappend()
 *
 */
static inline Eina_Bool eina_value_array_pappend(Eina_Value *value,
                                                 const void *ptr) EINA_ARG_NONNULL(1);

/**
 * @brief Gets a value from the array as an Eina_Value copy.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The argument @a dst is considered as uninitialized and it's setup to
 *          the type of the member.
 *
 * @param[in] src The source value object
 * @param[in] position The index of the member
 * @param[in] dst The location to return the array member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
static inline Eina_Bool eina_value_array_value_get(const Eina_Value *src,
                                                   unsigned int position,
                                                   Eina_Value *dst) EINA_ARG_NONNULL(1, 3);

/**
 * @}
 */


/**
 * @internal
 * @defgroup Eina_Value_List_Group Generic Value List management
 *
 * @{
 */


/**
 * @typedef Eina_Value_List
 * @brief The structure type defining the value type for #EINA_VALUE_TYPE_LIST.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @see #_Eina_Value_List explains fields.
 */
typedef struct _Eina_Value_List Eina_Value_List;

/**
 * @internal
 * @struct _Eina_Value_List
 * @brief The structure type used to store the list and its subtype.
 * @since 1.2
 *
 * @since_tizen 2.3
 */
struct _Eina_Value_List
{
   const Eina_Value_Type *subtype; /**< how to allocate and access items */
   Eina_List *list; /**< the list that holds data, members are of subtype->value_size bytes. */
};

/**
 * @brief Creates a generic value storage of type list.
 *
 * @details This creates a new generic value storage of type list. The members are
 *          managed using the description specified by @a subtype.
 *
 *          On failure, @c NULL is returned and #EINA_ERROR_OUT_OF_MEMORY
 *          or #EINA_ERROR_VALUE_FAILED is set.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks This creates from the mempool and then uses
 *          eina_value_list_setup().
 *
 * @param[in] subtype The subtype to manage these list members
 * @return The new value, otherwise @c NULL on failure
 *
 * @see eina_value_free()
 * @see eina_value_list_setup()
 *
 */
EAPI Eina_Value *eina_value_list_new(const Eina_Value_Type *subtype) EINA_ARG_NONNULL(1);

/**
 * @brief Initializes a generic value storage of type list.
 *
 * @details This initializes a new generic value storage of type list with the given
 *          @a subtype.
 *
 *          This is same as calling eina_value_set()
 *          with #EINA_VALUE_TYPE_LIST followed by eina_value_pset() with
 *          the #Eina_Value_List description configured.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks Existing contents are ignored. If the value is previously used, then
 *          use eina_value_flush() first.
 *
 *          On failure, #EINA_FALSE is returned and #EINA_ERROR_OUT_OF_MEMORY
 *          or #EINA_ERROR_VALUE_FAILED is set.
 *
 * @param[in] value The value object
 * @param[in] subtype The subtype to manage these list members
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_flush()
 */
static inline Eina_Bool eina_value_list_setup(Eina_Value *value,
                                               const Eina_Value_Type *subtype) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Queries the number of elements in a value of list type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The value object
 * @return The number of child elements
 */
static inline unsigned int eina_value_list_count(const Eina_Value *value);

/**
 * @brief Removes the element at the given position in a value of list type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The value object
 * @param[in] position The index of the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
static inline Eina_Bool eina_value_list_remove(Eina_Value *value,
                                                unsigned int position) EINA_ARG_NONNULL(1);

/**
 * @brief Sets a generic value for a list member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The variable argument is dependent on the chosen subtype. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char
 * @li EINA_VALUE_TYPE_USHORT: unsigned short
 * @li EINA_VALUE_TYPE_UINT: unsigned int
 * @li EINA_VALUE_TYPE_ULONG: unsigned long
 * @li EINA_VALUE_TYPE_UINT64: uint64_t
 * @li EINA_VALUE_TYPE_CHAR: char
 * @li EINA_VALUE_TYPE_SHORT: short
 * @li EINA_VALUE_TYPE_INT: int
 * @li EINA_VALUE_TYPE_LONG: long
 * @li EINA_VALUE_TYPE_INT64: int64_t
 * @li EINA_VALUE_TYPE_FLOAT: float
 * @li EINA_VALUE_TYPE_DOUBLE: double
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char *
 * @li EINA_VALUE_TYPE_STRING: const char *
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct
 *
 * @code
 *     Eina_Value *value = eina_value_list_new(EINA_VALUE_TYPE_INT);
 *     int x;
 *
 *     eina_value_list_append(value, 1234);
 *     eina_value_list_set(value, 0, 5678);
 *     eina_value_list_get(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_list_get()
 * @see eina_value_list_vset()
 * @see eina_value_list_pset()
 * @see eina_value_list_insert()
 * @see eina_value_list_vinsert()
 * @see eina_value_list_pinsert()
 * @see eina_value_list_append()
 * @see eina_value_list_vappend()
 * @see eina_value_list_pappend()
 *
 */
static inline Eina_Bool eina_value_list_set(Eina_Value *value,
                                             unsigned int position,
                                             ...) EINA_ARG_NONNULL(1);

/**
 * @brief Gets a generic value from a list member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in the variable argument parameter, the
 *          actual value is type-dependent, but usually it is what is
 *          stored inside the object. There shouldn't be any memory allocation,
 *          thus the contents should @b not be freed.
 *
 * @remarks The variable argument is dependent on the chosen subtype. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @code
 *     Eina_Value *value = eina_value_list_new(EINA_VALUE_TYPE_INT);
 *     int x;
 *
 *     eina_value_list_append(value, 1234);
 *     eina_value_list_get(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_list_set()
 * @see eina_value_list_vset()
 * @see eina_value_list_pset()
 *
 */
static inline Eina_Bool eina_value_list_get(const Eina_Value *value,
                                             unsigned int position,
                                             ...) EINA_ARG_NONNULL(1);

/**
 * @brief Inserts a generic value at a list member position.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The variable argument is dependent on the chosen subtype. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char
 * @li EINA_VALUE_TYPE_USHORT: unsigned short
 * @li EINA_VALUE_TYPE_UINT: unsigned int
 * @li EINA_VALUE_TYPE_ULONG: unsigned long
 * @li EINA_VALUE_TYPE_UINT64: uint64_t
 * @li EINA_VALUE_TYPE_CHAR: char
 * @li EINA_VALUE_TYPE_SHORT: short
 * @li EINA_VALUE_TYPE_INT: int
 * @li EINA_VALUE_TYPE_LONG: long
 * @li EINA_VALUE_TYPE_INT64: int64_t
 * @li EINA_VALUE_TYPE_FLOAT: float
 * @li EINA_VALUE_TYPE_DOUBLE: double
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char *
 * @li EINA_VALUE_TYPE_STRING: const char *
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct
 *
 * @code
 *     Eina_Value *value = eina_value_list_new(EINA_VALUE_TYPE_INT);
 *     int x;
 *
 *     eina_value_list_insert(value, 0, 1234);
 *     eina_value_list_get(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_list_set()
 * @see eina_value_list_get()
 * @see eina_value_list_vset()
 * @see eina_value_list_pset()
 * @see eina_value_list_vinsert()
 * @see eina_value_list_pinsert()
 * @see eina_value_list_append()
 * @see eina_value_list_vappend()
 * @see eina_value_list_pappend()
 *
 */
static inline Eina_Bool eina_value_list_insert(Eina_Value *value,
                                                unsigned int position,
                                                ...) EINA_ARG_NONNULL(1);


/**
 * @brief Appends a generic value to a list.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The variable argument is dependent on the chosen subtype. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char
 * @li EINA_VALUE_TYPE_USHORT: unsigned short
 * @li EINA_VALUE_TYPE_UINT: unsigned int
 * @li EINA_VALUE_TYPE_ULONG: unsigned long
 * @li EINA_VALUE_TYPE_UINT64: uint64_t
 * @li EINA_VALUE_TYPE_CHAR: char
 * @li EINA_VALUE_TYPE_SHORT: short
 * @li EINA_VALUE_TYPE_INT: int
 * @li EINA_VALUE_TYPE_LONG: long
 * @li EINA_VALUE_TYPE_INT64: int64_t
 * @li EINA_VALUE_TYPE_FLOAT: float
 * @li EINA_VALUE_TYPE_DOUBLE: double
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char *
 * @li EINA_VALUE_TYPE_STRING: const char *
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct
 *
 * @code
 *     Eina_Value *value = eina_value_list_new(EINA_VALUE_TYPE_INT);
 *     int x;
 *
 *     eina_value_list_append(value, 1234);
 *     eina_value_list_get(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_list_set()
 * @see eina_value_list_get()
 * @see eina_value_list_vset()
 * @see eina_value_list_pset()
 * @see eina_value_list_vinsert()
 * @see eina_value_list_pinsert()
 * @see eina_value_list_append()
 * @see eina_value_list_vappend()
 * @see eina_value_list_pappend()
 *
 */
static inline Eina_Bool eina_value_list_append(Eina_Value *value,
                                                ...) EINA_ARG_NONNULL(1);

/**
 * @brief Sets a generic value for a list member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 * @see eina_value_list_set()
 * @see eina_value_list_get()
 * @see eina_value_list_pset()
 * @see eina_value_list_insert()
 * @see eina_value_list_vinsert()
 * @see eina_value_list_pinsert()
 * @see eina_value_list_append()
 * @see eina_value_list_vappend()
 * @see eina_value_list_pappend()
 *
 */
static inline Eina_Bool eina_value_list_vset(Eina_Value *value,
                                              unsigned int position,
                                              va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Gets a generic value from a list member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in the variable argument parameter, the
 *          actual value is type-dependent, but usually it is what is
 *          stored inside the object. There shouldn't be any memory allocation,
 *          thus the contents should @b not be freed.
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_list_vset()
 * @see eina_value_list_get()
 * @see eina_value_list_pget()
 *
 */
static inline Eina_Bool eina_value_list_vget(const Eina_Value *value,
                                              unsigned int position,
                                              va_list args) EINA_ARG_NONNULL(1);
/**
 * @brief Inserts a generic value at a list member position.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 * @see eina_value_list_set()
 * @see eina_value_list_get()
 * @see eina_value_list_vset()
 * @see eina_value_list_pset()
 * @see eina_value_list_insert()
 * @see eina_value_list_pinsert()
 * @see eina_value_list_append()
 * @see eina_value_list_vappend()
 * @see eina_value_list_pappend()
 */
static inline Eina_Bool eina_value_list_vinsert(Eina_Value *value,
                                                unsigned int position,
                                                va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Appends a generic value to a list.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The source value object
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 * @see eina_value_list_set()
 * @see eina_value_list_get()
 * @see eina_value_list_vget()
 * @see eina_value_list_pset()
 * @see eina_value_list_insert()
 * @see eina_value_list_vinsert()
 * @see eina_value_list_pinsert()
 * @see eina_value_list_append()
 * @see eina_value_list_pappend()
 *
 */
static inline Eina_Bool eina_value_list_vappend(Eina_Value *value,
                                                 va_list args) EINA_ARG_NONNULL(1);


/**
 * @brief Sets a generic value for a list member from a pointer.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @remarks The pointer contents are written using the size defined by
 *          type. It can be larger than void* or uint64_t.
 *
 * @code
 *     Eina_Value *value = eina_value_list_new(EINA_VALUE_TYPE_INT);
 *     int x = 1234;
 *
 *     eina_value_list_append(value, 1234);
 *     eina_value_list_pset(value, 0, &x);
 *     eina_value_list_pget(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @param[in] ptr A pointer to specify the contents
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_list_set()
 * @see eina_value_list_get()
 * @see eina_value_list_vset()
 * @see eina_value_list_insert()
 * @see eina_value_list_vinsert()
 * @see eina_value_list_pinsert()
 * @see eina_value_list_append()
 * @see eina_value_list_vappend()
 * @see eina_value_list_pappend()
 *
 */
static inline Eina_Bool eina_value_list_pset(Eina_Value *value,
                                              unsigned int position,
                                              const void *ptr) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Gets a generic value into a pointer from a list member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in pointer contents, the actual value is
 *          type-dependent, but usually it is what is stored inside the
 *          object. There shouldn't be any memory allocation, thus the contents
 *          should @b not be freed.
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @code
 *     Eina_Value *value = eina_value_list_new(EINA_VALUE_TYPE_INT);
 *     int x;
 *
 *     eina_value_list_append(value, 1234);
 *     eina_value_list_pget(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @param[in] ptr A pointer to receive the contents
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_list_set()
 * @see eina_value_list_vset()
 * @see eina_value_list_pset()
 *
 */
static inline Eina_Bool eina_value_list_pget(const Eina_Value *value,
                                              unsigned int position,
                                              void *ptr) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Inserts a generic value at a list member position from a pointer.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @remarks The pointer contents are written using the size defined by
 *          type. It can be larger than void* or uint64_t.
 *
 * @code
 *     Eina_Value *value = eina_value_list_new(EINA_VALUE_TYPE_INT);
 *     int x = 1234;
 *
 *     eina_value_list_pinsert(value, 0, &x);
 *     eina_value_list_pget(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] position The index of the member
 * @param[in] ptr A pointer to specify the contents
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_list_set()
 * @see eina_value_list_get()
 * @see eina_value_list_vset()
 * @see eina_value_list_insert()
 * @see eina_value_list_vinsert()
 * @see eina_value_list_pinsert()
 * @see eina_value_list_append()
 * @see eina_value_list_vappend()
 * @see eina_value_list_pappend()
 *
 */
static inline Eina_Bool eina_value_list_pinsert(Eina_Value *value,
                                                 unsigned int position,
                                                 const void *ptr) EINA_ARG_NONNULL(1);

/**
 * @brief Appends a generic value to a list from a pointer.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_LIST: Eina_Value_List*
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @remarks The pointer contents are written using the size defined by
 *          type. It can be larger than void* or uint64_t.
 *
 * @code
 *     Eina_Value *value = eina_value_list_new(EINA_VALUE_TYPE_INT);
 *     int x = 1234;
 *
 *     eina_value_list_pappend(value, &x);
 *     eina_value_list_pget(value, 0, &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] ptr A pointer to specify the contents
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_list_set()
 * @see eina_value_list_get()
 * @see eina_value_list_vset()
 * @see eina_value_list_insert()
 * @see eina_value_list_vinsert()
 * @see eina_value_list_pinsert()
 * @see eina_value_list_append()
 * @see eina_value_list_vappend()
 * @see eina_value_list_pappend()
 */
static inline Eina_Bool eina_value_list_pappend(Eina_Value *value,
                                                 const void *ptr) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Eina_Value_Hash_Group Generic Value Hash management
 *
 * @{
 */

/**
 * @typedef Eina_Value_Hash
 * @brief The structure type containing the value type for #EINA_VALUE_TYPE_HASH.
 *
 * @see #_Eina_Value_Hash explains fields.
 * @since 1.2
 *
 * @since_tizen 2.3
 */
typedef struct _Eina_Value_Hash Eina_Value_Hash;

/**
 * @internal
 * @struct _Eina_Value_Hash
 * @brief The structure type used to store the hash and its subtype.
 * @since 1.2
 */
struct _Eina_Value_Hash
{
   const Eina_Value_Type *subtype; /**< How to allocate and access items */
   unsigned int buckets_power_size; /**< How to allocate hash buckets, if zero a default value is chosen */
   Eina_Hash *hash; /**< The hash that holds data, members are of subtype->value_size bytes */
};

/**
 * @brief Creates a generic value storage of type hash.
 *
 * @details This creates a new generic value storage of type hash. The members are
 *          managed using the description specified by @a subtype.
 *
 *          On failure, @c NULL is returned and #EINA_ERROR_OUT_OF_MEMORY
 *          or #EINA_ERROR_VALUE_FAILED is set.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks This creates from the mempool and then uses
 *          eina_value_hash_setup().
 *
 * @param[in] subtype The subtype to manage these hash members
 * @param[in] buckets_power_size The buckets power size based on which to allocate hash buckets (2 ^
 *                           buckets_power_size), if zero then a default value is chosen
 * @return The new value, otherwise @c NULL on failure
 *
 * @see eina_value_free()
 * @see eina_value_hash_setup()
 *
 */
EAPI Eina_Value *eina_value_hash_new(const Eina_Value_Type *subtype, unsigned int buckets_power_size) EINA_ARG_NONNULL(1);

/**
 * @brief Initializes a generic value storage of type hash.
 *
 * @details This initializes a new generic value storage of type hash with the given
 *          @a subtype.
 *
 *          This is same as calling eina_value_set()
 *          with #EINA_VALUE_TYPE_HASH followed by eina_value_pset() with
 *          the #Eina_Value_Hash description configured.
 *
 *          On failure, #EINA_FALSE is returned and #EINA_ERROR_OUT_OF_MEMORY
 *          or #EINA_ERROR_VALUE_FAILED is set.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks Existing contents are ignored. If the value is previously used, then
 *          use eina_value_flush() first.
 *
 * @param[in] value The value object
 * @param[in] subtype The subtype to manage these hash members
 * @param[in] buckets_power_size The buckets power size based on which to allocate hash buckets (2 ^
 *                           buckets_power_size), if zero then a default value is chosen
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_flush()
 */
static inline Eina_Bool eina_value_hash_setup(Eina_Value *value,
                                              const Eina_Value_Type *subtype,
                                              unsigned int buckets_power_size) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Queries the number of elements in a value of hash type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The value object
 * @return The number of child elements
 */
static inline unsigned int eina_value_hash_population(const Eina_Value *value);

/**
 * @brief Removes the element at the given position in a value of hash type.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The value object
 * @param[in] key The key to find the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
static inline Eina_Bool eina_value_hash_del(Eina_Value *value,
                                            const char *key) EINA_ARG_NONNULL(1);

/**
 * @brief Sets a generic value for a hash member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The variable argument is dependent on the chosen subtype. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char
 * @li EINA_VALUE_TYPE_USHORT: unsigned short
 * @li EINA_VALUE_TYPE_UINT: unsigned int
 * @li EINA_VALUE_TYPE_ULONG: unsigned long
 * @li EINA_VALUE_TYPE_UINT64: uint64_t
 * @li EINA_VALUE_TYPE_CHAR: char
 * @li EINA_VALUE_TYPE_SHORT: short
 * @li EINA_VALUE_TYPE_INT: int
 * @li EINA_VALUE_TYPE_LONG: long
 * @li EINA_VALUE_TYPE_INT64: int64_t
 * @li EINA_VALUE_TYPE_FLOAT: float
 * @li EINA_VALUE_TYPE_DOUBLE: double
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char *
 * @li EINA_VALUE_TYPE_STRING: const char *
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @code
 *     Eina_Value *value = eina_value_hash_new(EINA_VALUE_TYPE_INT, 0);
 *     int x;
 *
 *     eina_value_hash_set(value, "abc", 5678);
 *     eina_value_hash_get(value, "abc", &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] key The key to find the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_hash_get()
 * @see eina_value_hash_vset()
 * @see eina_value_hash_pset()
 * @see eina_value_hash_del()
 *
 */
static inline Eina_Bool eina_value_hash_set(Eina_Value *value,
                                            const char *key,
                                            ...) EINA_ARG_NONNULL(1);

/**
 * @brief Gets a generic value from a hash member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in the variable argument parameter, the
 *          actual value is type-dependent, but usually it is what is
 *          stored inside the object. There shouldn't be any memory allocation,
 *          thus the contents should @b not be freed.
 *
 * @remarks The variable argument is dependent on the chosen subtype. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @code
 *     Eina_Value *value = eina_value_hash_new(EINA_VALUE_TYPE_INT, 0);
 *     int x;
 *
 *     eina_value_hash_set(value, "abc", 1234);
 *     eina_value_hash_get(value, "abc", &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] key The key to find the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_hash_set()
 * @see eina_value_hash_vset()
 * @see eina_value_hash_pset()
 */
static inline Eina_Bool eina_value_hash_get(const Eina_Value *value,
                                            const char *key,
                                            ...) EINA_ARG_NONNULL(1);

/**
 * @brief Sets a generic value for a hash member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The source value object
 * @param[in] key The key to find the member
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 * @see eina_value_hash_set()
 * @see eina_value_hash_get()
 * @see eina_value_hash_pset()
 */
static inline Eina_Bool eina_value_hash_vset(Eina_Value *value,
                                             const char *key,
                                             va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Gets a generic value from a hash member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in the variable argument parameter, the
 *          actual value is type-dependent, but usually it is what is
 *          stored inside the object. There shouldn't be any memory allocation,
 *          thus the contents should @b not be freed.
 *
 * @param[in] value The source value object
 * @param[in] key The key to find the member
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_hash_vset()
 * @see eina_value_hash_get()
 * @see eina_value_hash_pget()
 *
 */
static inline Eina_Bool eina_value_hash_vget(const Eina_Value *value,
                                             const char *key,
                                             va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Sets a generic value for a hash member from a pointer.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @remarks  The pointer contents are written using the size defined by
 *           type. It can be larger than void* or uint64_t.
 *
 * @code
 *     Eina_Value *value = eina_value_hash_new(EINA_VALUE_TYPE_INT, 0);
 *     int x = 1234;
 *
 *     eina_value_hash_pset(value, "abc", &x);
 *     eina_value_hash_pget(value, "abc", &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] key The key to find the member
 * @param[in] ptr A pointer to specify the contents
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_hash_set()
 * @see eina_value_hash_get()
 * @see eina_value_hash_vset()
 */
static inline Eina_Bool eina_value_hash_pset(Eina_Value *value,
                                             const char *key,
                                             const void *ptr) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Gets a generic value into a pointer from a hash member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in pointer contents, the actual value is
 *          type-dependent, but usually it is what is stored inside the
 *          object. There shouldn't be any memory allocation, thus the contents
 *          should @b not be freed.
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @code
 *     Eina_Value *value = eina_value_hash_new(EINA_VALUE_TYPE_INT, 0);
 *     int x;
 *
 *     eina_value_hash_set(value, "abc", 1234);
 *     eina_value_hash_pget(value, "abc", &x);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] key The key to find the member
 * @param[in] ptr A pointer to receive the contents
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_hash_set()
 * @see eina_value_hash_vset()
 * @see eina_value_hash_pset()
 */
static inline Eina_Bool eina_value_hash_pget(const Eina_Value *value,
                                             const char *key,
                                             void *ptr) EINA_ARG_NONNULL(1, 3);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Eina_Value_Blob_Group Generic Value Blob management
 *
 * @{
 */

/**
 * @typedef Eina_Value_Blob_Operations
 * @brief The structure type for the operations used to manage a blob. Any @c NULL callback is ignored.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @see #_Eina_Value_Blob_Operations explains fields.
 */
typedef struct _Eina_Value_Blob_Operations Eina_Value_Blob_Operations;

/**
 * @def EINA_VALUE_BLOB_OPERATIONS_VERSION
 * @brief Definition of the current API version, used to validate #_Eina_Value_Blob_Operations.
 */
#define EINA_VALUE_BLOB_OPERATIONS_VERSION (1)

/**
 * @internal
 * @struct _Eina_Value_Blob_Operations
 * @brief The structure type for the operations used to manage a blob. Any @c NULL callback is ignored.
 * @since 1.2
 *
 * @since_tizen 2.3
 */
struct _Eina_Value_Blob_Operations
{
   unsigned int version; /**< Must be #EINA_VALUE_BLOB_OPERATIONS_VERSION */
   void (*free)(const Eina_Value_Blob_Operations *ops, void *memory, size_t size);
   void *(*copy)(const Eina_Value_Blob_Operations *ops, const void *memory, size_t size);
   int (*compare)(const Eina_Value_Blob_Operations *ops, const void *data1, size_t size_data1, const void *data2, size_t size_data2);
   char *(*to_string)(const Eina_Value_Blob_Operations *ops, const void *memory, size_t size);
};

/**
 * @var EINA_VALUE_BLOB_OPERATIONS_MALLOC
 *
 * @brief Assumes that @c memory is created with malloc() and applies free() to it
 *        during flush (Eina_Value_Blob_Operations::free). Copy is done with
 *        malloc() as well.
 *
 * @remarks No compare or to_string is provided, defaults are used.
 */
EAPI extern const Eina_Value_Blob_Operations *EINA_VALUE_BLOB_OPERATIONS_MALLOC;

/**
 * @typedef Eina_Value_Blob
 * @brief The structure type containing the value type for #EINA_VALUE_TYPE_BLOB.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @see #_Eina_Value_Blob explains fields.
 */
typedef struct _Eina_Value_Blob Eina_Value_Blob;

/**
 * @internal
 * @struct _Eina_Value_Blob
 * @brief The structure type used to store blob information and management operations.
 * @since 1.2
 *
 * @since_tizen 2.3
 */
struct _Eina_Value_Blob
{
   const Eina_Value_Blob_Operations *ops; /**< If @c NULL, nothing is freed, copy just copies the memory pointer, not its value */
   const void *memory;
   unsigned int size;
};

/**
 * @}
 */

/**
 * @internal
 * @defgroup Eina_Value_Struct_Group Generic Value Struct management
 *
 * @{
 */

/**
 * @typedef Eina_Value_Struct_Operations
 * @brief The structure type for the operations used to manage a struct. Any @c NULL callback is ignored.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks A structure can specify alternative methods to allocate, free, and
 *          copy itself. See structure definition for all methods.
 *
 * @see #_Eina_Value_Struct_Operations explains fields.
 */
typedef struct _Eina_Value_Struct_Operations Eina_Value_Struct_Operations;

/**
 * @typedef Eina_Value_Struct_Member
 * @brief The structure type that describes a single member of struct.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The member holds a name, type, and its byte offset within the struct
 *          memory. Most Eina_Value_Struct functions take the member name as
 *          a parameter, for example eina_value_struct_set().
 *
 * @see #_Eina_Value_Struct_Member explains fields.
 */
typedef struct _Eina_Value_Struct_Member Eina_Value_Struct_Member;

/**
 * @typedef Eina_Value_Struct_Desc
 * @brief The structure type that describes the struct by listing its size, members, and operations.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @see #_Eina_Value_Struct_Desc explains fields.
 */
typedef struct _Eina_Value_Struct_Desc Eina_Value_Struct_Desc;

/**
 * @typedef Eina_Value_Struct
 * @brief The structure type containing the value type for #EINA_VALUE_TYPE_STRUCT.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @see #_Eina_Value_Struct explains fields.
 */
typedef struct _Eina_Value_Struct Eina_Value_Struct;

/**
 * @def EINA_VALUE_STRUCT_OPERATIONS_VERSION
 * @brief Definition of the current API version, used to validate #_Eina_Value_Struct_Operations.
 */
#define EINA_VALUE_STRUCT_OPERATIONS_VERSION (1)

/**
 * @internal
 * @struct _Eina_Value_Struct_Operations
 * @brief The structure type for the operations used to manage a struct. Any @c NULL callback is ignored.
 * @since 1.2
 *
 * @since_tizen 2.3
 */
struct _Eina_Value_Struct_Operations
{
   unsigned int version; /**< must be #EINA_VALUE_STRUCT_OPERATIONS_VERSION */
   void *(*alloc)(const Eina_Value_Struct_Operations *ops, const Eina_Value_Struct_Desc *desc); /**< How to allocate struct memory to be managed by Eina_Value */
   void (*free)(const Eina_Value_Struct_Operations *ops, const Eina_Value_Struct_Desc *desc, void *memory); /**< How to release memory managed by Eina_Value */
   void *(*copy)(const Eina_Value_Struct_Operations *ops, const Eina_Value_Struct_Desc *desc, const void *memory); /**< How to copy struct memory from an existing Eina_Value, if not provided, alloc() is used and then every member is copied using eina_value_type_copy() with the member's type */
   int (*compare)(const Eina_Value_Struct_Operations *ops, const Eina_Value_Struct_Desc *desc, const void *data1, const void *data2); /**< How to compare two struct memories */
   const Eina_Value_Struct_Member *(*find_member)(const Eina_Value_Struct_Operations *ops, const Eina_Value_Struct_Desc *desc, const char *name); /**< How to find a description for a member. For huge structures consider using binary search, stringshared, hash, or gperf. The default function does linear search using strcmp() */
};

/**
 * @var EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH
 *
 * @brief Assumes that @c members is sorted by name and applies binary search for
 *        names.
 *
 * @remarks Ideally the @c member_count field is set to speed it up.
 *
 *          If no other methods are set (alloc, free, copy, compare), then it uses
 *          the default operations.
 */
EAPI extern const Eina_Value_Struct_Operations *EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH;

/**
 * @var EINA_VALUE_STRUCT_OPERATIONS_STRINGSHARE
 *
 * @brief Assumes that @c members name is stringshared and can be compared for
 *        equality without using its contents (simple pointer comparison).
 *
 * @remarks Ideally the search @c name is stringshared as well, but it
 *          does a second loop with a forced stringshare if it does not find
 *          the member.
 *
 *          If no other methods are set (alloc, free, copy, compare), then it uses
 *          the default operations.
 */
EAPI extern const Eina_Value_Struct_Operations *EINA_VALUE_STRUCT_OPERATIONS_STRINGSHARE;

/**
 * @internal
 * @struct _Eina_Value_Struct_Member
 * @brief The structure type that describes a single member of struct.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The name is used to lookup the member description. This is done as
 *          specified by _Eina_Value_Struct_Operations::find_member(). For
 *          structures with huge number of members, consider using a better
 *          find_member function to quickly find it. There are two helper
 *          operations provided to help this: #EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH
 *          and #EINA_VALUE_STRUCT_OPERATIONS_STRINGSHARE, both depend on properly
 *          set #_Eina_Value_Struct_Desc and #_Eina_Value_Struct_Member.
 *
 * @see #EINA_VALUE_STRUCT_MEMBER
 * @see #EINA_VALUE_STRUCT_MEMBER_SENTINEL
 */
struct _Eina_Value_Struct_Member
{
   const char *name; /**< Member name, used in lookups such as eina_value_struct_get() */
   const Eina_Value_Type *type; /**< How to use this member */
   unsigned int offset; /**< Where is this member located within the structure memory */
};

/**
 * @def EINA_VALUE_STRUCT_DESC_VERSION
 * @brief Definition of the current API version, used to validate #_Eina_Value_Struct_Desc.
 */
#define EINA_VALUE_STRUCT_DESC_VERSION (1)

/**
 * @internal
 * @struct _Eina_Value_Struct_Desc
 * @brief The structure type that describes the struct by listing its size, members, and operations.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks This is the root of Eina_Value knowledge about the memory it's
 *          handling as a structure. It adds introspection specifying the byte
 *          size of the structure, its members, and how to manage such members.
 *
 */
struct _Eina_Value_Struct_Desc
{
   unsigned int version; /**< Must be #EINA_VALUE_STRUCT_DESC_VERSION */
   const Eina_Value_Struct_Operations *ops; /**< Operations, if @c NULL defaults are used. You may use operations to optimize a member lookup using binary search or gperf hash */
   const Eina_Value_Struct_Member *members; /**< Array of member descriptions, if @c member_count is zero, then it must be @c NULL terminated */
   unsigned int member_count; /**< If > 0, it specifies the number of members. If zero then @c members must be @c NULL terminated */
   unsigned int size; /**< The byte size to allocate, may be bigger than the sum of the members */
};

/**
 * @def EINA_VALUE_STRUCT_MEMBER
 *
 * @brief Definition of the helper used to define Eina_Value_Struct_Member fields that use offsetof()
 *        with type and member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
#define EINA_VALUE_STRUCT_MEMBER(eina_value_type, type, member) \
   {#member, eina_value_type, offsetof(type, member)}

/**
 * @def EINA_VALUE_STRUCT_MEMBER_SENTINEL
 *
 * @brief Definition of the helper to define Eina_Value_Struct_Member fields for a sentinel (last
 *        item), that are useful if you did not define @c member_count.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
#define EINA_VALUE_STRUCT_MEMBER_SENTINEL {NULL, NULL, 0}


/**
 * @internal
 * @struct _Eina_Value_Struct
 * @brief The structure type used to store the memory and its description.
 * @since 1.2
 */
struct _Eina_Value_Struct
{
   const Eina_Value_Struct_Desc *desc; /**< How to manage the structure */
   void *memory; /**< The managed structure memory */
};

/**
 * @brief Creates a generic value storage of type struct.
 *
 * @details This creates a new generic value storage of type struct. The members are
 *          managed using the description specified by @a desc.
 *
 *          On failure, @c NULL is returned and #EINA_ERROR_OUT_OF_MEMORY
 *          or #EINA_ERROR_VALUE_FAILED is set.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks This creates from the mempool and then uses
 *          eina_value_struct_setup().
 *
 * @param[in] desc The description used to manage these struct members
 * @return The new value, otherwise @c NULL on failure
 *
 * @see eina_value_free()
 * @see eina_value_struct_setup()
 */
EAPI Eina_Value *eina_value_struct_new(const Eina_Value_Struct_Desc *desc) EINA_ARG_NONNULL(1);

/**
 * @brief Initializes a generic value storage of type struct.
 *
 * @details This initializes a new generic value storage of type struct with the given
 *          @a desc.
 *
 *          This is same as calling eina_value_set()
 *          with #EINA_VALUE_TYPE_STRUCT followed by eina_value_pset() with
 *          the #Eina_Value_Struct description configured.
 *
 *          On failure, #EINA_FALSE is returned and #EINA_ERROR_OUT_OF_MEMORY
 *          or #EINA_ERROR_VALUE_FAILED is set.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks Existing contents are ignored. If the value is previously used, then
 *          use eina_value_flush() first.
 *
 * @param[in] value The value object
 * @param[in] desc The description used to manage these struct members
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_flush()
 */
static inline Eina_Bool eina_value_struct_setup(Eina_Value *value,
                                                const Eina_Value_Struct_Desc *desc) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Sets a generic value for an struct member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The variable argument is dependent on the chosen member type. The list
 *          of basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char
 * @li EINA_VALUE_TYPE_USHORT: unsigned short
 * @li EINA_VALUE_TYPE_UINT: unsigned int
 * @li EINA_VALUE_TYPE_ULONG: unsigned long
 * @li EINA_VALUE_TYPE_UINT64: uint64_t
 * @li EINA_VALUE_TYPE_CHAR: char
 * @li EINA_VALUE_TYPE_SHORT: short
 * @li EINA_VALUE_TYPE_INT: int
 * @li EINA_VALUE_TYPE_LONG: long
 * @li EINA_VALUE_TYPE_INT64: int64_t
 * @li EINA_VALUE_TYPE_FLOAT: float
 * @li EINA_VALUE_TYPE_DOUBLE: double
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char *
 * @li EINA_VALUE_TYPE_STRING: const char *
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @code
 *     struct myst {
 *         int i;
 *         char c;
 *     };
 *     const Eina_Value_Struct_Member myst_members[] = {
 *         {"i", EINA_VALUE_TYPE_INT, 0},
 *         {"c", EINA_VALUE_TYPE_CHAR, 4},
 *         {NULL, NULL, 0}
 *     };
 *     const Eina_Value_Struct_Desc myst_desc = {
 *         EINA_VALUE_STRUCT_DESC_VERSION,
 *         NULL, myst_members, 2, sizeof(struct myst)
 *     };
 *     Eina_Value *value = eina_value_struct_new(&my_desc);
 *     int x;
 *     char y;
 *
 *     eina_value_struct_set(value, "i", 5678);
 *     eina_value_struct_get(value, "i", &x);
 *     eina_value_struct_set(value, "c", 0xf);
 *     eina_value_struct_get(value, "c", &y);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] name The name used to find the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_struct_get()
 * @see eina_value_struct_vset()
 * @see eina_value_struct_pset()
 */
static inline Eina_Bool eina_value_struct_set(Eina_Value *value,
                                              const char *name,
                                              ...) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Gets a generic value from a struct member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in the variable argument parameter, the
 *          actual value is type-dependent, but usually it is what is
 *          stored inside the object. There shouldn't be any memory allocation,
 *          thus the contents should @b not be freed.
 *
 * @remarks The variable argument is dependent on the chosen member type. The list
 *          of basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @code
 *     struct myst {
 *         int i;
 *         char c;
 *     };
 *     const Eina_Value_Struct_Member myst_members[] = {
 *         {"i", EINA_VALUE_TYPE_INT, 0},
 *         {"c", EINA_VALUE_TYPE_CHAR, 4},
 *         {NULL, NULL, 0}
 *     };
 *     const Eina_Value_Struct_Desc myst_desc = {
 *         EINA_VALUE_STRUCT_DESC_VERSION,
 *         NULL, myst_members, 2, sizeof(struct myst)
 *     };
 *     Eina_Value *value = eina_value_struct_new(&my_desc);
 *     int x;
 *     char y;
 *
 *     eina_value_struct_set(value, "i", 5678);
 *     eina_value_struct_get(value, "i", &x);
 *     eina_value_struct_set(value, "c", 0xf);
 *     eina_value_struct_get(value, "c", &y);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] name The name used to find the member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_struct_set()
 * @see eina_value_struct_vset()
 * @see eina_value_struct_pset()
 */
static inline Eina_Bool eina_value_struct_get(const Eina_Value *value,
                                              const char *name,
                                              ...) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Sets a generic value for a struct member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] value The source value object
 * @param[in] name The name used to find the member
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 * @see eina_value_struct_set()
 * @see eina_value_struct_get()
 * @see eina_value_struct_pset()
 */
static inline Eina_Bool eina_value_struct_vset(Eina_Value *value,
                                               const char *name,
                                               va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Gets a generic value from a struct member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in the variable argument parameter, the
 *          actual value is type-dependent, but usually it is what is
 *          stored inside the object. There shouldn't be any memory allocation,
 *          thus the contents should @b not be freed.
 *
 * @param[in] value The source value object
 * @param[in] name The name used to find the member
 * @param[in] args The variable argument
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_struct_vset()
 * @see eina_value_struct_get()
 * @see eina_value_struct_pget()
 */
static inline Eina_Bool eina_value_struct_vget(const Eina_Value *value,
                                               const char *name,
                                               va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Sets a generic value for a struct member from a pointer.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @remarks The pointer contents are written using the size defined by
 *          type. It can be larger than void* or uint64_t.
 *
 * @code
 *     struct myst {
 *         int i;
 *         char c;
 *     };
 *     const Eina_Value_Struct_Member myst_members[] = {
 *         {"i", EINA_VALUE_TYPE_INT, 0},
 *         {"c", EINA_VALUE_TYPE_CHAR, 4},
 *         {NULL, NULL, 0}
 *     };
 *     const Eina_Value_Struct_Desc myst_desc = {
 *         EINA_VALUE_STRUCT_DESC_VERSION,
 *         NULL, myst_members, 2, sizeof(struct myst)
 *     };
 *     Eina_Value *value = eina_value_struct_new(&my_desc);
 *     int x = 5678;
 *     char y = 0xf;
 *
 *     eina_value_struct_pset(value, "i", &);
 *     eina_value_struct_pget(value, "i", &x);
 *     eina_value_struct_pset(value, "c", &y);
 *     eina_value_struct_pget(value, "c", &y);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] name The name used to find the member
 * @param[in] ptr A pointer to specify the contents
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_struct_set()
 * @see eina_value_struct_get()
 * @see eina_value_struct_vset()
 */
static inline Eina_Bool eina_value_struct_pset(Eina_Value *value,
                                               const char *name,
                                               const void *ptr) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Gets a generic value into pointer from a struct member.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The value is returned in pointer contents, the actual value is
 *          type-dependent, but usually it is what is stored inside the
 *          object. There shouldn't be any memory allocation, thus the contents
 *          should @b not be freed.
 *
 * @remarks The pointer type is dependent on the chosen value type. The list of
 *          basic types:
 *
 * @li EINA_VALUE_TYPE_UCHAR: unsigned char*
 * @li EINA_VALUE_TYPE_USHORT: unsigned short*
 * @li EINA_VALUE_TYPE_UINT: unsigned int*
 * @li EINA_VALUE_TYPE_ULONG: unsigned long*
 * @li EINA_VALUE_TYPE_UINT64: uint64_t*
 * @li EINA_VALUE_TYPE_CHAR: char*
 * @li EINA_VALUE_TYPE_SHORT: short*
 * @li EINA_VALUE_TYPE_INT: int*
 * @li EINA_VALUE_TYPE_LONG: long*
 * @li EINA_VALUE_TYPE_INT64: int64_t*
 * @li EINA_VALUE_TYPE_FLOAT: float*
 * @li EINA_VALUE_TYPE_DOUBLE: double*
 * @li EINA_VALUE_TYPE_STRINGSHARE: const char **
 * @li EINA_VALUE_TYPE_STRING: const char **
 * @li EINA_VALUE_TYPE_HASH: Eina_Value_Hash*
 * @li EINA_VALUE_TYPE_TIMEVAL: struct timeval*
 * @li EINA_VALUE_TYPE_BLOB: Eina_Value_Blob*
 * @li EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct*
 *
 * @code
 *     struct myst {
 *         int i;
 *         char c;
 *     };
 *     const Eina_Value_Struct_Member myst_members[] = {
 *         {"i", EINA_VALUE_TYPE_INT, 0},
 *         {"c", EINA_VALUE_TYPE_CHAR, 4},
 *         {NULL, NULL, 0}
 *     };
 *     const Eina_Value_Struct_Desc myst_desc = {
 *         EINA_VALUE_STRUCT_DESC_VERSION,
 *         NULL, myst_members, 2, sizeof(struct myst)
 *     };
 *     Eina_Value *value = eina_value_struct_new(&my_desc);
 *     int x = 5678;
 *     char y = 0xf;
 *
 *     eina_value_struct_pset(value, "i", &);
 *     eina_value_struct_pget(value, "i", &x);
 *     eina_value_struct_pset(value, "c", &y);
 *     eina_value_struct_pget(value, "c", &y);
 *     eina_value_free(value);
 * @endcode
 *
 * @param[in] value The source value object
 * @param[in] name The name used to find the member
 * @param[in] ptr A pointer to receive the contents
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 * @see eina_value_struct_set()
 * @see eina_value_struct_vset()
 * @see eina_value_struct_pset()
 */
static inline Eina_Bool eina_value_struct_pget(const Eina_Value *value,
                                               const char *name,
                                               void *ptr) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Gets a member as an Eina_Value copy.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The argument @a dst is considered as uninitialized and it's setup to
 *          the type of the member.
 *
 * @param[in] src The source value object
 * @param[in] name The name used to find the member
 * @param[in] dst An object used to return the member value
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
static inline Eina_Bool eina_value_struct_value_get(const Eina_Value *src,
                                                    const char *name,
                                                    Eina_Value *dst) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Sets a member from an Eina_Value source.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] dst The destination value object
 * @param[in] name The name used to find the member
 * @param[in] src The source value
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure.
 *
 */
static inline Eina_Bool eina_value_struct_value_set(Eina_Value *dst,
                                                    const char *name,
                                                    const Eina_Value *src) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Gets a member as a Eina_Value copy given that its member description is provided.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The argument @a dst is considered as uninitialized and it's setup to
 *          the type of the member.
 *
 * @param[in] src The source value object
 * @param[in] member The member description to use
 * @param[in] dst An object used to return the member value
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
static inline Eina_Bool eina_value_struct_member_value_get(const Eina_Value *src,
                                                           const Eina_Value_Struct_Member *member,
                                                           Eina_Value *dst) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Sets a member from an Eina_Value source.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] dst The destination value object
 * @param[in] member The member description to use
 * @param[in] src The source value
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
static inline Eina_Bool eina_value_struct_member_value_set(Eina_Value *dst,
                                                           const Eina_Value_Struct_Member *member,
                                                           const Eina_Value *src) EINA_ARG_NONNULL(1, 2, 3);


/**
 * @}
 */


/**
 * @internal
 * @defgroup Eina_Value_Type_Group Generic Value Type management
 *
 * @{
 */

/**
 * @def EINA_VALUE_TYPE_VERSION
 * @brief Definition of the current API version, used to validate type.
 */
#define EINA_VALUE_TYPE_VERSION (1)

/**
 * @internal
 * @struct _Eina_Value_Type
 * @brief The structure type containing the API to access values.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
struct _Eina_Value_Type
{
   unsigned int version; /**< Must be #EINA_VALUE_TYPE_VERSION */
   unsigned int value_size; /**< Byte size of value */
   const char *name; /**< Name for debug and introspection */
   Eina_Bool (*setup)(const Eina_Value_Type *type, void *mem); /**< Mem is malloc(value_size) and should be configured */
   Eina_Bool (*flush)(const Eina_Value_Type *type, void *mem); /**< Clear any values from mem */
   Eina_Bool (*copy)(const Eina_Value_Type *type, const void *src, void *dst); /**< How to copy values, both memories are @c value_size */
   int (*compare)(const Eina_Value_Type *type, const void *a, const void *b); /**< How to compare values, both memories are @c value_size */
   Eina_Bool (*convert_to)(const Eina_Value_Type *type, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem); /**< How to convert values, both memories are @c value_size */
   Eina_Bool (*convert_from)(const Eina_Value_Type *type, const Eina_Value_Type *convert, void *type_mem, const void *convert_mem); /**< How to convert values, both memories are @c value_size */
   Eina_Bool (*vset)(const Eina_Value_Type *type, void *mem, va_list args); /**< How to set memory from a variable argument */
   Eina_Bool (*pset)(const Eina_Value_Type *type, void *mem, const void *ptr); /**< How to set memory from a pointer */
   Eina_Bool (*pget)(const Eina_Value_Type *type, const void *mem, void *ptr); /**< How to read memory */
};

/**
 * @brief Queries type name.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] type The type reference
 * @return A string, otherwise @c NULL if the type is invalid
 */
EAPI const char *eina_value_type_name_get(const Eina_Value_Type *type) EINA_PURE EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Checks whether the type is valid.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks A type is invalid if it's NULL or if the version field is not same
 *          as the runtime #EINA_VALUE_TYPE_VERSION.
 *
 * @param[in] type The type reference
 * @return @c EINA_TRUE if valid, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_value_type_check(const Eina_Value_Type *type) EINA_PURE EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Initializes the memory using a type descriptor.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] type The type reference
 * @param[in] mem The memory to operate, must be of size @c type->value_size
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
static inline Eina_Bool eina_value_type_setup(const Eina_Value_Type *type, void *mem);

/**
 * @brief Flushes (clears) the memory using a type descriptor.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] type The type reference
 * @param[in] mem The memory to operate, must be of size @c type->value_size
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
static inline Eina_Bool eina_value_type_flush(const Eina_Value_Type *type, void *mem);

/**
 * @brief Copies the memory using a type descriptor.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] type The type reference
 * @param[in] src The memory to operate, must be of size @c type->value_size
 * @param[in] dst The memory to operate, must be of size @c type->value_size
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
static inline Eina_Bool eina_value_type_copy(const Eina_Value_Type *type, const void *src, void *dst);

/**
 * @brief Compares the memory using a type descriptor.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] type The type reference
 * @param[in] a The memory to operate, must be of size @c type->value_size
 * @param[in] b The memory to operate, must be of size @c type->value_size
 * @return Less than zero if a < b, greater than zero if a > b, and zero if a = b
 */
static inline int eina_value_type_compare(const Eina_Value_Type *type, const void *a, const void *b);

/**
 * @brief Converts the memory using a type descriptor.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] type The type reference of the source
 * @param[in] convert The type reference of the destination
 * @param[in] type_mem The memory to operate, must be of size @c type->value_size
 * @param[in] convert_mem The memory to operate, must be of size @c convert->value_size
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
static inline Eina_Bool eina_value_type_convert_to(const Eina_Value_Type *type, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem);

/**
 * @brief Converts the memory using a type descriptor.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] type The type reference of the destination
 * @param[in] convert The type reference of the source
 * @param[in] type_mem The memory to operate, must be of size @c type->value_size
 * @param[in] convert_mem The memory to operate, must be of size @c convert->value_size
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
static inline Eina_Bool eina_value_type_convert_from(const Eina_Value_Type *type, const Eina_Value_Type *convert, void *type_mem, const void *convert_mem);

/**
 * @brief Sets the memory using a type descriptor and a variable argument.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] type The type reference of the source
 * @param[in] mem The memory to operate, must be of size @c type->value_size
 * @param[in] args The input value
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
static inline Eina_Bool eina_value_type_vset(const Eina_Value_Type *type, void *mem, va_list args);

/**
 * @brief Sets the memory using a type descriptor and a pointer.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] type The type reference of the source
 * @param[in] mem The memory to operate, must be of size @c type->value_size
 * @param[in] ptr A pointer to the input value.
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
static inline Eina_Bool eina_value_type_pset(const Eina_Value_Type *type, void *mem, const void *ptr);

/**
 * @brief Gets the memory using a type descriptor.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] type The type reference of the source
 * @param[in] mem The memory to operate, must be of size @c type->value_size
 * @param[in] ptr A pointer to the output
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
static inline Eina_Bool eina_value_type_pget(const Eina_Value_Type *type, const void *mem, void *ptr);

/**
 * @}
 */

#include "eina_inline_value.x"

/**
 * @}
 */

#endif
