/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Gustavo Sverzut Barbieri,
 *                         Vincent Torri, Jorge Luis Zapata Muga, Cedric Bail
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

#ifndef EINA_HASH_H_
#define EINA_HASH_H_

#include "eina_types.h"
#include "eina_iterator.h"

/**
 * @page hash_01_example_page Eina_Hash in action
 * @dontinclude eina_hash_01.c
 *
 * We are going to store some tuples into our table, that will map each @a name
 * to a @a number. The cost to access a given number from the name should be
 * very small, even with many entries in our table. This is the initial data:
 * @skip _Phone_Entry
 * @until // _start_entries
 *
 * Before starting to play with the hash, let's write a callback that will be
 * used to free the elements from it. Since we are just storing strduped
 * strings, we just need to free them:
 *
 * @skip static
 * @until }
 *
 * We also need a callback to iterate over the elements of the list later, so
 * we are defining it now:
 *
 * @skip Eina_Bool
 * @until }
 *
 * Now let's create our @ref Eina_Hash using @ref
 * eina_hash_string_superfast_new :
 *
 * @skip eina_init
 * @until phone_book
 *
 * Now we add the keys and data to the hash using @ref eina_hash_add . This
 * means that the key is copied inside the table, together with the pointer to
 * the data (phone numbers).
 *
 * @skip for
 * @until }
 *
 * Some basic manipulations with the hash, like finding a value given a key,
 * deleting an entry, modifying an entry are exemplified in the following lines.
 * Notice that the @ref eina_hash_modify function returns the old value stored
 * in that entry, and it needs to be freed, while the @ref eina_hash_del
 * function already calls our free callback:
 *
 * @skip Look for
 * @until free(
 *
 * The @ref eina_hash_set function can be used to set a key-value entry to the
 * table if it doesn't exist, or to modify an existent entry. It returns the old
 * entry if it was already set, and NULL otherwise. But since it will
 * return NULL on error too, we need to check if an error has occurred:
 *
 * @skip Modify
 * @until printf("\n");
 *
 * There are different ways of iterate over the entries of a hash. Here we show
 * two of them: using @ref eina_hash_foreach and @ref Eina_Iterator.
 *
 * @skip List of phones
 * @until eina_iterator_free(it);
 *
 * It's also possible to change the key for a specific entry, without having to
 * remove the entry from the table and adding it again:
 *
 * @skipline eina_hash_move
 *
 * We can remove all the elements from the table without free the table itself:
 *
 * @skip Empty the phone book
 * @until eina_hash_population
 *
 * Or free the the entire table with its content:
 *
 * @skipline eina_hash_free
 *
 *
 * The full code for this example can be seen here: @ref eina_hash_01_c
 */

/**
 * @page eina_hash_01_c Hash table in action
 *
 * @include eina_hash_01.c
 * @example eina_hash_01.c
 */

/**
 * @page hash_02_example_page Different types of tables
 *
 * This example shows two more types of hash tables that can be created using
 * @ref Eina_Hash . For more types, consult the reference documentation of @ref
 * eina_hash_new.
 * @include eina_hash_02.c
 * @example eina_hash_02.c
 */

/**
 * @example eina_hash_03.c
 * Same example as @ref hash_01_example_page but using a "string small" hash
 * table instead of "string superfast".
 */

/**
 * @example eina_hash_04.c
 * Same example as @ref hash_01_example_page but using a "string djb2" hash
 * table instead of "string superfast".
 */

/**
 * @example eina_hash_05.c
 * Same example as @ref hash_01_example_page but using a "int32" hash
 * table instead of "string superfast".
 */

/**
 * @example eina_hash_06.c
 * Same example as @ref hash_01_example_page but using a "int64" hash
 * table instead of "string superfast".
 */

/**
 * @example eina_hash_07.c
 * Same example as @ref hash_01_example_page but using a "pointer" hash
 * table instead of "string superfast".
 */

/**
 * @example eina_hash_08.c
 * This example shows the the usage of eina_hash_add(), eina_hash_add_by_hash(),
 * eina_hash_direct_add_by_hash(), eina_hash_del(), eina_hash_del_by_key_hash(),
 * eina_hash_del_by_key(), eina_hash_del_by_data(), eina_hash_find_by_hash() and
 * eina_hash_modify_by_hash().
 */

