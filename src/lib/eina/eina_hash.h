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
 * two of them: using @ref eina_hash_foreach and @ref Eina_Iterator .
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
 * @brief Hash table management. Useful for mapping keys to values.
 *
 * The hash table is useful for when one wants to implement a table that maps
 * keys (usually strings) to data, and have relatively fast access time. The
 * performance is proportional to the load factor of the table (number of
 * elements / number of buckets). See @ref hashtable_algo for implementation
 * details.
 *
 * Different implementations exists depending on what kind of key will be used
 * to access the data: strings, integers, pointers, stringshared or your own.
 *
 * Eina hash tables can copy the keys when using eina_hash_add() or not when
 * using eina_hash_direct_add().
 *
 * @section hashtable_algo Algorithm
 *
 * The Eina_Hash is implemented using an array of N "buckets", where each
 * bucket is a pointer to a structure that is the head of a <a
 * href="http://en.wikipedia.org/wiki/Red-black_tree">red-black tree</a>. The
 * array can then be indexed by the [hash_of_element mod N]. The
 * hash_of_element is calculated using the hashing function, passed as
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
 * Adding an element to the hash table is made of:
 * @li calculating the hash for that key (using the specified hash function);
 * @li calculate the array position [hash mod N];
 * @li add the element to the rbtree on that position.
 *
 * The two first steps have constant time, proportional to the hash function
 * being used. Adding the key to the rbtree will be proportional on the number
 * of keys on that bucket.
 *
 * The average cost of lookup depends on the number of keys per
 * bucket (load factor) of the table, if the distribution of keys is
 * sufficiently uniform.
 *
 * @section hashtable_perf Performance
 *
 * As said before, the performance depends on the load factor. So trying to keep
 * the load factor as small as possible will improve the hash table performance. But
 * increasing the buckets_power_size will also increase the memory consumption.
 * The default hash table creation functions already have a good number of
 * buckets, enough for most cases. Particularly for strings, if just a few keys
 * (less than 30) will be added to the hash table, @ref
 * eina_hash_string_small_new should be used, since it will reduce the memory
 * consumption for the buckets, and you still won't have many collisions.
 * However, @ref eina_hash_string_small_new still uses the same hash calculation
 * function that @ref eina_hash_string_superfast_new, which is more complex than
 * @ref eina_hash_string_djb2_new. The latter has a faster hash computation
 * function, but that will imply on a not so good distribution. But if just a
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
 * the key may be already inside the hash table, instead of checking with
 * @ref eina_hash_find and then doing @ref eina_hash_add, one can use just @ref
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

typedef struct _Eina_Hash_Tuple Eina_Hash_Tuple;

struct _Eina_Hash_Tuple
{
   const void  *key; /**< The key */
   void        *data; /**< The data associated to the key */
   unsigned int key_length; /**< The length of the key */
};

typedef unsigned int (*Eina_Key_Length)(const void *key);
/**
 * @def EINA_KEY_LENGTH
 * @param Function The function used to calculate length of hash key.
 */
#define EINA_KEY_LENGTH(Function) ((Eina_Key_Length)Function)
typedef int          (*Eina_Key_Cmp)(const void *key1, int key1_length, const void *key2, int key2_length);
/**
 * @def EINA_KEY_CMP
 * @param Function The function used to compare hash key.
 */
#define EINA_KEY_CMP(Function)    ((Eina_Key_Cmp)Function)
typedef int          (*Eina_Key_Hash)(const void *key, int key_length);
/**
 * @def EINA_KEY_HASH
 * @param Function The function used to hash key.
 */
#define EINA_KEY_HASH(Function)   ((Eina_Key_Hash)Function)
typedef Eina_Bool    (*Eina_Hash_Foreach)(const Eina_Hash *hash, const void *key, void *data, void *fdata);


/**
 * @brief Create a new hash table.
 *
 * @param key_length_cb The function called when getting the size of the key.
 * @param key_cmp_cb The function called when comparing the keys.
 * @param key_hash_cb The function called when getting the values.
 * @param data_free_cb The function called on each value when the hash table is
 * freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @param buckets_power_size The size of the buckets.
 * @return The new hash table.
 *
 * This function creates a new hash table using user-defined callbacks
 * to manage the hash table. On failure, @c NULL is returned.
 * If @p key_cmp_cb or @p key_hash_cb
 * are @c NULL, @c NULL is returned. If @p buckets_power_size is
 * smaller or equal than 2, or if it is greater or equal than 17,
 * @c NULL is returned.
 *
 * The number of buckets created will be 2 ^ @p buckets_power_size. This means
 * that if @p buckets_power_size is 5, there will be created 32 buckets. for a
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
 * @brief Redefine the callback that clean the data of a hash
 *
 * @param hash The given hash table
 * @param data_free_cb The function called on each value when the hash
 * table is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback to remove an existing callback.
 *
 * The argument received by @p data_free_cb will be that data of the item being
 * removed.
 *
 * @since 1.1
 * @see eina_hash_new.
 */
