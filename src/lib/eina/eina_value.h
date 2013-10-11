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

#ifndef EINA_VALUE_H_
#define EINA_VALUE_H_

#include <stdarg.h>

#include "eina_types.h"
#include "eina_fp.h" /* defines int64_t and uint64_t */
#include "eina_inarray.h"
#include "eina_list.h"
#include "eina_hash.h"

/**
 * @page eina_value_example_01_page Eina_Value usage
 * @dontinclude eina_value_01.c
 *
 * This very simple example shows how to use some of the basic features of eina
 * value: setting and getting values, converting between types and printing a
 * value as a string.
 *
 * Our main function starts out with the basic, declaring some variables and
 * initializing eina:
 * @until eina_init
 *
 * Now we can jump into using eina value. We set a value, get this value and
 * then print it:
 * @until printf
 *
 * In the above snippet of code we printed an @c int value, we can however print
 * the value as a string:
 * @until free
 *
 * And once done with a value it's good practice to destroy it:
 * @until eina_value_flush
 *
 * We now reuse @c v to store a string, get its value and print it:
 * @until printf
 * @note Since @c s is the value and not returned by @c eina_value_to_string()
 * we don't need to free it.
 *
 * Just because we stored a string doesn't mean we can't use the @c
 * eina_value_to_string() function, we can and it's important to note that it
 * will return not the stored string but rather a copy of it(one we have to
 * free):
 * @until eina_value_flush
 *
 * And now to explore conversions between two type we'll create another value:
 * @until eina_value_setup
 *
 * And make sure @c v and @c otherv have different types:
 * @until eina_value_setup
 *
 * We then set a value to @c v and have it converted, to do this we don't need
 * to tell to which type we want to convert, we just say were we want to store
 * the converted value and eina value will figure out what to convert to, and
 * how:
 * @until eina_value_convert
 *
 * And now let's check the conversion worked:
 * @until printf
 *
 * But converting to strings is not particularly exciting, @c
 * eina_value_to_string() already did that, so now let's make the conversion the
 * other way around, from string to @c int:
 * @until printf
 *
 * And once done, destroy the values:
 * @until }
 *
 * Full source code: @ref eina_value_01_c
 */

/**
 * @page eina_value_01_c eina_value_01.c
 * @include eina_value_01.c
 * @example eina_value_01.c
 */

/**
 * @page eina_value_example_02_page Eina_Value struct usage
 * @dontinclude eina_value_02.c
 *
 * This example will examine a hypothetical situation in which we had a
 * structure(which represented parameters) with two fields, and then need to add
 * a third field to our structure. If using structs directly we'd need to
 * rewrite every piece of code that touches the struct, by using eina value, and
 * thus having the compiler not even know the struct, we can reduce the amount
 * of changes needed and retain interoperability between the old and new format.
 *
 * Our example will start with a function that creates descriptions of both of
 * our structs for eina value usage. The first step is to create a struct and
 * describe its members:
 * @until v1_members[1]
 * @note We can't pass the types of the members to EINA_VALUE_STRUCT_MEMBER
 * macro because they are not constant initializers.
 *
 * So far it should be pretty easy to understand, we said @c My_Struct_V1 has
 * two members, one of type @c int and another of type @c char. We now create
 * the description of the actual struct, again nothing overly complex, we signal
 * which version of EINA_VALUE_STRUCT we're using, we declare no special
 * operations, our members and our size:
 * @until V1_DESC
 *
 * We now repeat the process for the second version of our struct, the only
 * difference is the addition of a third parameter of type @c int :
 * @until V2_DESC
 * @until }
 *
 * We'll now look at a function that sets the values of our structs. For
 * simplicity's sake we initialize it we random values, a real world case would
 * read these values from a file, a database or even from the network. The
 * fundamental detail here is that this function works for both V1 and V2
 * structs, this is because setting a parameter that a struct that doesn't have
 * does nothing without throwing any errors:
 * @until }
 * @note While using eina_value_struct_set() with an in-existing parameter
 * causes no error, it does return #EINA_FALSE, to notify it was not possible
 * to set the value. This could be used to determine that we're handling a V1
 * struct and take some action based on that.
 *
 * The next thing is to do is see what a function that uses the values of the
 * struct looks like. We'll again be very simplistic in our usage, we'll just
 * print the values, but a real world case, might send these values to another
 * process use them to open a network/database connection or anything else.
 * Since all versions of the struct have @c param1 and @c param2 we'll
 * unconditionally use them:
 * @until printf
 *
 * The next step is to conditionally use @c param3, which can fortunately be
 * done in the same step in which we get it's value:
 * @until }
 *
 * There we've now got functions that can both populate and use values from both
 * our structs, so now let's actually use them in our main function by creating
 * a struct of each type, initializing them and them using them:
 * @until }
 *
 * This concludes our example. For the full source code see @ref
 * eina_value_02_c.
 */

/**
 * @page eina_value_02_c eina_value_02.c
 * @include eina_value_02.c
 * @example eina_value_02.c
 */

/**
 * @page eina_value_example_03_page Eina value custom type example
 * @dontinclude eina_value_03.c
 *
 * For this example we'll be creating our own custom type of eina value. Eina
 * value can already store struct timeval(man gettimeofday for more information)
 * but it has no type to store struct timezone, so that's what this example will
 * do.
 * @note struct timezone is actually obsolete, so using it in real world
 * programs is probably not a good idea, but this is an example so, bear with
 * us.
 *
 * To create our own custom eina value type we need to define functions to
 * do the following operations on it:
 * @li Setup
 * @li Flush
 * @li Copy
 * @li Compare
 * @li Set
 * @li Get
 * @li Conversion
 *
 * Most of this functions are very simple, so let's look at them, starting with
 * setup which only clear the memory so that we can be certain we won't be using
 * stale data:
 * @until }
 *
 * Now the flush function, which is even simpler, it does nothing, that's
 * because there is nothing we need to do, all the necessary steps are taken by
 * eina value itself:
 * @until }
 *
 * Our next function, copy, is a bit more interesting, but not much, it just
 * casts our void pointers to struct timezone pointers and does the copy:
 * @until }
 * @note By now you might be wondering why our functions receive void pointers
 * instead of pointers to struct timezone, and this is a good point. The reason
 * for this is that eina value doesn't know anything about our type so it must
 * use a generic void pointer, casting that pointer into a proper value is the
 * job of the implementor of the new type.
 *
 * Next we have the comparison function, which compares the @c tz_minuteswest
 * field of struct timezone, we don't compare @c tz_dsttime because that field
 * is not used in linux:
 * @until }
 *
 * Next we have setting, this however requires not one but rather two functions,
 * the reason for this is because to be able to receive arguments of any type
 * eina value uses <a href="https://wikipedia.org/wiki/Variadic_functions">
 * variadic functions</a>, so we need a function to get the argument from a 
 * va_list and another to actually to the setting.
 *
 * Lets first look at the pset function which sets the received value to a
 * pointer:
 * @until }
 *
 * Next we have the vset function which get the argument from the va_list and
 * passes it to the pset function:
 * @until }
 *
 * And now the function to get the value, a very simple copying of the value to
 * the given pointer:
 * @until }
 *
 * And finally our conversion function, this is our longest and most interesting
 * one. For numeric type we simply assign the value of @c tz_minuteswest to the
 * new type and call a set function using it:
 * @until EINA_VALUE_TYPE_DOUBLE
 * @until return
 * @note It would be a good idea to add checks for over and underflow for these
 * types and return #EINA_FALSE in thoses cases, we omit this here for brevity.
 *
 * For string types we use @c snprintf() to format our @c tz_minuteswest field
 * and put it in a string(again @c tz_dsttime is ignored because it's not used):
 * @until }
 *
 * Finally we handle any other types by returning an error in that case:
 * @until }
 *
 * Now that we have all the functions, we can populate an @c Eina_Value_Type to
 * later use it with @c eina_value_setup():
 * @until }
 *
 * We can now finally use our new TZ_TYPE with eina value, so lets conclude our
 * example by practicing that by setting its value and printing it:
 * @until }
 *
 * For the full source code see @ref eina_value_03_c.
 */