/**
 * @addtogroup Eina_Hash_Group Hash Table
 *
 * @brief Hash table management. Maps keys to values.
 *
 * The hash table associates keys (e.g. strings) to data, with
 * relatively fast access time. The performance is proportional to the
 * load factor of the table (number of elements / number of
 * buckets). See @ref hashtable_algo for implementation details.
 *
 * There are optimized implementations for some common key types, such
 * as strings, integers, pointers, and stringshared; custom optimizations
 * are also permitted.
 *
 * The hash table keys can be either copied or non-copied, using
 * eina_hash_add() or eina_hash_direct_add(), respectively.
 *
 * @section hashtable_algo Algorithm
 *
 * The Eina_Hash is implemented using an array of N "buckets", where each
 * bucket is a pointer to a structure that is the head of a <a
 * href="http://en.wikipedia.org/wiki/Red-black_tree">red-black tree</a>. The
 * array can then be indexed by the [hash_of_element mod N]. The
 * hash_of_element is calculated using the hashing function, passed as a
 * parameter to the @ref eina_hash_new function. N is the number of buckets
 * (array positions), and is calculated based on the buckets_power_size
 * (argument of @ref eina_hash_new too). The following picture illustrates the
 * basic idea:
 *
 * @htmlonly
 * <img src="01_hash-table.png" width="500" />
 * @endhtmlonly
 * @image latex 01_hash-table.eps
 *
 * Adding an element to the hash table involves the following steps:
 * @li calculate the hash for that key (using the specified hash function);
 * @li calculate the array position [hash mod N];
 * @li add the element to the rbtree on that position.
 *
 * The first two steps have constant time, proportional to the hash function
 * being used. Adding the key to the rbtree will be proportional to the number
 * of keys in that bucket.
 *
 * The average lookup cost depends on the number of keys per bucket
 * (load factor) of the table, assuming the distribution of keys is
 * sufficiently uniform.
 *
 * @section hashtable_perf Performance
 *
 * Keeping the load factor small will improve the hash table performance. But
 * increasing the buckets_power_size will also increase the memory consumption.
 * The default hash table creation functions provides enough
 * buckets for most cases. If just a few string keys
 * (less than 30) will be added to the hash table, @ref
 * eina_hash_string_small_new should be used, since it reduces the memory
 * consumption for the buckets without causing too many collisions.
 * However, @ref eina_hash_string_small_new still uses the same hash calculation
 * function that @ref eina_hash_string_superfast_new, which is more complex than
 * @ref eina_hash_string_djb2_new. The latter has a faster hash computation
 * function, but that will imply a not so good distribution. But if just a
 * few keys are being added, this is not a problem, it will still have not many
 * collisions and be faster to calculate the hash than in a hash created with
 * @ref eina_hash_string_small_new and @ref eina_hash_string_superfast_new.
 *
 * A simple comparison between them would be:
 *
 * @li @c djb2 - faster hash function - 256 buckets (higher memory consumption)
 * @li @c string_small - slower hash function but less collisions - 32 buckets
 * (lower memory consumption)
 * @li @c string_superfast - slower hash function but less collisions - 256 buckets
 * (higher memory consumption) - not randomized, avoid it on public remote interface.
 *
 * Basically for a very small number of keys (10 or less), @c djb2 should be
 * used, or @c string_small if you have a restriction on memory usage. And for a
 * higher number of keys, @c string_superfast should be preferred if not used on a
 * public remote interface.
 *
 * If just stringshared keys are being added, use @ref
 * eina_hash_stringshared_new. If a lot of keys will be added to the hash table
 * (e.g. more than 1000), then it's better to increase the buckets_power_size.
 * See @ref eina_hash_new for more details.
 *
 * When adding a new key to a hash table, use @ref eina_hash_add or @ref
 * eina_hash_direct_add (the latter if this key is already stored elsewhere). If
 * the key may be already inside the hash table, rather than checking with
 * @ref eina_hash_find followed by @ref eina_hash_add, one can use just @ref
 * eina_hash_set (this will change the data pointed by this key if it was
 * already present in the table).
 *
 * @section hashtable_tutorial Tutorial
 *
 * These examples show many Eina_Hash functions in action:
 * <ul>
 * <li> @ref hash_01_example_page
 * <li> @ref hash_02_example_page
 * <li> Different types of hash in use:
 *      <ul>
 *      <li> @ref eina_hash_03.c "string small"
 *      <li> @ref eina_hash_04.c "string djb2"
 *      <li> @ref eina_hash_05.c "int32"
 *      <li> @ref eina_hash_06.c "int64"
 *      <li> @ref eina_hash_07.c "pointer"
 *      </ul>
 * <li> @ref eina_hash_08.c "Different add and delete functions"
 * </ul>
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @addtogroup Eina_Containers_Group Containers
 *
 * @{
 */

/**
 * @defgroup Eina_Hash_Group Hash Table
 *
 * @{
 */

/**
 * @typedef Eina_Hash
 * Type for a generic hash table.
 */
typedef struct _Eina_Hash       Eina_Hash;

/**
 * @typedef Eina_Hash_Tuple
 * Type for a hash table of key/value pairs.
 */