EAPI void eina_hash_free_cb_set(Eina_Hash *hash, Eina_Free_Cb data_free_cb) EINA_ARG_NONNULL(1);

/**
 * @brief Create a new hash table using the djb2 algorithm.
 *
 * @param data_free_cb The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return The new hash table.
 *
 * This function creates a new hash table using the djb2 algorithm for
 * table management and strcmp() to compare the keys. Values can then
 * be looked up with pointers other than the original key pointer that
 * was used to add values. On failure, this function returns @c NULL.
 */
EAPI Eina_Hash *eina_hash_string_djb2_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Create a new hash table for use with strings.
 *
 * @param data_free_cb The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return The new hash table.
 *
 * This function creates a new hash table using the superfast algorithm
 * for table management and strcmp() to compare the keys. Values can
 * then be looked up with pointers other than the original key pointer
 * that was used to add values. On failure, this function returns
 * @c NULL.
 *
 * NOTE: don't use this kind of hash when their is a possibility to remotely
 * request and push data in it. This hash is subject to denial of service.
 */
EAPI Eina_Hash *eina_hash_string_superfast_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Create a new hash table for use with strings with small bucket size.
 *
 * @param data_free_cb  The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return  The new hash table.
 *
 * This function creates a new hash table using the superfast algorithm
 * for table management and strcmp() to compare the keys, but with a
 * smaller bucket size (compared to eina_hash_string_superfast_new())
 * which will minimize the memory used by the returned hash
 * table. Values can then be looked up with pointers other than the
 * original key pointer that was used to add values. On failure, this
 * function returns @c NULL.
 */
EAPI Eina_Hash *eina_hash_string_small_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Create a new hash table for use with 32bit integers.
 *
 * @param data_free_cb  The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return  The new hash table.
 *
 * This function creates a new hash table where keys are 32bit integers.
 * When adding or looking up in the hash table, pointers to 32bit integers
 * must be passed. They can be addresses on the stack if you let the
 * eina_hash copy the key. Values can then
 * be looked up with pointers other than the original key pointer that was
 * used to add values. This method is not suitable to match string keys as
 * it would only match the first character.
 * On failure, this function returns @c NULL.
 */
EAPI Eina_Hash *eina_hash_int32_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Create a new hash table for use with 64bit integers.
 *
 * @param data_free_cb  The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return  The new hash table.
 *
 * This function creates a new hash table where keys are 64bit integers.
 * When adding or looking up in the hash table, pointers to 64bit integers
 * must be passed. They can be addresses on the stack. Values can then
 * be looked up with pointers other than the original key pointer that was
 * used to add values. This method is not suitable to match string keys as
 * it would only match the first character.
 * On failure, this function returns @c NULL.
 */
EAPI Eina_Hash *eina_hash_int64_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Create a new hash table for use with pointers.
 *
 * @param data_free_cb  The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return  The new hash table.
 *
 * This function creates a new hash table using the int64/int32 algorithm for
 * table management and dereferenced pointers to compare the
 * keys. Values can then be looked up with pointers other than the
 * original key pointer that was used to add values. This method may
 * appear to be able to match string keys, actually it only matches
 * the first character. On failure, this function returns @c NULL.
 */
EAPI Eina_Hash *eina_hash_pointer_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Create a new hash table optimized for stringshared values.
 *
 * @param data_free_cb  The function called on each value when the hash table
 * is freed, or when an item is deleted from it. @c NULL can be passed as
 * callback.
 * @return  The new hash table.
 *
 * This function creates a new hash table optimized for stringshared
 * values. Values CAN NOT be looked up with pointers not
 * equal to the original key pointer that was used to add a value. On failure,
 * this function returns @c NULL.
 *
 * Excerpt of code that will NOT work with this type of hash:
 *
 * @code
 * extern Eina_Hash *hash;
 * extern const char *value;
 * const char *a = eina_stringshare_add("key");
 *
 * eina_hash_add(hash, a, value);
 * eina_hash_find(hash, "key")
 * @endcode
 */