/**
 * @page eina_value_03_c eina_value_03.c
 * @include eina_value_03.c
 * @example eina_value_03.c
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @since 1.2
 *
 * @{
 */

/**
 * @addtogroup Eina_Containers_Group Containers
 *
 * @{
 */

/**
 * @defgroup Eina_Value_Group Generic Value Storage
 *
 * Abstracts generic data storage and access to it in an extensible
 * and efficient way.
 *
 * It comes with pre-defined types for numbers, array, list, hash,
 * blob and structs. It is able to convert between data types,
 * including to string.
 *
 * It is meant for simple data types, providing uniform access and
 * release functions, useful to exchange data preserving their
 * types. For more complex hierarchical data, with properties and
 * children, reference counting, inheritance and interfaces, 
 *
 * Examples of usage of the Eina_Value API:
 * @li @ref eina_value_example_01_page
 * @li @ref eina_value_example_02_page
 * @li @ref eina_value_example_03_page
 *
 * @{
 */


/**
 * @typedef Eina_Value
 * Store generic values.
 *
 * @since 1.2
 */
typedef struct _Eina_Value Eina_Value;

/**
 * @typedef Eina_Value_Type
 * Describes the data contained by the value
 *
 * @since 1.2
 */
typedef struct _Eina_Value_Type Eina_Value_Type;

/**
 * @typedef Eina_Value_Union
 * Union of all known value types.
 *
 * This is only used to specify the minimum payload memory for #Eina_Value.
 *
 * @internal
 * @since 1.2
 */
typedef union _Eina_Value_Union Eina_Value_Union;

/**
 * @union _Eina_Value_Union
 * All possible value types.
 *
 * This is only used to specify the minimum payload memory for #Eina_Value.
 *
 * @internal
 * @since 1.2
 */
union _Eina_Value_Union
{
   unsigned char  buf[8];    /**< just hold 8-bytes, more goes into ptr */
   void          *ptr;       /**< used as generic pointer */
   uint64_t      _guarantee; /**< guarantees 8-byte alignment */
};

/**
 * @var EINA_VALUE_TYPE_UCHAR
 * manages unsigned char type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_UCHAR;

/**
 * @var EINA_VALUE_TYPE_USHORT
 * manages unsigned short type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_USHORT;

/**
 * @var EINA_VALUE_TYPE_UINT
 * manages unsigned int type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_UINT;

/**
 * @var EINA_VALUE_TYPE_ULONG
 * manages unsigned long type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_ULONG;

/**
 * @var EINA_VALUE_TYPE_TIMESTAMP
 * manages unsigned long type used for timestamps.
 * @note this is identical in function to EINA_VALUE_TYPE_ULONG
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_TIMESTAMP;

/**
 * @var EINA_VALUE_TYPE_UINT64
 * manages unsigned integer of 64 bits type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_UINT64;

/**
 * @var EINA_VALUE_TYPE_CHAR
 * manages char type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_CHAR;

/**
 * @var EINA_VALUE_TYPE_SHORT
 * manages short type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_SHORT;

/**
 * @var EINA_VALUE_TYPE_INT
 * manages int type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_INT;

/**
 * @var EINA_VALUE_TYPE_LONG
 * manages long type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_LONG;

/**
 * @var EINA_VALUE_TYPE_INT64
 * manages integer of 64 bits type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_INT64;

/**
 * @var EINA_VALUE_TYPE_FLOAT
 * manages float type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_FLOAT;

/**
 * @var EINA_VALUE_TYPE_DOUBLE
 * manages double type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_DOUBLE;

/**
 * @var EINA_VALUE_TYPE_STRINGSHARE
 * manages stringshared string type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_STRINGSHARE;

/**
 * @var EINA_VALUE_TYPE_STRING
 * manages string type.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_STRING;


/**
 * @var EINA_VALUE_TYPE_ARRAY
 *
 * manages array type. Use the value get/set for arrays:
 *  @li eina_value_array_get() and eina_value_array_set()
 *  @li eina_value_array_vget() and eina_value_array_vset()
 *  @li eina_value_array_pget() and eina_value_array_pset()
 *
 * eina_value_set() takes an #Eina_Value_Array where just @c subtype
 * and @c step are used. If there is an @c array, it will be copied
 * (including each item) and its contents must be properly
 * configurable as @c subtype expects. eina_value_pset() takes a
 * pointer to an #Eina_Value_Array.  For your convenience, use
 * eina_value_array_setup().
 *
 * eina_value_get() and eina_value_pget() takes a pointer
 * to #Eina_Value_Array, it's an exact copy of the current structure in
 * use by value, no copies are done.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_ARRAY;

/**
 * @var EINA_VALUE_TYPE_LIST
 *
 * manages list type. Use the value get/set for lists:
 *  @li eina_value_list_get() and eina_value_list_set()
 *  @li eina_value_list_vget() and eina_value_list_vset()
 *  @li eina_value_list_pget() and eina_value_list_pset()
 *
 * eina_value_set() takes an #Eina_Value_List where just @c subtype is
 * used. If there is an @c list, it will be copied (including each
 * item) and its contents must be properly configurable as @c
 * subtype expects. eina_value_pset() takes a pointer to an #Eina_Value_List.
 * For your convenience, use eina_value_list_setup().
 *
 * eina_value_get() and eina_value_pget() takes a pointer to #Eina_Value_List,
 * it's an exact copy of the current structure in use by value, no copies are
 * done.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_LIST;

/**
 * @var EINA_VALUE_TYPE_HASH
 *
 * manages hash type. Use the value get/set for hashes:
 *  @li eina_value_hash_get() and eina_value_hash_set()
 *  @li eina_value_hash_vget() and eina_value_hash_vset()
 *  @li eina_value_hash_pget() and eina_value_hash_pset()
 *
 * eina_value_set() takes an #Eina_Value_Hash where just @c subtype
 * and @c buckets_power_size are used. If there is an @c hash, it will
 * be copied (including each item) and its contents must be
 * properly configurable as @c subtype expects. eina_value_pset()
 * takes a pointer to an #Eina_Value_Hash.  For your convenience, use
 * eina_value_hash_setup().
 *
 * eina_value_get() and eina_value_pget() takes a pointer to #Eina_Value_Hash,
 * it's an exact copy of the current structure in use by value, no copies are
 * done.
 *
 * @note be aware that hash data is always an allocated memory of size
 *       defined by @c subtype->value_size. If your @c subtype is an
 *       integer, add as data malloc(sizeof(int)). If your @c subtype
 *       is an string, add as data malloc(sizeof(char*)) and this data
 *       value must point to strdup(string)!
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_HASH;

/**
 * @var EINA_VALUE_TYPE_TIMEVAL
 * manages 'struct timeval' type
 *
 * eina_value_set() takes a "struct timeval" from sys/time.h.
 * eina_value_pset() takes a pointer to "struct timeval".
 *
 * eina_value_get() and eina_value_pget() takes a pointer to "struct
 * timeval" and it's an exact copy of value.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_TIMEVAL;

/**
 * @var EINA_VALUE_TYPE_BLOB
 * manages blob of bytes type, see @ref Eina_Value_Blob
 *
 * eina_value_set() takes an #Eina_Value_Blob
 * eina_value_pset() takes a pointer to #Eina_Value_Blob.
 *
 * eina_value_get() and eina_value_pget() takes a pointer to #Eina_Value_Blob
 * and it's an exact copy of value, no allocations are made.
 *
 * Memory is untouched unless you provide @c ops (operations) pointer.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_BLOB;

/**
 * @var EINA_VALUE_TYPE_STRUCT
 *
 * manages struct type. Use the value get/set for structs:
 *  @li eina_value_struct_get() and eina_value_struct_set()
 *  @li eina_value_struct_vget() and eina_value_struct_vset()
 *  @li eina_value_struct_pget() and eina_value_struct_pset()
 *
 * eina_value_set() takes an #Eina_Value_Struct where just @c desc is
 * used. If there is an @c memory, it will be copied (including each
 * member) and its contents must be properly configurable as @c desc
 * expects. eina_value_pset() takes a pointer to an #Eina_Value_Struct. For
 * your convenience, use eina_value_struct_setup().
 *
 * eina_value_get() and eina_value_pget() takes a pointer
 * to #Eina_Value_Struct, it's an exact copy of the current structure in
 * use by value, no copies are done.
 *
 * @since 1.2
 */
EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_STRUCT;

EAPI extern Eina_Error EINA_ERROR_VALUE_FAILED;

/**
 * @defgroup Eina_Value_Value_Group Generic Value management
 *
 * @{
 */

/**
 * @struct _Eina_Value
 * defines the contents of a value
 *
 * @since 1.2
 */
struct _Eina_Value
{
   const Eina_Value_Type *type; /**< how to access values */
   Eina_Value_Union value; /**< to be accessed with type descriptor */
};

/**
 * @brief Create generic value storage.
 * @param type how to manage this value.
 * @return The new value or @c NULL on failure.
 *
 * Create a new generic value storage. The members are managed using
 * the description specified by @a type.
 *
 * Some types may specify more operations:
 * eg. #EINA_VALUE_TYPE_ARRAY uses eina_value_array_set(),
 * eina_value_array_get() and so on.
 *
 * On failure, @c NULL is returned.
 *
 * @note this calls creates from mempool and then uses
 *       eina_value_setup(). Consider using eina_value_flush() and
 *       eina_value_setup() instead to avoid memory allocations.
 *
 * @see eina_value_free()
 *
 * @since 1.2
 */
EAPI Eina_Value *eina_value_new(const Eina_Value_Type *type) EINA_ARG_NONNULL(1) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Free value and its data.
 * @param value value object
 *
 * @see eina_value_flush()
 *
 * @since 1.2
 */
EAPI void eina_value_free(Eina_Value *value) EINA_ARG_NONNULL(1);


/**
 * @brief Initialize generic value storage.
 * @param value value object
 * @param type how to manage this value.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * Initializes existing generic value storage. The members are managed using the
 * description specified by @a type.
 *
 * Some types may specify more operations, as an example #EINA_VALUE_TYPE_ARRAY
 * uses eina_value_array_set(), eina_value_array_get() and so on.
 *
 * @note Existing contents are ignored! If the value was previously used, then
 *       use eina_value_flush() first.
 *
 * On failure, #EINA_FALSE is returned.
 *
 * @see eina_value_flush()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_setup(Eina_Value *value,
                                         const Eina_Value_Type *type) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Create generic value storage.
 * @param value value object
 *
 * Releases all the resources associated with an #Eina_Value. The
 * value must be already set with eina_value_setup() or
 * eina_value_new().
 *
 * After this call returns, the contents of the value are undefined,
 * but the value can be reused by calling eina_value_setup() again.
 *
 * @see eina_value_setup()
 * @see eina_value_free()
 *
 * @since 1.2
 */
static inline void eina_value_flush(Eina_Value *value) EINA_ARG_NONNULL(1);

/**
 * @brief Copy generic value storage.
 * @param value source value object
 * @param copy destination value object
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The @a copy object is considered uninitialized and its existing
 * contents are overwritten (just as if eina_value_flush() was called on
 * it).
 *
 * The copy happens by calling eina_value_setup() on @a copy, followed
 * by getting the contents of @a value and setting it to @a copy.
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_value_copy(const Eina_Value *value,
                               Eina_Value *copy) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Compare generic value storage.
 * @param a left side of comparison
 * @param b right side of comparison
 * @return less than zero if a < b, greater than zero if a > b, zero
 *         if a == b
 *
 * @since 1.2
 */
static inline int eina_value_compare(const Eina_Value *a,
                                     const Eina_Value *b) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Set the generic value.
 * @param value source value object
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The variable argument is dependent on chosen type. The list for
 * basic types:
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
 * @note for array member see eina_value_array_set()
 * @note for list member see eina_value_list_set()
 * @note for hash member see eina_value_hash_set()
 *
 * @see eina_value_get()
 * @see eina_value_vset()
 * @see eina_value_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_set(Eina_Value *value,
                                       ...) EINA_ARG_NONNULL(1);

/**
 * @brief Get the generic value.
 * @param value source value object
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in the variable argument parameter, the
 * actual value is type-dependent, but usually it will be what is
 * stored inside the object. There shouldn't be any memory allocation,
 * thus the contents should @b not be freed.
 *
 * The variable argument is dependent on chosen type. The list for
 * basic types:
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
 * @note for array member see eina_value_array_get()
 * @note for list member see eina_value_list_get()
 * @note for hash member see eina_value_hash_get()
 *
 * @see eina_value_set()
 * @see eina_value_vset()
 * @see eina_value_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_get(const Eina_Value *value,
                                       ...) EINA_ARG_NONNULL(1);

/**
 * @brief Set the generic value.
 * @param value source value object
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @note for array member see eina_value_array_vset()
 * @note for list member see eina_value_list_vset()
 * @note for hash member see eina_value_hash_vset()
 *
 * @see eina_value_vget()
 * @see eina_value_set()
 * @see eina_value_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_vset(Eina_Value *value,
                                        va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Get the generic value.
 * @param value source value object
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in the variable argument parameter, the
 * actual value is type-dependent, but usually it will be what is
 * stored inside the object. There shouldn't be any memory allocation,
 * thus the contents should @b not be freed.
 *
 * @note for array member see eina_value_array_vget()
 * @note for list member see eina_value_list_vget()
 * @note for hash member see eina_value_hash_vget()
 *
 * @see eina_value_vset()
 * @see eina_value_get()
 * @see eina_value_pget()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_vget(const Eina_Value *value,
                                        va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Set the generic value from pointer.
 * @param value source value object
 * @param ptr pointer to specify the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @note the pointer contents are written using the size defined by
 *       type. It can be larger than void* or uint64_t.
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
 * @note for array member see eina_value_array_pset()
 * @note for list member see eina_value_list_pset()
 * @note for hash member see eina_value_hash_pset()
 *
 * @see eina_value_pget()
 * @see eina_value_set()
 * @see eina_value_vset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_pset(Eina_Value *value,
                                        const void *ptr) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Get the generic value to pointer.
 * @param value source value object
 * @param ptr pointer to receive the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in pointer contents, the actual value is
 * type-dependent, but usually it will be what is stored inside the
 * object. There shouldn't be any memory allocation, thus the contents
 * should @b not be freed.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @note for array member see eina_value_array_get()
 * @note for list member see eina_value_list_get()
 * @note for hash member see eina_value_hash_get()
 *
 * @see eina_value_set()
 * @see eina_value_vset()
 * @see eina_value_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_pget(const Eina_Value *value,
                                        void *ptr) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Convert one value to another type.
 * @param value source value object.
 * @param convert destination value object.
 * @return #EINA_TRUE if converted, #EINA_FALSE otherwise.
 *
 * Converts one value to another trying first @a value type
 * @c convert_to() function. If unsuccessful, tries using @c convert_from()
 * function in @a convert.
 *
 * Conversion functions are type defined, and the basic types can convert
 * between themselves, but conversion is strict! That is, if
 * converting from negative value to unsigned type, it will fail. It
 * also fails on value overflow.
 *
 * It is recommended that all types implement at least convert to
 * string, used by eina_value_to_string().
 *
 * @note Both objects must have eina_value_setup() called on them beforehand!
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_value_convert(const Eina_Value *value,
                                  Eina_Value *convert) EINA_ARG_NONNULL(1, 2);


/**
 * @brief Convert value to string.
 * @param value value object.
 * @return newly allocated memory or @c NULL on failure.
 *
 * @see eina_value_convert()
 * @since 1.2
 */
EAPI char *eina_value_to_string(const Eina_Value *value) EINA_ARG_NONNULL(1);

/**
 * @brief Query value type.
 * @param value value object.
 * @return type instance or @c NULL if type is invalid.
 *
 * Check if value type is valid and returns it. A type is invalid if
 * it does not exist or if it is using a different version field.
 *
 * @see eina_value_type_check()
 *
 * @since 1.2
 */