typedef struct _Eina_Hash_Tuple Eina_Hash_Tuple;

/**
 * @struct _Eina_Hash_Tuple
 * Data for a hash table of key/value pairs.
 */
struct _Eina_Hash_Tuple
{
   const void  *key; /**< The key */
   void        *data; /**< The data associated to the key */
   unsigned int key_length; /**< The length of the key */
};

/**
 * @typedef Eina_Key_Length
 * Type for a function to determine the length of a hash key.
 */
typedef unsigned int (*Eina_Key_Length)(const void *key);

/**
 * @def EINA_KEY_LENGTH
 * @param Function The function used to calculate length of hash key.
 */
#define EINA_KEY_LENGTH(Function) ((Eina_Key_Length)Function)

/**
 * @typedef Eina_Key_Cmp
 * Type for a function to compare two hash keys.
 */
typedef int          (*Eina_Key_Cmp)(const void *key1, int key1_length, const void *key2, int key2_length);
/**
 * @def EINA_KEY_CMP
 * @param Function The function used to compare hash key.
 */
#define EINA_KEY_CMP(Function)    ((Eina_Key_Cmp)Function)

/**
 * @typedef Eina_Key_Hash
 * Type for a function to create a hash key.
 */
typedef int          (*Eina_Key_Hash)(const void *key, int key_length);
/**
 * @def EINA_KEY_HASH
 * @param Function The function used to hash key.
 */
#define EINA_KEY_HASH(Function)   ((Eina_Key_Hash)Function)

/**
 * @typedef Eina_Hash_Foreach
 * Type for a function to iterate over a hash table.
 */
typedef Eina_Bool    (*Eina_Hash_Foreach)(const Eina_Hash *hash, const void *key, void *data, void *fdata);


/**
 * @brief Creates a new hash table.
 *
 * @param key_length_cb The function called when getting the size of the key.
 * @param key_cmp_cb The function called when comparing the keys.
 * @param key_hash_cb The function called when getting the values.
 * @param data_free_cb The function called on each value when the hash table is
 * freed, or when an item is deleted from it. @c NULL can be passed as a
 * callback.
 * @param buckets_power_size The size of the buckets.
 * @return The new hash table, or @c NULL on failure.
 *
 * This function creates a new hash table using user-defined callbacks
 * to manage the hash table.
 * If @p key_cmp_cb or @p key_hash_cb
 * are @c NULL, @c NULL is returned. If @p buckets_power_size is
 * smaller or equal than 2, or if it is greater or equal than 17,
 * @c NULL is returned.
 *
 * The number of buckets created will be 2 ^ @p buckets_power_size. This means
 * that if @p buckets_power_size is 5, there will be created 32 buckets, whereas for a
 * @p buckets_power_size of 8, there will be 256 buckets.
 *
 * Pre-defined functions are available to create a hash table. See
 * eina_hash_string_djb2_new(), eina_hash_string_superfast_new(),
 * eina_hash_string_small_new(), eina_hash_int32_new(),
 * eina_hash_int64_new(), eina_hash_pointer_new() and
 * eina_hash_stringshared_new().
 */
EAPI Eina_Hash *eina_hash_new(Eina_Key_Length key_length_cb,
                              Eina_Key_Cmp    key_cmp_cb,
                              Eina_Key_Hash   key_hash_cb,
                              Eina_Free_Cb    data_free_cb,
                              int             buckets_power_size) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(2, 3);

/**
 * @brief Sets the data cleanup callback for a hash.
 *
 * @param hash The given hash table.
 * @param data_free_cb The function called on each value when the hash
 * table is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback to remove an existing callback.
 *
 * The argument received by @p data_free_cb will be the data of the item being
 * removed.
 *
 * @since 1.1
 * @see eina_hash_new.
 */
EAPI void eina_hash_free_cb_set(Eina_Hash *hash, Eina_Free_Cb data_free_cb) EINA_ARG_NONNULL(1);

/**
 * @brief Creates a new hash table using the djb2 algorithm.
 *
 * @param data_free_cb The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return The new hash table, or @c NULL on failure.
 *
 * This function creates a new hash table using the djb2 algorithm for
 * table management and strcmp() to compare the keys. Values can then
 * be looked up with pointers other than the original key pointer that
 * was used to add values.
 */
EAPI Eina_Hash *eina_hash_string_djb2_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Creates a new hash table for use with strings.
 *
 * @param data_free_cb The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return The new hash table, or @c NULL on failure.
 *
 * This function creates a new hash table using the superfast algorithm
 * for table management and strcmp() to compare the keys. Values can
 * then be looked up with pointers other than the original key pointer
 * that was used to add values.
 *
 * @warning Don't use this kind of hash when there is a possibility to
 * remotely request and push data in it. This hash is subject to denial
 * of service.
 */