EAPI Eina_Hash *eina_hash_stringshared_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Add an entry to the given hash table.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key A unique key. Cannot be @c NULL.
 * @param data Data to associate with the string given by @p key. Cannot be @c
 * NULL.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function adds @p key to @p hash. @p key is
 * expected to be unique within the hash table. Key uniqueness varies
 * depending on the type of @p hash: a stringshared @ref Eina_Hash
 * need to have unique pointers (which implies unique strings).
 * All other string hash types require the strings
 * themselves to be unique. Pointer, int32 and int64 hashes need to have these
 * values as unique. Failure to use sufficient uniqueness will
 * result in unexpected results when inserting data pointers accessed
 * with eina_hash_find(), and removed with eina_hash_del(). Key
 * strings are case sensitive. This function returns #EINA_FALSE if an error
 * occurred, #EINA_TRUE otherwise.
 */
EAPI Eina_Bool  eina_hash_add(Eina_Hash  *hash,
                              const void *key,
                              const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Add an entry to the given hash table without duplicating the string
 * key.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key A unique key. Cannot be @c NULL.
 * @param data Data to associate with the string given by @p key. Cannot be @c
 * NULL
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function adds @p key to @p hash. @p key is
 * expected to be unique within the hash table. Key uniqueness varies
 * depending on the type of @p hash: a stringshared @ref Eina_Hash
 * need have unique pointers (which implies unique strings).
 * All other string hash types require the strings
 * themselves to be unique. Pointer, int32 and int64 hashes need to have these
 * values as unique. Failure to use sufficient uniqueness will
 * result in unexpected results when inserting data pointers accessed
 * with eina_hash_find(), and removed with eina_hash_del(). This
 * function does not make a copy of @p key, so it must be a string
 * constant or stored elsewhere ( in the object being added). Key
 * strings are case sensitive. This function returns #EINA_FALSE if an error
 * occurred, #EINA_TRUE otherwise.
 */
EAPI Eina_Bool eina_hash_direct_add(Eina_Hash  *hash,
                                    const void *key,
                                    const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Remove the entry identified by a key or a data from the given
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
 * deleted. If @p hash is @c NULL, the functions returns immediately #EINA_FALSE.
 * If @p key is @c NULL, then @p data is used to find the a
 * match to remove, otherwise @p key is used and @p data is not
 * required and can be @c NULL. This function returns #EINA_FALSE if
 * an error occurred, #EINA_TRUE otherwise.
 *
 * @note if you know you already have the key, use
 *       eina_hash_del_by_key() or eina_hash_del_by_key_hash(). If you
 *       know you don't have the key, use eina_hash_del_by_data()
 *       directly.
 */
EAPI Eina_Bool eina_hash_del(Eina_Hash  *hash,
                             const void *key,
                             const void *data) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieve a specific entry in the given hash table.
 *
 * @param hash The given hash table.
 * @param key The key of the entry to find.
 * @return The data pointer for the stored entry on success, @c NULL
 * otherwise.
 *
 * This function retrieves the entry associated to @p key in
 * @p hash. If @p hash is @c NULL, this function returns immediately
 * @c NULL. This function returns the data pointer on success, @c NULL
 * otherwise.
 */
EAPI void *eina_hash_find(const Eina_Hash *hash,
                          const void      *key) EINA_ARG_NONNULL(2);

/**
 * @brief Modify the entry pointer at the specified key and return the old
 * entry.
 * @param hash The given hash table.
 * @param key The key of the entry to modify.
 * @param data The data to replace the old entry.
 * @return The data pointer for the old stored entry on success, or
 * @c NULL otherwise.
 *
 * This function modifies the data of @p key with @p data in @p
 * hash. If no entry is found, nothing is added to @p hash. On success
 * this function returns the old entry, otherwise it returns @c NULL.
 */
EAPI void *eina_hash_modify(Eina_Hash  *hash,
                            const void *key,
                            const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Modify the entry pointer at the specified key and return the
 * old entry or add the entry if not found.
 *
 * @param hash The given hash table.
 * @param key The key of the entry to modify.
 * @param data The data to replace the old entry
 * @return The data pointer for the old stored entry, or @c NULL
 * otherwise.
 *
 * This function modifies the data of @p key with @p data in @p
 * hash. If no entry is found, @p data is added to @p hash with the
 * key @p key. On success this function returns the old entry,
 * otherwise it returns @c NULL.
 */
EAPI void *eina_hash_set(Eina_Hash  *hash,
                         const void *key,
                         const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Change the key associated with a data without triggering the
 * free callback.
 *
 * @param hash    The given hash table.
 * @param old_key The current key associated with the data
 * @param new_key The new key to associate data with
 * @return #EINA_FALSE in any case but success, #EINA_TRUE on success.
 *
 * This function allows for the move of data from one key to another,
 * but does not call the Eina_Free_Cb associated with the hash table
 * when destroying the old key.
 */
EAPI Eina_Bool eina_hash_move(Eina_Hash  *hash,
                              const void *old_key,
                              const void *new_key) EINA_ARG_NONNULL(1, 2, 3);

/**
 * Free the given hash table resources.
 *
 * @param hash The hash table to be freed.
 *
 * This function frees up all the memory allocated to storing @p hash,
 * and call the free callback if it has been passed to the hash table
 * at creation time. If no free callback has been passed, any entries
 * in the table that the program has no more pointers for elsewhere
 * may now be lost, so this should only be called if the program has
 * already freed any allocated data in the hash table or has the
 * pointers for data in the table stored elsewhere as well. If @p hash
 * is @c NULL, the function returns immediately.
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
 * Free the given hash table buckets resources.
 *
 * @param hash The hash table whose buckets have to be freed.
 *
 * This function frees up all the memory allocated to storing the
 * buckets of @p hash, and calls the free callback on all hash table
 * buckets if it has been passed to the hash table at creation time,
 * then frees the buckets. If no free callback has been passed, no
 * buckets value will be freed. If @p hash is @c NULL, the function
 * returns immediately.
 */
EAPI void      eina_hash_free_buckets(Eina_Hash *hash) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the number of entries in the given hash table.
 *
 * @param hash The given hash table.
 * @return The number of entries in the hash table.
 *
 * This function returns the number of entries in @p hash, or 0 on
 * error. If @p hash is @c NULL, @c 0 is returned.
 */
EAPI int       eina_hash_population(const Eina_Hash *hash) EINA_ARG_NONNULL(1);

/**
 * @brief Add an entry to the given hash table.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key A unique key. Cannot be @c NULL.
 * @param key_length The length of the key.
 * @param key_hash The hash that will always match key.
 * @param data The data to associate with the string given by the key. Cannot be
 * @c NULL.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function adds @p key to @p hash. @p hash, @p key and @p data
 * cannot be @c NULL, in that case #EINA_FALSE is returned. @p key is
 * expected to be a unique within the hash table. Otherwise,
 * one cannot be sure which inserted data pointer will be accessed
 * with @ref eina_hash_find, and removed with @ref eina_hash_del. Do
 * not forget to count '\\0' for string when setting the value of
 * @p key_length. @p key_hash is expected to always match
 * @p key. Otherwise, one cannot be sure to find it again with @ref
 * eina_hash_find_by_hash. Key strings are case sensitive. This function
 * returns #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * @see eina_hash_add()
 */
EAPI Eina_Bool eina_hash_add_by_hash(Eina_Hash  *hash,
                                     const void *key,
                                     int         key_length,
                                     int         key_hash,
                                     const void *data) EINA_ARG_NONNULL(1, 2, 5);

/**
 * @brief Add an entry to the given hash table and do not duplicate the string
 * key.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key A unique key. Cannot be @c NULL.
 * @param key_length Should be the length of @p key (don't forget to count
 * '\\0' for string).
 * @param key_hash The hash that will always match key.
 * @param data Data to associate with the string given by @p key. Cannot be @c
 * NULL.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function adds @p key to @p hash. @p hash, @p key and @p data
 * can be @c NULL, in that case #EINA_FALSE is returned. @p key is
 * expected to be unique within the hash table. Otherwise,
 * one cannot be sure which inserted data pointer will be accessed
 * with @ref eina_hash_find, and removed with @ref eina_hash_del. This
 * function does not make a copy of @p key so it must be a string
 * constant or stored elsewhere (in the object being added). Do
 * not forget to count '\\0' for string when setting the value of
 * @p key_length. @p key_hash is expected to always match
 * @p key. Otherwise, one cannot be sure to find it again with @ref
 * eina_hash_find_by_hash. Key strings are case sensitive. This function
 * returns #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * @see eina_hash_direct_add()
 */
EAPI Eina_Bool eina_hash_direct_add_by_hash(Eina_Hash  *hash,
                                            const void *key,
                                            int         key_length,
                                            int         key_hash,
                                            const void *data) EINA_ARG_NONNULL(1, 2, 5);

/**
 * @brief Remove the entry identified by a key and a key hash from the given
 * hash table.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key The key. Cannot be @c NULL.
 * @param key_length The length of the key.
 * @param key_hash The hash that always match the key.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function removes the entry identified by @p key and
 * @p key_hash from @p hash. If a free function was given to the
 * callback on creation, it will be called for the data being
 * deleted. Do not forget to count '\\0' for string when setting the
 * value of @p key_length. If @p hash or @p key are @c NULL, the
 * functions returns immediately #EINA_FALSE. This function
 * returns #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * @note if you don't have the key_hash, use eina_hash_del_by_key() instead.
 * @note if you don't have the key, use eina_hash_del_by_data() instead.
 */
EAPI Eina_Bool eina_hash_del_by_key_hash(Eina_Hash  *hash,
                                         const void *key,
                                         int         key_length,
                                         int         key_hash) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Remove the entry identified by a key from the given hash table.
 *
 * This version will calculate key length and hash by using functions
 * provided to hash creation function.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key  The key. Cannot be @c NULL.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function removes the entry identified by @p key from @p
 * hash. The key length and hash will be calculated automatically by
 * using functiond provided to has creation function. If a free
 * function was given to the callback on creation, it will be called
 * for the data being deleted. If @p hash or @p key are @c NULL, the
 * functions returns immediately #EINA_FALSE. This function
 * returns #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * @note if you already have the key_hash, use eina_hash_del_by_key_hash()
 * instead.
 * @note if you don't have the key, use eina_hash_del_by_data() instead.
 */
EAPI Eina_Bool eina_hash_del_by_key(Eina_Hash  *hash,
                                    const void *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Remove the entry identified by a data from the given hash table.
 *
 * This version is slow since there is no quick access to nodes based on data.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param data The data value to search and remove. Cannot be @c NULL.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *          thing goes fine.
 *
 * This function removes the entry identified by @p data from @p
 * hash. If a free function was given to the callback on creation, it
 * will be called for the data being deleted. If @p hash or @p data
 * are @c NULL, the functions returns immediately #EINA_FALSE. This
 * function returns #EINA_FALSE if an error occurred, #EINA_TRUE
 * otherwise.
 *
 * @note if you already have the key, use eina_hash_del_by_key() or
 * eina_hash_del_by_key_hash() instead.
 */
EAPI Eina_Bool eina_hash_del_by_data(Eina_Hash  *hash,
                                     const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Remove the entry identified by a key and a key hash or a
 * data from the given hash table.
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
 * This function removes the entry identified by @p key and
 * @p key_hash, or @p data, from @p hash. If a free function was given to
 * the  callback on creation, it will be called for the data being
 * deleted. If @p hash is @c NULL, the functions returns immediately #EINA_FALSE.
 * If @p key is @c NULL, then @p key_length and @p key_hash
 * are ignored and @p data is used to find a match to remove,
 * otherwise @p key and @p key_hash are used and @p data is not
 * required and can be @c NULL. Do not forget to count '\\0' for
 * string when setting the value of @p key_length. This function
 * returns #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * @note if you know you already have the key, use eina_hash_del_by_key_hash(),
 *       if you know you don't have the key, use eina_hash_del_by_data()
 *       directly.
 */
EAPI Eina_Bool eina_hash_del_by_hash(Eina_Hash  *hash,
                                     const void *key,
                                     int         key_length,
                                     int         key_hash,
                                     const void *data) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieve a specific entry in the given hash table.
 *
 * @param hash The given hash table. Cannot be @c NULL.
 * @param key The key of the entry to find.
 * @param key_length The length of the key.
 * @param key_hash The hash that always match the key
 * @return The data pointer for the stored entry on success, @c NULL
 * otherwise.
 *
 * This function retrieves the entry associated to @p key of length
 * @p key_length in @p hash. @p key_hash is the hash that always match
 * @p key. It is ignored if @p key is @c NULL. Do not forget to count
 * '\\0' for string when setting the value of @p key_length. If
 * @p hash is @c NULL, this function returns immediately @c NULL. This
 * function returns the data pointer on success, @c NULL otherwise.
 */
EAPI void *eina_hash_find_by_hash(const Eina_Hash *hash,
                                  const void      *key,
                                  int              key_length,
                                  int              key_hash) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Modify the entry pointer at the specified key and returns
 * the old entry.
 *
 * @param hash The given hash table.
 * @param key The key of the entry to modify.
 * @param key_length Should be the length of @p key (don't forget to count
 * '\\0' for string).
 * @param key_hash The hash that always match the key. Ignored if @p key is
 *                 @c NULL.
 * @param data The data to replace the old entry, if it exists.
 * @return The data pointer for the old stored entry, or @c NULL if not
 *          found. If an existing entry is not found, nothing is added to the
 *          hash.
 */
EAPI void *eina_hash_modify_by_hash(Eina_Hash  *hash,
                                    const void *key,
                                    int         key_length,
                                    int         key_hash,
                                    const void *data) EINA_ARG_NONNULL(1, 2, 5);

/**
 * @brief Returned a new iterator associated to hash keys.
 *
 * @param hash The hash.
 * @return A new iterator.
 *
 * This function returns a newly allocated iterator associated to @p
 * hash. If @p hash is not populated, this function still returns a
 * valid iterator that will always return false on
 * eina_iterator_next(), thus keeping API sane.
 *
 * If the memory can not be allocated, NULL is returned.
 * Otherwise, a valid iterator is returned.
 *
 * @warning if the hash structure changes then the iterator becomes
 *    invalid! That is, if you add or remove items this iterator
 *    behavior is undefined and your program may crash!
 */
EAPI Eina_Iterator *eina_hash_iterator_key_new(const Eina_Hash *hash) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returned a new iterator associated to hash data.
 *
 * @param hash The hash.
 * @return A new iterator.
 *
 * This function returns a newly allocated iterator associated to
 * @p hash. If @p hash is not populated, this function still returns a
 * valid iterator that will always return false on
 * eina_iterator_next(), thus keeping API sane.
 *
 * If the memory can not be allocated, @c NULL is returned.
 * Otherwise, a valid iterator is returned.
 *
 * @warning if the hash structure changes then the iterator becomes
 * invalid. That is, if you add or remove items this iterator behavior
 * is undefined and your program may crash.
 */
EAPI Eina_Iterator *eina_hash_iterator_data_new(const Eina_Hash *hash) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returned a new iterator associated to hash keys and data.
 *
 * @param hash The hash.
 * @return A new iterator.
 *
 * This function returns a newly allocated iterator associated to @p
 * hash. If @p hash is not populated, this function still returns a
 * valid iterator that will always return false on
 * eina_iterator_next(), thus keeping API sane.
 *
 * If the memory can not be allocated, @c NULL is returned.
 * Otherwise, a valid iterator is returned.
 *
 * @note iterator data will provide values as Eina_Hash_Tuple that should not
 *   be modified!
 *
 * @warning if the hash structure changes then the iterator becomes
 *    invalid! That is, if you add or remove items this iterator
 *    behavior is undefined and your program may crash!
 */
EAPI Eina_Iterator *eina_hash_iterator_tuple_new(const Eina_Hash *hash) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Call a function on every member stored in the hash table
 *
 * @param hash The hash table whose members will be walked
 * @param func The function to call on each parameter
 * @param fdata The data pointer to pass to the function being called
 *
 * This function goes through every entry in the hash table @p hash and calls
 * the function @p func on each member. The function should @b not modify the
 * hash table contents if it returns @c 1. @b If the hash table contents are
 * modified by this function or the function wishes to stop processing it must
 * return @c 0, otherwise return @c 1 to keep processing.
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
 *   return 1;
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
/* Paul Hsieh (http://www.azillionmonkeys.com/qed/hash.html) hash function used by WebCore (http://webkit.org/blog/8/hashtables-part-2/) */
EAPI int eina_hash_superfast(const char *key,
                             int         len) EINA_ARG_NONNULL(1);
/* Hash function first reported by dan bernstein many years ago in comp.lang.c */
static inline int eina_hash_djb2(const char *key,
                                 int         len) EINA_ARG_NONNULL(1);
static inline int eina_hash_djb2_len(const char *key,
                                     int        *plen) EINA_ARG_NONNULL(1, 2);
/* Hash function from http://www.concentric.net/~Ttwang/tech/inthash.htm */
static inline int eina_hash_int32(const unsigned int *pkey,
                                  int                 len) EINA_ARG_NONNULL(1);
static inline int eina_hash_int64(const unsigned long long int *pkey,
                                  int                      len) EINA_ARG_NONNULL(1);
/* http://sites.google.com/site/murmurhash/ */
static inline int eina_hash_murmur3(const char *key,
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