static inline const Eina_Value_Type *eina_value_type_get(const Eina_Value *value) EINA_PURE EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @}
 */


/**
 * @defgroup Eina_Value_Array_Group Generic Value Array management
 *
 * @{
 */


/**
 * @typedef Eina_Value_Array
 * Value type for #EINA_VALUE_TYPE_ARRAY.
 *
 * @see #_Eina_Value_Array explains fields.
 * @since 1.2
 */
typedef struct _Eina_Value_Array Eina_Value_Array;

/**
 * @struct _Eina_Value_Array
 * Used to store the array and its subtype.
 * @since 1.2
 */
struct _Eina_Value_Array
{
   const Eina_Value_Type *subtype; /**< how to allocate and access items */
   unsigned int step; /**< how to grow the members array */
   Eina_Inarray *array; /**< the array that holds data, members are of subtype->value_size bytes. */
};

/**
 * @brief Create generic value storage of type array.
 * @param subtype how to manage this array members.
 * @param step how to grow the members array.
 * @return The new value or @c NULL on failure.
 *
 * Create a new generic value storage of type array. The members are
 * managed using the description specified by @a subtype.
 *
 * On failure, @c NULL is returned.
 *
 * @note this creates from mempool and then uses
 *       eina_value_array_setup().  @see eina_value_free() @see
 *       eina_value_array_setup()
 *
 * @since 1.2
 */
EAPI Eina_Value *eina_value_array_new(const Eina_Value_Type *subtype,
                                      unsigned int step) EINA_ARG_NONNULL(1);

/**
 * @brief Initialize generic value storage of type array.
 * @param value value object
 * @param subtype how to manage array members.
 * @param step how to grow the members array.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * Initializes new generic value storage of type array with the given
 * @a subtype.
 *
 * This is the same as calling eina_value_set()
 * with #EINA_VALUE_TYPE_ARRAY followed by eina_value_pset() with
 * the #Eina_Value_Array description configured.
 *
 * @note Existing contents are ignored! If the value was previously used, then
 *       use eina_value_flush() first.
 *
 * On failure, #EINA_FALSE is returned.
 *
 * @see eina_value_flush()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_setup(Eina_Value *value,
                                               const Eina_Value_Type *subtype,
                                               unsigned int step) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Query number of elements in value of array type.
 * @param value value object.
 * @return number of child elements.
 * @since 1.2
 */
static inline unsigned int eina_value_array_count(const Eina_Value *value);

/**
 * @brief Remove element at given position in value of array type.
 * @param value value object.
 * @param position index of the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_remove(Eina_Value *value,
                                                unsigned int position) EINA_ARG_NONNULL(1);

/**
 * @brief Set the generic value in an array member.
 * @param value source value object
 * @param position index of the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The variable argument is dependent on chosen subtype. The list for
 * basic types:
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_set(Eina_Value *value,
                                             unsigned int position,
                                             ...) EINA_ARG_NONNULL(1);

/**
 * @brief Get the generic value from an array member.
 * @param value source value object
 * @param position index of the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in the variable argument parameter, and the
 * actual value is type-dependent, but usually it will be what is
 * stored inside the object. There shouldn't be any memory allocation;
 * thus the contents should @b not be freed.
 *
 * The variable argument is dependent on chosen subtype. The list for
 * basic types:
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
 * @see eina_value_array_set()
 * @see eina_value_array_vset()
 * @see eina_value_array_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_get(const Eina_Value *value,
                                             unsigned int position,
                                             ...) EINA_ARG_NONNULL(1);

/**
 * @brief Insert a generic value in an array member position.
 * @param value source value object
 * @param position index of the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The variable argument is dependent on chosen subtype. The list for
 * basic types:
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_insert(Eina_Value *value,
                                                unsigned int position,
                                                ...) EINA_ARG_NONNULL(1);


/**
 * @brief Append a generic value in an array.
 * @param value source value object
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The variable argument is dependent on chosen subtype. The list for
 * basic types:
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_append(Eina_Value *value,
                                                ...) EINA_ARG_NONNULL(1);

/**
 * @brief Set a generic value to an array member.
 * @param value source value object
 * @param position index of the member
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_vset(Eina_Value *value,
                                              unsigned int position,
                                              va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Get the generic value from an array member.
 * @param value source value object
 * @param position index of the member
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in the variable argument parameter, the
 * actual value is type-dependent, but usually it will be what is
 * stored inside the object. There shouldn't be any memory allocation,
 * thus the contents should @b not be freed.
 *
 * @see eina_value_array_vset()
 * @see eina_value_array_get()
 * @see eina_value_array_pget()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_vget(const Eina_Value *value,
                                              unsigned int position,
                                              va_list args) EINA_ARG_NONNULL(1);
/**
 * @brief Insert a generic value to an array member position.
 * @param value source value object
 * @param position index of the member
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @see eina_value_array_set()
 * @see eina_value_array_get()
 * @see eina_value_array_vset()
 * @see eina_value_array_pset()
 * @see eina_value_array_insert()
 * @see eina_value_array_pinsert()
 * @see eina_value_array_append()
 * @see eina_value_array_vappend()
 * @see eina_value_array_pappend()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_vinsert(Eina_Value *value,
                                                unsigned int position,
                                                va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Append a generic value to an array.
 * @param value source value object
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @see eina_value_array_set()
 * @see eina_value_array_get()
 * @see eina_value_array_vget()
 * @see eina_value_array_pset()
 * @see eina_value_array_insert()
 * @see eina_value_array_vinsert()
 * @see eina_value_array_pinsert()
 * @see eina_value_array_append()
 * @see eina_value_array_pappend()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_vappend(Eina_Value *value,
                                                 va_list args) EINA_ARG_NONNULL(1);


/**
 * @brief Set a generic value to an array member from a pointer.
 * @param value source value object
 * @param position index of the member
 * @param ptr pointer to specify the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @note the pointer contents are written using the size defined by
 *       type. It can be larger than void* or uint64_t.
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_pset(Eina_Value *value,
                                              unsigned int position,
                                              const void *ptr) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Retrieve a generic value into a pointer from an array member.
 * @param value source value object
 * @param position index of the member
 * @param ptr pointer to receive the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in pointer contents, the actual value is
 * type-dependent, but usually it will be what is stored inside the
 * object. There shouldn't be any memory allocation, thus the contents
 * should @b not be freed.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @see eina_value_array_set()
 * @see eina_value_array_vset()
 * @see eina_value_array_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_pget(const Eina_Value *value,
                                              unsigned int position,
                                              void *ptr) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Insert a generic value to an array member position from a pointer.
 * @param value source value object
 * @param position index of the member
 * @param ptr pointer to specify the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @note the pointer contents are written using the size defined by
 *       type. It can be larger than void* or uint64_t.
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_pinsert(Eina_Value *value,
                                                 unsigned int position,
                                                 const void *ptr) EINA_ARG_NONNULL(1);

/**
 * @brief Append a generic value to an array from a pointer.
 * @param value source value object
 * @param ptr pointer to specify the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @note the pointer contents are written using the size defined by
 *       type. It can be larger than void* or uint64_t.
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_pappend(Eina_Value *value,
                                                 const void *ptr) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieves a value from the array as an Eina_Value copy.
 * @param src source value object
 * @param position index of the member
 * @param dst where to return the array member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The argument @a dst is considered uninitialized and it's setup to
 * the type of the member.
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_array_value_get(const Eina_Value *src,
                                                   unsigned int position,
                                                   Eina_Value *dst) EINA_ARG_NONNULL(1, 3);

/**
 * @}
 */


/**
 * @defgroup Eina_Value_List_Group Generic Value List management
 *
 * @{
 */


/**
 * @typedef Eina_Value_List
 * Value type for #EINA_VALUE_TYPE_LIST.
 *
 * @see #_Eina_Value_List explains fields.
 * @since 1.2
 */
typedef struct _Eina_Value_List Eina_Value_List;

/**
 * @struct _Eina_Value_List
 * Used to store the list and its subtype.
 * @since 1.2
 */