EAPI Eina_Hash *eina_hash_string_superfast_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Creates a new hash table for use with strings with small bucket size.
 *
 * @param data_free_cb  The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return The new hash table, or @c NULL on failure.
 *
 * This function creates a new hash table using the superfast algorithm
 * for table management and strcmp() to compare the keys, but with a
 * smaller bucket size (compared to eina_hash_string_superfast_new())
 * which will minimize the memory used by the returned hash
 * table. Values can then be looked up with pointers other than the
 * original key pointer that was used to add values.
 */
EAPI Eina_Hash *eina_hash_string_small_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Creates a new hash table for use with 32bit integers.
 *
 * @param data_free_cb  The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return  The new hash table, or @c NULL on failure.
 *
 * This function creates a new hash table where keys are 32bit integers.
 * When adding or looking up in the hash table, pointers to 32bit integers
 * must be passed. They can be addresses on the stack if you let the
 * eina_hash copy the key. Values can then
 * be looked up with pointers other than the original key pointer that was
 * used to add values. This method is not suitable to match string keys as
 * it would only match the first character.
 */
EAPI Eina_Hash *eina_hash_int32_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Creates a new hash table for use with 64bit integers.
 *
 * @param data_free_cb  The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return The new hash table, or @c NULL on failure.
 *
 * This function creates a new hash table where keys are 64bit integers.
 * When adding or looking up in the hash table, pointers to 64bit integers
 * must be passed. They can be addresses on the stack. Values can then
 * be looked up with pointers other than the original key pointer that was
 * used to add values. This method is not suitable to match string keys as
 * it would only match the first character.
 */
EAPI Eina_Hash *eina_hash_int64_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Creates a new hash table for use with pointers.
 *
 * @param data_free_cb  The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return The new hash table, or @c NULL on failure.
 *
 * This function creates a new hash table using the int64/int32 algorithm for
 * table management and dereferenced pointers to compare the
 * keys. Values can then be looked up with pointers other than the
 * original key pointer that was used to add values. This method may
 * appear to be able to match string keys, but actually it only matches
 * the first character.
 *
 * @code
 * // For a hash that will have only one pointer to each structure
 * extern Eina_Hash *hash;
 * extern void *data;
 *
 * if (!eina_hash_find(hash, &data))
 *    eina_hash_add(hash, &data, data);
 * @endcode
 */
EAPI Eina_Hash *eina_hash_pointer_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Creates a new hash table optimized for stringshared values.
 *
 * @param data_free_cb  The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return  The new hash table, or @c NULL on failure.
 *
 * This function creates a new hash table optimized for stringshared
 * values. Values CANNOT be looked up with pointers not
 * equal to the original key pointer that was used to add a value.
 *
 * Excerpt of code that will NOT work with this type of hash:
 *
 * @code
 * extern Eina_Hash *hash;
 * extern const char *value;
 * const char *a = eina_stringshare_add("key");
 *
 * eina_hash_add(hash, a, value);
 * eina_hash_find(hash, "key");
 * @endcode
 */
EAPI Eina_Hash *eina_hash_stringshared_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Adds an entry to the given hash table.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key A unique key. Cannot be @c NULL.
 * @param data The data to associate with the string given by @p key. Cannot be @c
 * NULL.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function adds @p key to @p hash. @p key must be unique within
 * the hash table so that @ref eina_hash_find() and @ref eina_hash_del()
 * operate on the correct data item.
 *
 * Key uniqueness varies depending on the type of @p hash: a
 * stringshared @ref Eina_Hash needs unique pointers (which implies
 * unique strings).  All other string hash types require that the
 * strings themselves are unique. Pointer, int32 and int64 hashes need
 * to have unique values. Failure to use sufficient uniqueness will
 * result in unexpected results when inserting data pointers accessed
 * with @ref eina_hash_find(), and removed with @ref eina_hash_del().
 *
 * Key strings are case sensitive.
 */
EAPI Eina_Bool  eina_hash_add(Eina_Hash  *hash,
                              const void *key,
                              const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Adds an entry to the given hash table without duplicating the string.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key A unique key. Cannot be @c NULL.
 * @param data The data to associate with the string given by @p
 * key. Cannot be @c NULL
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function adds @p key to @p hash. @p key must be unique within
 * the hash table so that @ref eina_hash_find() and @ref eina_hash_del()
 * operate on the correct data item.
 *
 * Key uniqueness varies depending on the type of @p hash: a
 * stringshared @ref Eina_Hash needs unique pointers (which implies
 * unique strings).  All other string hash types require that the
 * strings themselves are unique. Pointer, int32 and int64 hashes need
 * to have unique values. Failure to use sufficient uniqueness will
 * result in unexpected results when inserting data pointers accessed
 * with @ref eina_hash_find(), and removed with @ref eina_hash_del().
 *
 * Unlike @ref eina_hash_add(), this function does not make a copy of
 * @p key, so it must be a string constant or stored elsewhere (such as
 * in the object being added). Key strings are case sensitive.
 */
EAPI Eina_Bool eina_hash_direct_add(Eina_Hash  *hash,
                                    const void *key,
                                    const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Removes the entry identified by a key or a data from the given
 * hash table.
 *
 * @param hash The given hash table.
 * @param key  The key.
 * @param data The data pointer to remove if the key is @c NULL.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function removes the entry identified by @p key or @p data
 * from @p hash. If a free function was given to the
 * callback on creation, it will be called for the data being
 * deleted. If @p hash is @c NULL, the function returns immediately #EINA_FALSE.
 * If @p key is @c NULL, then @p data is used to find the a
 * match to remove, otherwise @p key is used and @p data is not
 * required and can be @c NULL.
 *
 * @note If you already have the key, use eina_hash_del_by_key() or
 * eina_hash_del_by_key_hash(). If you don't have the key, use
 * eina_hash_del_by_data() directly.
 */
EAPI Eina_Bool eina_hash_del(Eina_Hash  *hash,
                             const void *key,
                             const void *data) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieves a specific entry in the given hash table.
 *
 * @param hash The given hash table.
 * @param key The key of the entry to find.
 * @return The data pointer for the stored entry on success, or @c NULL
 * otherwise.
 *
 * This function retrieves the entry associated with @p key in
 * @p hash. If @p hash is @c NULL, this function returns @c NULL.
 */
EAPI void *eina_hash_find(const Eina_Hash *hash,
                          const void      *key) EINA_ARG_NONNULL(2);

/**
 * @brief Modifies the entry pointer at the specified key and returns
 * the previous entry.
 * @param hash The given hash table.
 * @param key The key of the entry to modify.
 * @param data The new data.
 * @return The data pointer for the previously stored entry on success,
 * or @c NULL otherwise.
 *
 * This function modifies the data of @p key with @p data in @p
 * hash. If no entry is found, nothing is added to @p hash.
 */
EAPI void *eina_hash_modify(Eina_Hash  *hash,
                            const void *key,
                            const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Modifies the entry pointer at the specified key and returns the
 * previous entry or adds the entry if not found.
 *
 * @param hash The given hash table.
 * @param key The key of the entry to modify.
 * @param data The data to replace the previous entry.
 * @return The data pointer for the previous stored entry, or @c NULL
 * otherwise.
 *
 * This function modifies the value of @p key to @p data in @p
 * hash. If no entry is found, @p data is added to @p hash with the
 * key @p key.
 */
EAPI void *eina_hash_set(Eina_Hash  *hash,
                         const void *key,
                         const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Changes the key of an entry in a hash without triggering the
 * free callback.
 *
 * @param hash    The given hash table.
 * @param old_key The current key associated with the data.
 * @param new_key The new key to associate data with.
 * @return #EINA_FALSE in any case but success, #EINA_TRUE on success.
 *
 * This function moves data from one key to another,
 * but does not call the Eina_Free_Cb associated with the hash table
 * when destroying the old key.
 */
EAPI Eina_Bool eina_hash_move(Eina_Hash  *hash,
                              const void *old_key,
                              const void *new_key) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Frees the given hash table's resources.
 *
 * @param hash The hash table to be freed.
 *
 * This function frees memory allocated for the @p hash and to its
 * internal buckets.
 *
 * If @p data_free_cb was specified at creation time in
 * @ref eina_hash_new, it will be called for each element as it gets
 * freed.  If the callback was not specified, then any data in these
 * elements may now be lost, if not stored or freed elsewhere.
 *
 * If @p hash is @c NULL, the function returns immediately.
 *
 * Example:
 * @code
 * extern Eina_Hash *hash;
 *
 * eina_hash_free(hash);
 * hash = NULL;
 * @endcode
 */
EAPI void      eina_hash_free(Eina_Hash *hash) EINA_ARG_NONNULL(1);

/**
 * @brief Frees the given hash table buckets resources.
 *
 * @param hash The hash table whose buckets have to be freed.
 *
 * This function frees memory allocated to internal buckets for @p hash.
 *
 * If @p data_free_cb was specified at creation time in
 * @ref eina_hash_new(), it will be called for each element as it gets
 * freed. If the callback was not specified, then any data in these
 * elements may now be lost, if not stored or freed elsewhere.
 *
 * If @p hash is @c NULL, the function returns immediately.
 */
EAPI void      eina_hash_free_buckets(Eina_Hash *hash) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the number of entries in the given hash table.
 *
 * @param hash The given hash table.
 * @return The number of entries in the hash table, or @c 0 on error or
 * if @p hash is @c NULL.
 */
EAPI int       eina_hash_population(const Eina_Hash *hash) EINA_ARG_NONNULL(1);

/**
 * @brief Adds an entry to the given hash table by its key hash.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key A unique key. Cannot be @c NULL.
 * @param key_length The length of @p key (including terminating '\\0').
 * @param key_hash The hash of @p key.
 * @param data The data to associate with the string given by the key. Cannot be
 * @c NULL.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function adds @p key to @p hash.
 *
 * @p key must be unique within the hash table so that
 * @ref eina_hash_find() and @ref eina_hash_del() operate on the correct
 * data item.  @p key_hash must match @p key so that the correct item can
 * be found by @ref eina_hash_find_by_hash().  Key strings are case
 * sensitive.
 *
 * @see eina_hash_add()
 */
EAPI Eina_Bool eina_hash_add_by_hash(Eina_Hash  *hash,
                                     const void *key,
                                     int         key_length,
                                     int         key_hash,
                                     const void *data) EINA_ARG_NONNULL(1, 2, 5);

/**
 * @brief Adds an entry to a hash table by its key hash without duplicating the string key.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key A unique key. Cannot be @c NULL.
 * @param key_length The length of @p key (including terminating '\\0').
 * @param key_hash The hash of @p key.
 * @param data The data to associate with the string given by @p key. Cannot be @c
 * NULL.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function adds @p key to @p hash.
 *
 * @p key must be unique within the hash table so that
 * @ref eina_hash_find() and @ref eina_hash_del() operate on the correct
 * data item.  @p key_hash must match @p key so that the correct item can
 * be found by @ref eina_hash_find_by_hash().  Key strings are case
 * sensitive.
 *
 * Unlike @ref eina_hash_add_by_hash(), this function does not make a
 * copy of @p key, so it must be a string constant or stored elsewhere
 * (such as inside the object being added).
 *
 * @see eina_hash_direct_add()
 */
EAPI Eina_Bool eina_hash_direct_add_by_hash(Eina_Hash  *hash,
                                            const void *key,
                                            int         key_length,
                                            int         key_hash,
                                            const void *data) EINA_ARG_NONNULL(1, 2, 5);

/**
 * @brief Removes the entry identified by a key and a key hash from the given
 * hash table.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key The key. Cannot be @c NULL.
 * @param key_length The length of the key (including terminating '\\0').
 * @param key_hash The hash that always matches the key.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function removes the entry identified by @p key and
 * @p key_hash from @p hash. If a free function was given to the
 * callback on creation, it will be called for the data being
 * deleted. If @p hash or @p key are @c NULL, the
 * function returns #EINA_FALSE immediately.
 *
 * @note If you don't have the key_hash, use eina_hash_del_by_key()
 * instead.  If you don't have the key, use eina_hash_del_by_data().
 */
EAPI Eina_Bool eina_hash_del_by_key_hash(Eina_Hash  *hash,
                                         const void *key,
                                         int         key_length,
                                         int         key_hash) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Removes the entry identified by a key from the given hash table.
 *
 * This version will calculate key length and hash by using functions
 * provided to the hash creation function.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key  The key. Cannot be @c NULL.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function removes the entry identified by @p key from @p
 * hash. The key length and hash will be calculated automatically via
 * a function provided to the hash creation function. If a free
 * function was given to the callback on creation, it will be called
 * for the data being deleted. If @p hash or @p key are @c NULL, the
 * function returns #EINA_FALSE immediately.
 *
 * @note If you already have the key_hash, use eina_hash_del_by_key_hash().
 * If you don't have the key, use eina_hash_del_by_data() instead.
 */
EAPI Eina_Bool eina_hash_del_by_key(Eina_Hash  *hash,
                                    const void *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Removes an entry from a hash table identified by its data value.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param data The data value to search and remove. Cannot be @c NULL.
 * @return #EINA_FALSE if an error occurred or if @p hash or @p data are
 * @c NULL, #EINA_TRUE otherwise.
 *          thing goes fine.
 *
 * This function removes the entry identified by @p data from @p
 * hash. If a free function was given to the callback on creation, it
 * will be called for the data being deleted.
 *
 * @note This version is slow since there is no quick access to nodes
 * based on data.
 *
 * @note If you already have the key, use eina_hash_del_by_key()
 * or eina_hash_del_by_key_hash() instead.
 */
EAPI Eina_Bool eina_hash_del_by_data(Eina_Hash  *hash,
                                     const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Removes the entry identified by a key and a key hash, or a
 * data value from the given hash table.
 *
 * If @p key is @c NULL, then @p data is used to find a match to
 * remove.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key The key.
 * @param key_length The length of the key.
 * @param key_hash The hash that always match the key.
 * @param data The data pointer to remove if the key is @c NULL.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function removes the entry identified by @p key and @p key_hash,
 * or @p data, from @p hash. If a free function was given to the
 * callback on creation, it will be called for the data being
 * deleted. If @p hash is @c NULL, the function returns #EINA_FALSE
 * immediately.  If @p key is @c NULL, then @p key_length and @p
 * key_hash are ignored and @p data is used to find a match to remove,
 * otherwise @p key and @p key_hash are used and @p data is not required
 * and can be @c NULL. Do not forget to count '\\0' for string when
 * setting the value of @p key_length.
 *
 * @note If you already have the key, use eina_hash_del_by_key_hash().
 * If you don't have the key, use eina_hash_del_by_data() directly.
 */
EAPI Eina_Bool eina_hash_del_by_hash(Eina_Hash  *hash,
                                     const void *key,
                                     int         key_length,
                                     int         key_hash,
                                     const void *data) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieves a specific entry from the given hash table.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key The key of the entry to find.
 * @param key_length The length of the key.
 * @param key_hash The hash that always matches the key
 * @return The data pointer for the stored entry on success, or @c NULL
 * if @p hash is @c NULL or if the data pointer could not be retrieved.
 *
 * This function retrieves the entry associated with @p key of length
 * @p key_length in @p hash. @p key_hash is the hash that always matches
 * @p key. It is ignored if @p key is @c NULL. Do not forget to count
 * '\\0' for string when setting the value of @p key_length.
 */
EAPI void *eina_hash_find_by_hash(const Eina_Hash *hash,
                                  const void      *key,
                                  int              key_length,
                                  int              key_hash) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Modifies the entry pointer at the specified key and returns
 * the previous entry.
 *
 * @param hash The given hash table.
 * @param key The key of the entry to modify.
 * @param key_length Should be the length of @p key (don't forget to count
 * '\\0' for string).
 * @param key_hash The hash that always matches the key. Ignored if @p
 * key is @c NULL.
 * @param data The data to replace the current entry, if it exists.
 * @return The data pointer for the previously stored entry, or @c NULL
 * if not found. If an existing entry is not found, nothing is added to
 * the hash.
 */
EAPI void *eina_hash_modify_by_hash(Eina_Hash  *hash,
                                    const void *key,
                                    int         key_length,
                                    int         key_hash,
                                    const void *data) EINA_ARG_NONNULL(1, 2, 5);

/**
 * @brief Returns a new iterator associated with hash keys.
 *
 * @param hash The hash.
 * @return A new iterator, or @c NULL if memory could not be allocated.
 *
 * This function returns a newly allocated iterator associated with @p
 * hash. If @p hash is not populated, this function still returns a
 * valid iterator that will always return false on
 * eina_iterator_next().
 *
 * @warning If the hash structure changes then the iterator becomes
 * invalid; adding or removing items may lead to program crash.
 */
EAPI Eina_Iterator *eina_hash_iterator_key_new(const Eina_Hash *hash) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a new iterator associated with a hash.
 *
 * @param hash The hash.
 * @return A new iterator, or @c NULL if memory could not be allocated.
 *
 * This function returns a newly allocated iterator associated with
 * @p hash. If @p hash is not populated, this function still returns a
 * valid iterator that will always return false on
 * eina_iterator_next().
 *
 * @warning If the hash structure changes then the iterator becomes
 * invalid; adding or removing items may lead to program crash.
 */
EAPI Eina_Iterator *eina_hash_iterator_data_new(const Eina_Hash *hash) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returned a new iterator associated with hash keys and data.
 *
 * @param hash The hash.
 * @return A new iterator, or @c NULL if memory could not be allocated.
 *
 * This function returns a newly allocated iterator associated with @p
 * hash. If @p hash is not populated, this function still returns a
 * valid iterator that will always return false on
 * eina_iterator_next().
 *
 * @note Iterator data will provide values as Eina_Hash_Tuple that
 * should not be modified!
 *
 * @warning If the hash structure changes then the iterator becomes
 * invalid; adding or removing items may lead to program crash.
 */
EAPI Eina_Iterator *eina_hash_iterator_tuple_new(const Eina_Hash *hash) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Calls a function on every member stored in the hash table.
 *
 * @param hash The hash table whose members will be walked.
 * @param func The function to call on each parameter.
 * @param fdata The data pointer to pass to the function being called.
 *
 * This function iterates over the hash table @p hash, calling the
 * function @p func on each member. If @p func modifies the contents
 * of the hash table, or wishes to stop processing it should return
 * #EINA_FALSE.  If @p func returns #EINA_TRUE the foreach loop will
 * keep processing.
 *
 * Example:
 * @code
 * extern Eina_Hash *hash;
 *
 * Eina_Bool hash_fn(const Eina_Hash *hash, const void *key,
 *                   void *data, void *fdata)
 * {
 *   printf("Func data: %s, Hash entry: %s / %p\n",
 *          fdata, (const char *)key, data);
 *   return EINA_TRUE;
 * }
 *
 * int main(int argc, char **argv)
 * {
 *   char *hash_fn_data;
 *
 *   hash_fn_data = strdup("Hello World");
 *   eina_hash_foreach(hash, hash_fn, hash_fn_data);
 *   free(hash_fn_data);
 * }
 * @endcode
 */
EAPI void           eina_hash_foreach(const Eina_Hash  *hash,
                                      Eina_Hash_Foreach func,
                                      const void       *fdata) EINA_ARG_NONNULL(1, 2);


/**
 * @brief Appends data to an #Eina_List inside a hash.
 *
 * This function is identical to the sequence of calling
 * eina_hash_find(), eina_list_append(), eina_hash_set(),
 * but with one fewer required hash lookup.
 * @param hash The hash table.
 * @param key The key associated with the data.
 * @param data The data to append to the list.
 * @since 1.10
 */
EAPI void eina_hash_list_append(Eina_Hash *hash, const void *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Prepends data to an #Eina_List inside a hash.
 *
 * This function is identical to the sequence of calling
 * eina_hash_find(), eina_list_prepend(), eina_hash_set(),
 * but with one fewer required hash lookup.
 * @param hash The hash table.
 * @param key The key associated with the data.
 * @param data The data to prepend to the list.
 * @since 1.10
 */
EAPI void eina_hash_list_prepend(Eina_Hash *hash, const void *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Removes data from an #Eina_List inside a hash.
 *
 * This function is identical to the sequence of calling
 * eina_hash_find(), eina_list_remove(), eina_hash_set(),
 * but with one fewer required hash lookup.
 * @param hash The hash table.
 * @param key The key associated with the data.
 * @param data The data to remove from the list.
 * @since 1.10
 */
EAPI void eina_hash_list_remove(Eina_Hash *hash, const void *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief
 * Paul Hsieh (http://www.azillionmonkeys.com/qed/hash.html) hash function used by WebCore (http://webkit.org/blog/8/hashtables-part-2/)
 *
 * @param key The key to hash.
 * @param len The length of the key.
 * @return The hash value.
 */
EAPI int eina_hash_superfast(const char *key,
                             int         len) EINA_ARG_NONNULL(1);

/**
 * @brief
 * Hash function first reported by Dan Bernstein many years ago in comp.lang.c
 *
 * @param key The key to hash.
 * @param len The length of the key.
 * @return The hash value.
 */
static inline int eina_hash_djb2(const char *key,
                                 int         len) EINA_ARG_NONNULL(1);

/**
 * @brief
 * Hash function first reported by Dan Bernstein many years ago in comp.lang.c
 *
 * @param key The key to hash.
 * @param plen The length of the key.
 * @return The hash value.
 */
static inline int eina_hash_djb2_len(const char *key,
                                     int        *plen) EINA_ARG_NONNULL(1, 2);

/**
 * @brief
 * Hash function from http://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
 *
 * @param pkey The key to hash.
 * @param len The length of the key.
 * @return The hash value.
 */
static inline int eina_hash_int32(const unsigned int *pkey,
                                  int                 len) EINA_ARG_NONNULL(1);

/**
 * @brief
 * Hash function from http://web.archive.org/web/20071223173210/http://www.concentric.net/~Ttwang/tech/inthash.htm
 *
 * @param pkey The key to hash.
 * @param len The length of the key.
 * @return The hash value.
 */
static inline int eina_hash_int64(const unsigned long long int *pkey,
                                  int                      len) EINA_ARG_NONNULL(1);

/**
 * @brief
 * Hash function from http://sites.google.com/site/murmurhash/
 *
 * @param key The key to hash.
 * @param len The length of the key.
 * @return The hash value.
 */
static inline int eina_hash_murmur3(const char *key,
                           int         len) EINA_ARG_NONNULL(1);

/**
 * @brief
 * Hash function using crc-32 algorithm and and 0xEDB88320 polynomial
 *
 * @param key The key to hash.
 * @param len The length of the key.
 * @return The hash value.
 */
static inline int eina_hash_crc(const char *key,
                           int         len) EINA_ARG_NONNULL(1);

#include "eina_inline_hash.x"

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /*EINA_HASH_H_*/