struct _Eina_Value_List
{
   const Eina_Value_Type *subtype; /**< how to allocate and access items */
   Eina_List *list; /**< the list that holds data, members are of subtype->value_size bytes. */
};

/**
 * @brief Create generic value storage of type list.
 * @param subtype how to manage this list members.
 * @return The new value or @c NULL on failure.
 *
 * Create a new generic value storage of type list. The members are
 * managed using the description specified by @a subtype.
 *
 * On failure, @c NULL is returned.
 *
 * @note this creates from mempool and then uses
 *       eina_value_list_setup().
 *
 * @see eina_value_free()
 * @see eina_value_list_setup()
 *
 * @since 1.2
 */
EAPI Eina_Value *eina_value_list_new(const Eina_Value_Type *subtype) EINA_ARG_NONNULL(1);

/**
 * @brief Initialize generic value storage of type list.
 * @param value value object
 * @param subtype how to manage this list members.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * Initializes new generic value storage of type list with the given
 * @a subtype.
 *
 * This is the same as calling eina_value_set()
 * with #EINA_VALUE_TYPE_LIST followed by eina_value_pset() with
 * the #Eina_Value_List description configured.
 *
 * @note Existing contents are ignored! If the value was previously used, then
 *       use eina_value_flush() first.
 *
 * On failure, #EINA_FALSE is returned.
 *
 * @see eina_value_flush()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_setup(Eina_Value *value,
                                               const Eina_Value_Type *subtype) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Query number of elements in value of list type.
 * @param value value object.
 * @return number of child elements.
 * @since 1.2
 */
static inline unsigned int eina_value_list_count(const Eina_Value *value);

/**
 * @brief Remove element at given position in value of list type.
 * @param value value object.
 * @param position index of the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_remove(Eina_Value *value,
                                                unsigned int position) EINA_ARG_NONNULL(1);

/**
 * @brief Set the generic value in an list member.
 * @param value source value object
 * @param position index of the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The variable argument is dependent on chosen subtype. The list for
 * basic types:
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_set(Eina_Value *value,
                                             unsigned int position,
                                             ...) EINA_ARG_NONNULL(1);

/**
 * @brief Get the generic value from an list member.
 * @param value source value object
 * @param position index of the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in the variable argument parameter, the
 * actual value is type-dependent, but usually it will be what is
 * stored inside the object. There shouldn't be any memory allocation,
 * thus the contents should @b not be freed.
 *
 * The variable argument is dependent on chosen subtype. The list for
 * basic types:
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
 * @see eina_value_list_set()
 * @see eina_value_list_vset()
 * @see eina_value_list_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_get(const Eina_Value *value,
                                             unsigned int position,
                                             ...) EINA_ARG_NONNULL(1);

/**
 * @brief Insert the generic value in an list member position.
 * @param value source value object
 * @param position index of the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The variable argument is dependent on chosen subtype. The list for
 * basic types:
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_insert(Eina_Value *value,
                                                unsigned int position,
                                                ...) EINA_ARG_NONNULL(1);


/**
 * @brief Append the generic value in an list.
 * @param value source value object
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The variable argument is dependent on chosen subtype. The list for
 * basic types:
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_append(Eina_Value *value,
                                                ...) EINA_ARG_NONNULL(1);

/**
 * @brief Set the generic value in an list member.
 * @param value source value object
 * @param position index of the member
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_vset(Eina_Value *value,
                                              unsigned int position,
                                              va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Get the generic value from an list member.
 * @param value source value object
 * @param position index of the member
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in the variable argument parameter, the
 * actual value is type-dependent, but usually it will be what is
 * stored inside the object. There shouldn't be any memory allocation,
 * thus the contents should @b not be freed.
 *
 * @see eina_value_list_vset()
 * @see eina_value_list_get()
 * @see eina_value_list_pget()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_vget(const Eina_Value *value,
                                              unsigned int position,
                                              va_list args) EINA_ARG_NONNULL(1);
/**
 * @brief Insert the generic value in an list member position.
 * @param value source value object
 * @param position index of the member
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @see eina_value_list_set()
 * @see eina_value_list_get()
 * @see eina_value_list_vset()
 * @see eina_value_list_pset()
 * @see eina_value_list_insert()
 * @see eina_value_list_pinsert()
 * @see eina_value_list_append()
 * @see eina_value_list_vappend()
 * @see eina_value_list_pappend()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_vinsert(Eina_Value *value,
                                                unsigned int position,
                                                va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Append the generic value in an list.
 * @param value source value object
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_vappend(Eina_Value *value,
                                                 va_list args) EINA_ARG_NONNULL(1);


/**
 * @brief Set the generic value in an list member from pointer.
 * @param value source value object
 * @param position index of the member
 * @param ptr pointer to specify the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @note the pointer contents are written using the size defined by
 *       type. It can be larger than void* or uint64_t.
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_pset(Eina_Value *value,
                                              unsigned int position,
                                              const void *ptr) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Get the generic value to pointer from an list member.
 * @param value source value object
 * @param position index of the member
 * @param ptr pointer to receive the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in pointer contents, the actual value is
 * type-dependent, but usually it will be what is stored inside the
 * object. There shouldn't be any memory allocation, thus the contents
 * should @b not be freed.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @see eina_value_list_set()
 * @see eina_value_list_vset()
 * @see eina_value_list_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_pget(const Eina_Value *value,
                                              unsigned int position,
                                              void *ptr) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Insert the generic value in an list member position from pointer.
 * @param value source value object
 * @param position index of the member
 * @param ptr pointer to specify the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @note the pointer contents are written using the size defined by
 *       type. It can be larger than void* or uint64_t.
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_pinsert(Eina_Value *value,
                                                 unsigned int position,
                                                 const void *ptr) EINA_ARG_NONNULL(1);

/**
 * @brief Append the generic value in an list from pointer.
 * @param value source value object
 * @param ptr pointer to specify the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @note the pointer contents are written using the size defined by
 *       type. It can be larger than void* or uint64_t.
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
 * @since 1.2
 */
static inline Eina_Bool eina_value_list_pappend(Eina_Value *value,
                                                 const void *ptr) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @defgroup Eina_Value_Hash_Group Generic Value Hash management
 *
 * @{
 */

/**
 * @typedef Eina_Value_Hash
 * Value type for #EINA_VALUE_TYPE_HASH.
 *
 * @see #_Eina_Value_Hash explains fields.
 * @since 1.2
 */
typedef struct _Eina_Value_Hash Eina_Value_Hash;

/**
 * @struct _Eina_Value_Hash
 * Used to store the hash and its subtype.
 * @since 1.2
 */
struct _Eina_Value_Hash
{
   const Eina_Value_Type *subtype; /**< how to allocate and access items */
   unsigned int buckets_power_size; /**< how to allocate hash buckets, if zero a sane default is chosen. */
   Eina_Hash *hash; /**< the hash that holds data, members are of subtype->value_size bytes. */
};

/**
 * @brief Create generic value storage of type hash.
 * @param subtype how to manage this hash members.
 * @param buckets_power_size how to allocate hash buckets (2 ^
 *        buckets_power_size), if zero then a sane value is chosen.
 * @return The new value or @c NULL on failure.
 *
 * Create a new generic value storage of type hash. The members are
 * managed using the description specified by @a subtype.
 *
 * On failure, @c NULL is returned.
 *
 * @note this creates from mempool and then uses
 *       eina_value_hash_setup().
 *
 * @see eina_value_free()
 * @see eina_value_hash_setup()
 *
 * @since 1.2
 */
EAPI Eina_Value *eina_value_hash_new(const Eina_Value_Type *subtype, unsigned int buckets_power_size) EINA_ARG_NONNULL(1);

/**
 * @brief Initialize generic value storage of type hash.
 * @param value value object
 * @param subtype how to manage this hash members.
 * @param buckets_power_size how to allocate hash buckets (2 ^
 *        buckets_power_size), if zero then a sane value is chosen.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * Initializes new generic value storage of type hash with the given
 * @a subtype.
 *
 * This is the same as calling eina_value_set()
 * with #EINA_VALUE_TYPE_HASH followed by eina_value_pset() with
 * the #Eina_Value_Hash description configured.
 *
 * @note Existing contents are ignored! If the value was previously used, then
 *       use eina_value_flush() first.
 *
 * On failure, #EINA_FALSE is returned.
 *
 * @see eina_value_flush()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_hash_setup(Eina_Value *value,
                                              const Eina_Value_Type *subtype,
                                              unsigned int buckets_power_size) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Query number of elements in value of hash type.
 * @param value value object.
 * @return number of child elements.
 * @since 1.2
 */
static inline unsigned int eina_value_hash_population(const Eina_Value *value);

/**
 * @brief Remove element at given position in value of hash type.
 * @param value value object.
 * @param key key to find the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @since 1.2
 */
static inline Eina_Bool eina_value_hash_del(Eina_Value *value,
                                            const char *key) EINA_ARG_NONNULL(1);

/**
 * @brief Set the generic value in an hash member.
 * @param value source value object
 * @param key key to find the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The variable argument is dependent on chosen subtype. The list for
 * basic types:
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
 * @see eina_value_hash_get()
 * @see eina_value_hash_vset()
 * @see eina_value_hash_pset()
 * @see eina_value_hash_del()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_hash_set(Eina_Value *value,
                                            const char *key,
                                            ...) EINA_ARG_NONNULL(1);

/**
 * @brief Get the generic value from an hash member.
 * @param value source value object
 * @param key key to find the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in the variable argument parameter, the
 * actual value is type-dependent, but usually it will be what is
 * stored inside the object. There shouldn't be any memory allocation,
 * thus the contents should @b not be freed.
 *
 * The variable argument is dependent on chosen subtype. The list for
 * basic types:
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
 * @see eina_value_hash_set()
 * @see eina_value_hash_vset()
 * @see eina_value_hash_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_hash_get(const Eina_Value *value,
                                            const char *key,
                                            ...) EINA_ARG_NONNULL(1);

/**
 * @brief Set the generic value in an hash member.
 * @param value source value object
 * @param key key to find the member
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @see eina_value_hash_set()
 * @see eina_value_hash_get()
 * @see eina_value_hash_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_hash_vset(Eina_Value *value,
                                             const char *key,
                                             va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Get the generic value from an hash member.
 * @param value source value object
 * @param key key to find the member
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in the variable argument parameter, the
 * actual value is type-dependent, but usually it will be what is
 * stored inside the object. There shouldn't be any memory allocation,
 * thus the contents should @b not be freed.
 *
 * @see eina_value_hash_vset()
 * @see eina_value_hash_get()
 * @see eina_value_hash_pget()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_hash_vget(const Eina_Value *value,
                                             const char *key,
                                             va_list args) EINA_ARG_NONNULL(1);

/**
 * @brief Set the generic value in an hash member from pointer.
 * @param value source value object
 * @param key key to find the member
 * @param ptr pointer to specify the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @note the pointer contents are written using the size defined by
 *       type. It can be larger than void* or uint64_t.
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
 * @see eina_value_hash_set()
 * @see eina_value_hash_get()
 * @see eina_value_hash_vset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_hash_pset(Eina_Value *value,
                                             const char *key,
                                             const void *ptr) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Get the generic value to pointer from an hash member.
 * @param value source value object
 * @param key key to find the member
 * @param ptr pointer to receive the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in pointer contents, the actual value is
 * type-dependent, but usually it will be what is stored inside the
 * object. There shouldn't be any memory allocation, thus the contents
 * should @b not be freed.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @see eina_value_hash_set()
 * @see eina_value_hash_vset()
 * @see eina_value_hash_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_hash_pget(const Eina_Value *value,
                                             const char *key,
                                             void *ptr) EINA_ARG_NONNULL(1, 3);

/**
 * @}
 */

/**
 * @defgroup Eina_Value_Blob_Group Generic Value Blob management
 *
 * @{
 */

/**
 * @typedef Eina_Value_Blob_Operations
 * How to manage blob. Any @c NULL callback is ignored.
 * @see #_Eina_Value_Blob_Operations explains fields.
 * @since 1.2
 */
typedef struct _Eina_Value_Blob_Operations Eina_Value_Blob_Operations;

/**
 * @def EINA_VALUE_BLOB_OPERATIONS_VERSION
 * Current API version, used to validate #_Eina_Value_Blob_Operations.
 */
#define EINA_VALUE_BLOB_OPERATIONS_VERSION (1)

/**
 * @struct _Eina_Value_Blob_Operations
 * How to manage blob. Any @c NULL callback is ignored.
 * @since 1.2
 */
struct _Eina_Value_Blob_Operations
{
   unsigned int version; /**< must be #EINA_VALUE_BLOB_OPERATIONS_VERSION */
   void (*free)(const Eina_Value_Blob_Operations *ops, void *memory, size_t size);
   void *(*copy)(const Eina_Value_Blob_Operations *ops, const void *memory, size_t size);
   int (*compare)(const Eina_Value_Blob_Operations *ops, const void *data1, size_t size_data1, const void *data2, size_t size_data2);
   char *(*to_string)(const Eina_Value_Blob_Operations *ops, const void *memory, size_t size);
};

/**
 * @var EINA_VALUE_BLOB_OPERATIONS_MALLOC
 *
 * Assumes @c memory was create with malloc() and applies free() to it
 * during flush (Eina_Value_Blob_Operations::free). Copy is done with
 * malloc() as well.
 *
 * No compare or to_string are provided, defaults will be used.
 */
EAPI extern const Eina_Value_Blob_Operations *EINA_VALUE_BLOB_OPERATIONS_MALLOC;

/**
 * @typedef Eina_Value_Blob
 * Value type for #EINA_VALUE_TYPE_BLOB.
 *
 * @see #_Eina_Value_Blob explains fields.
 * @since 1.2
 */
typedef struct _Eina_Value_Blob Eina_Value_Blob;

/**
 * @struct _Eina_Value_Blob
 * Used to store the blob information and management operations.
 * @since 1.2
 */
struct _Eina_Value_Blob
{
   const Eina_Value_Blob_Operations *ops; /**< if @c NULL, nothing is freed, copy will just copy the memory pointer, not its value. */
   const void *memory;
   unsigned int size;
};

/**
 * @}
 */

/**
 * @defgroup Eina_Value_Struct_Group Generic Value Struct management
 *
 * @{
 */

/**
 * @typedef Eina_Value_Struct_Operations
 * How to manage struct. Any @c NULL callback is ignored.
 *
 * A structure can specify alternative methods to allocate, free and
 * copy itself. See structure definition for all methods.
 *
 * @see #_Eina_Value_Struct_Operations explains fields.
 * @since 1.2
 */
typedef struct _Eina_Value_Struct_Operations Eina_Value_Struct_Operations;

/**
 * @typedef Eina_Value_Struct_Member
 * Describes a single member of struct.
 *
 * The member holds a name, type and its byte offset within the struct
 * memory. Most Eina_Value_Struct functions takes the member name as
 * parameter, as in eina_value_struct_set().
 *
 * @see #_Eina_Value_Struct_Member explains fields.
 * @since 1.2
 */
typedef struct _Eina_Value_Struct_Member Eina_Value_Struct_Member;

/**
 * @typedef Eina_Value_Struct_Desc
 * Describes the struct by listing its size, members and operations.
 * @see #_Eina_Value_Struct_Desc explains fields.
 * @since 1.2
 */
typedef struct _Eina_Value_Struct_Desc Eina_Value_Struct_Desc;

/**
 * @typedef Eina_Value_Struct
 * Value type for #EINA_VALUE_TYPE_STRUCT.
 *
 * @see #_Eina_Value_Struct explains fields.
 * @since 1.2
 */
typedef struct _Eina_Value_Struct Eina_Value_Struct;

/**
 * @def EINA_VALUE_STRUCT_OPERATIONS_VERSION
 * Current API version, used to validate #_Eina_Value_Struct_Operations.
 */
#define EINA_VALUE_STRUCT_OPERATIONS_VERSION (1)

/**
 * @struct _Eina_Value_Struct_Operations
 * How to manage struct. Any @c NULL callback is ignored.
 * @since 1.2
 */
struct _Eina_Value_Struct_Operations
{
   unsigned int version; /**< must be #EINA_VALUE_STRUCT_OPERATIONS_VERSION */
   void *(*alloc)(const Eina_Value_Struct_Operations *ops, const Eina_Value_Struct_Desc *desc); /**< How to allocate struct memory to be managed by the Eina_Value */
   void (*free)(const Eina_Value_Struct_Operations *ops, const Eina_Value_Struct_Desc *desc, void *memory); /**< How to release memory managed by the Eina_Value */
   void *(*copy)(const Eina_Value_Struct_Operations *ops, const Eina_Value_Struct_Desc *desc, const void *memory); /**< How to copy struct memory from an existing Eina_Value, if not provided alloc() will be used, then every member is copied using eina_value_type_copy() with member's type. */
   int (*compare)(const Eina_Value_Struct_Operations *ops, const Eina_Value_Struct_Desc *desc, const void *data1, const void *data2); /**< How to compare two struct memories */
   const Eina_Value_Struct_Member *(*find_member)(const Eina_Value_Struct_Operations *ops, const Eina_Value_Struct_Desc *desc, const char *name); /**< How to find description for member. For huge structures consider using binary search, stringshared, hash or gperf. The default function does linear search using strcmp(). */
};

/**
 * @var EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH
 *
 * Assumes @c members is sorted by name and applies binary search for
 * names.
 *
 * Ideally the @c member_count field is set to speed it up.
 *
 * No other methods are set (alloc, free, copy, compare), then it uses
 * the default operations.
 */
EAPI extern const Eina_Value_Struct_Operations *EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH;

/**
 * @var EINA_VALUE_STRUCT_OPERATIONS_STRINGSHARE
 *
 * Assumes @c members name are stringshared and can be compared for
 * equality without using its contents (simple pointer comparison).
 *
 * Ideally the search @c name will be stringshared as well, but it
 * will do a second loop with a forced stringshare if it did not find
 * the member.
 *
 * No other methods are set (alloc, free, copy, compare), then it uses
 * the default operations.
 */
EAPI extern const Eina_Value_Struct_Operations *EINA_VALUE_STRUCT_OPERATIONS_STRINGSHARE;

/**
 * @struct _Eina_Value_Struct_Member
 * Describes a single member of struct.
 *
 * The name is used to lookup the member description. This is done as
 * specified as _Eina_Value_Struct_Operations::find_member(). For
 * structures with huge number of members, consider using a better
 * find_member function to quickly finding it! There are two helper
 * operations provided to help this: #EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH
 * and #EINA_VALUE_STRUCT_OPERATIONS_STRINGSHARE, both depend on properly
 * set #_Eina_Value_Struct_Desc and #_Eina_Value_Struct_Member.
 *
 * @see #EINA_VALUE_STRUCT_MEMBER
 * @see #EINA_VALUE_STRUCT_MEMBER_SENTINEL
 *
 * @since 1.2
 */
struct _Eina_Value_Struct_Member
{
   const char *name; /**< member name, used in lookups such as eina_value_struct_get() */
   const Eina_Value_Type *type; /**< how to use this member */
   unsigned int offset; /**< where this member is located within the structure memory */
};

/**
 * @def EINA_VALUE_STRUCT_DESC_VERSION
 * Current API version, used to validate #_Eina_Value_Struct_Desc.
 */
#define EINA_VALUE_STRUCT_DESC_VERSION (1)

/**
 * @struct _Eina_Value_Struct_Desc
 * Describes the struct by listing its size, members and operations.
 *
 * This is the root of Eina_Value knowledge about the memory it's
 * handling as a structure. It adds introspection, saying the byte
 * size of the structure, its members and how to manage such members.
 *
 * @since 1.2
 */
struct _Eina_Value_Struct_Desc
{
   unsigned int version; /**< must be #EINA_VALUE_STRUCT_DESC_VERSION */
   const Eina_Value_Struct_Operations *ops; /**< operations, if @c NULL defaults will be used. You may use operations to optimize member lookup using binary search or gperf hash. */
   const Eina_Value_Struct_Member *members; /**< array of member descriptions, if @c member_count is zero, then it must be @c NULL terminated. */
   unsigned int member_count; /**< if > 0, specifies number of members. If zero then @c members must be NULL terminated. */
   unsigned int size; /**< byte size to allocate, may be bigger than sum of members */
};

/**
 * @def EINA_VALUE_STRUCT_MEMBER
 *
 * Helper to define Eina_Value_Struct_Member fields, uses offsetof()
 * with type and member.
 *
 * @since 1.2
 */
#define EINA_VALUE_STRUCT_MEMBER(eina_value_type, type, member) \
   {#member, eina_value_type, offsetof(type, member)}

/**
 * @def EINA_VALUE_STRUCT_MEMBER_SENTINEL
 *
 * Helper to define Eina_Value_Struct_Member fields for sentinel (last
 * item), useful if you did not define @c member_count.
 *
 * @since 1.2
 */
#define EINA_VALUE_STRUCT_MEMBER_SENTINEL {NULL, NULL, 0}


/**
 * @struct _Eina_Value_Struct
 * Used to store the memory and its description.
 * @since 1.2
 */
struct _Eina_Value_Struct
{
   const Eina_Value_Struct_Desc *desc; /**< How to manage the structure */
   void *memory; /**< The managed structure memory */
};

/**
 * @brief Create generic value storage of type struct.
 * @param desc how to manage this struct members.
 * @return The new value or @c NULL on failure.
 *
 * Create a new generic value storage of type struct. The members are
 * managed using the description specified by @a desc.
 *
 * On failure, @c NULL is returned.
 *
 * @note this creates from mempool and then uses
 *       eina_value_struct_setup().
 *
 * @see eina_value_free()
 * @see eina_value_struct_setup()
 *
 * @since 1.2
 */
EAPI Eina_Value *eina_value_struct_new(const Eina_Value_Struct_Desc *desc) EINA_ARG_NONNULL(1);

/**
 * @brief Initialize generic value storage of type struct.
 * @param value value object
 * @param desc how to manage this struct members.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * Initializes new generic value storage of type struct with the given
 * @a desc.
 *
 * This is the same as calling eina_value_set()
 * with #EINA_VALUE_TYPE_STRUCT followed by eina_value_pset() with
 * the #Eina_Value_Struct description configured.
 *
 * @note Existing contents are ignored! If the value was previously used, then
 *       use eina_value_flush() first.
 *
 * On failure, #EINA_FALSE is returned.
 *
 * @see eina_value_flush()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_struct_setup(Eina_Value *value,
                                                const Eina_Value_Struct_Desc *desc) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Set the generic value in an struct member.
 * @param value source value object
 * @param name name to find the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The variable argument is dependent on chosen member type. The list
 * for basic types:
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
 * @see eina_value_struct_get()
 * @see eina_value_struct_vset()
 * @see eina_value_struct_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_struct_set(Eina_Value *value,
                                              const char *name,
                                              ...) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Get the generic value from an struct member.
 * @param value source value object
 * @param name name to find the member
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in the variable argument parameter, the
 * actual value is type-dependent, but usually it will be what is
 * stored inside the object. There shouldn't be any memory allocation,
 * thus the contents should @b not be freed.
 *
 * The variable argument is dependent on chosen member type. The list
 * for basic types:
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
 * @see eina_value_struct_set()
 * @see eina_value_struct_vset()
 * @see eina_value_struct_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_struct_get(const Eina_Value *value,
                                              const char *name,
                                              ...) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Set the generic value in an struct member.
 * @param value source value object
 * @param name name to find the member
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @see eina_value_struct_set()
 * @see eina_value_struct_get()
 * @see eina_value_struct_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_struct_vset(Eina_Value *value,
                                               const char *name,
                                               va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Get the generic value from an struct member.
 * @param value source value object
 * @param name name to find the member
 * @param args variable argument
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in the variable argument parameter, the
 * actual value is type-dependent, but usually it will be what is
 * stored inside the object. There shouldn't be any memory allocation,
 * thus the contents should @b not be freed.
 *
 * @see eina_value_struct_vset()
 * @see eina_value_struct_get()
 * @see eina_value_struct_pget()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_struct_vget(const Eina_Value *value,
                                               const char *name,
                                               va_list args) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Set the generic value in an struct member from pointer.
 * @param value source value object
 * @param name name to find the member
 * @param ptr pointer to specify the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @note the pointer contents are written using the size defined by
 *       type. It can be larger than void* or uint64_t.
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
 * @see eina_value_struct_set()
 * @see eina_value_struct_get()
 * @see eina_value_struct_vset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_struct_pset(Eina_Value *value,
                                               const char *name,
                                               const void *ptr) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Get the generic value to pointer from an struct member.
 * @param value source value object
 * @param name name to find the member
 * @param ptr pointer to receive the contents.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * The value is returned in pointer contents, the actual value is
 * type-dependent, but usually it will be what is stored inside the
 * object. There shouldn't be any memory allocation, thus the contents
 * should @b not be freed.
 *
 * The pointer type is dependent on chosen value type. The list for
 * basic types:
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
 * @see eina_value_struct_set()
 * @see eina_value_struct_vset()
 * @see eina_value_struct_pset()
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_struct_pget(const Eina_Value *value,
                                               const char *name,
                                               void *ptr) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Get the member as Eina_Value copy
 * @param src source value object
 * @param name name to find the member
 * @param dst where to return the member value.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * The argument @a dst is considered uninitialized and it's setup to
 * the type of the member.
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_struct_value_get(const Eina_Value *src,
                                                    const char *name,
                                                    Eina_Value *dst) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Set the member from Eina_Value source
 * @param dst destination value object
 * @param name name to find the member
 * @param src source value
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_struct_value_set(Eina_Value *dst,
                                                    const char *name,
                                                    const Eina_Value *src) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Get the member as Eina_Value copy given its member description.
 * @param src source value object
 * @param member the member description to use
 * @param dst where to return the member value.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * The argument @a dst is considered uninitialized and it's setup to
 * the type of the member.
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_struct_member_value_get(const Eina_Value *src,
                                                           const Eina_Value_Struct_Member *member,
                                                           Eina_Value *dst) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Set the member from Eina_Value source
 * @param dst destination value object
 * @param member the member description to use
 * @param src source value
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @since 1.2
 */
static inline Eina_Bool eina_value_struct_member_value_set(Eina_Value *dst,
                                                           const Eina_Value_Struct_Member *member,
                                                           const Eina_Value *src) EINA_ARG_NONNULL(1, 2, 3);


/**
 * @}
 */


/**
 * @defgroup Eina_Value_Type_Group Generic Value Type management
 *
 * @{
 */

/**
 * @def EINA_VALUE_TYPE_VERSION
 * Current API version, used to validate type.
 */
#define EINA_VALUE_TYPE_VERSION (1)

/**
 * @struct _Eina_Value_Type
 * API to access values.
 *
 * @since 1.2
 */
struct _Eina_Value_Type
{
   unsigned int version; /**< must be #EINA_VALUE_TYPE_VERSION */
   unsigned int value_size; /**< byte size of value */
   const char *name; /**< name for debug and introspection */
   Eina_Bool (*setup)(const Eina_Value_Type *type, void *mem); /**< mem will be malloc(value_size) and should be configured */
   Eina_Bool (*flush)(const Eina_Value_Type *type, void *mem); /**< clear any values from mem */
   Eina_Bool (*copy)(const Eina_Value_Type *type, const void *src, void *dst); /**< how to copy values, both memory are @c value_size */
   int (*compare)(const Eina_Value_Type *type, const void *a, const void *b); /**< how to compare values, both memory are @c value_size */
   Eina_Bool (*convert_to)(const Eina_Value_Type *type, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem); /**< how to convert values, both memory are @c value_size */
   Eina_Bool (*convert_from)(const Eina_Value_Type *type, const Eina_Value_Type *convert, void *type_mem, const void *convert_mem); /**< how to convert values, both memory are @c value_size */
   Eina_Bool (*vset)(const Eina_Value_Type *type, void *mem, va_list args); /**< how to set memory from variable argument */
   Eina_Bool (*pset)(const Eina_Value_Type *type, void *mem, const void *ptr); /**< how to set memory from pointer */
   Eina_Bool (*pget)(const Eina_Value_Type *type, const void *mem, void *ptr); /**< how to read memory */
};

/**
 * @brief Query type name.
 * @param type type reference.
 * @return string or @c NULL if type is invalid.
 * @since 1.2
 */
EAPI const char *eina_value_type_name_get(const Eina_Value_Type *type) EINA_PURE EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check if type is valid.
 * @param type type reference.
 * @return #EINA_TRUE if valid, #EINA_FALSE otherwise.
 *
 * A type is invalid if it's NULL or if version field is not the same
 * as runtime #EINA_VALUE_TYPE_VERSION.
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_value_type_check(const Eina_Value_Type *type) EINA_PURE EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Initialize memory using type descriptor.
 * @param type type reference.
 * @param mem memory to operate, must be of size @c type->value_size.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @since 1.2
 */
static inline Eina_Bool eina_value_type_setup(const Eina_Value_Type *type, void *mem);

/**
 * @brief Flush (clear) memory using type descriptor.
 * @param type type reference.
 * @param mem memory to operate, must be of size @c type->value_size.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @since 1.2
 */
static inline Eina_Bool eina_value_type_flush(const Eina_Value_Type *type, void *mem);

/**
 * @brief Copy memory using type descriptor.
 * @param type type reference.
 * @param src memory to operate, must be of size @c type->value_size.
 * @param dst memory to operate, must be of size @c type->value_size.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @since 1.2
 */
static inline Eina_Bool eina_value_type_copy(const Eina_Value_Type *type, const void *src, void *dst);

/**
 * @brief Compare memory using type descriptor.
 * @param type type reference.
 * @param a memory to operate, must be of size @c type->value_size.
 * @param b memory to operate, must be of size @c type->value_size.
 * @return less than zero if a < b, greater than zero if a > b, zero if equal.
 * @since 1.2
 */
static inline int eina_value_type_compare(const Eina_Value_Type *type, const void *a, const void *b);

/**
 * @brief Convert memory using type descriptor.
 * @param type type reference of the source.
 * @param convert type reference of the destination.
 * @param type_mem memory to operate, must be of size @c type->value_size.
 * @param convert_mem memory to operate, must be of size @c convert->value_size.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @since 1.2
 */
static inline Eina_Bool eina_value_type_convert_to(const Eina_Value_Type *type, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem);

/**
 * @brief Convert memory using type descriptor.
 * @param type type reference of the destination.
 * @param convert type reference of the source.
 * @param type_mem memory to operate, must be of size @c type->value_size.
 * @param convert_mem memory to operate, must be of size @c convert->value_size.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @since 1.2
 */
static inline Eina_Bool eina_value_type_convert_from(const Eina_Value_Type *type, const Eina_Value_Type *convert, void *type_mem, const void *convert_mem);

/**
 * @brief Set memory using type descriptor and variable argument.
 * @param type type reference of the source.
 * @param mem memory to operate, must be of size @c type->value_size.
 * @param args input value.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @since 1.2
 */
static inline Eina_Bool eina_value_type_vset(const Eina_Value_Type *type, void *mem, va_list args);

/**
 * @brief Set memory using type descriptor and pointer.
 * @param type type reference of the source.
 * @param mem memory to operate, must be of size @c type->value_size.
 * @param ptr pointer to input value.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @since 1.2
 */
static inline Eina_Bool eina_value_type_pset(const Eina_Value_Type *type, void *mem, const void *ptr);

/**
 * @brief Get memory using type descriptor.
 * @param type type reference of the source.
 * @param mem memory to operate, must be of size @c type->value_size.
 * @param ptr pointer to output.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 * @since 1.2
 */
static inline Eina_Bool eina_value_type_pget(const Eina_Value_Type *type, const void *mem, void *ptr);

/**
 * @}
 */

#include "eina_inline_value.x"

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
#endif
