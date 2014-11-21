/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Gustavo Sverzut Barbieri,
 *                         Vincent Torri, Jorge Luis Zapata Muga, Cedric Bail
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

#ifndef EINA_HASH_H_
#define EINA_HASH_H_

#include "eina_types.h"
#include "eina_iterator.h"

/**
 * @defgroup Eina_Hash_Group Hash Table
 * @ingroup Eina_Containers_Group
 *
 * @brief Performs hash table management. It is useful for mapping keys to values.
 *
 * The hash table is useful when one wants to implement a table that maps
 * keys (usually strings) to data, and have relatively fast access time. The
 * performance is proportional to the load factor of the table (number of
 * elements / number of buckets). See @ref hashtable_algo for implementation
 * details.
 *
 * Different implementations exist depending on what kind of key is used
 * to access the data: strings, integers, pointers, stringshared, or your own key.
 *
 * Eina hash tables can copy keys by using eina_hash_add() or not copy by
 * using eina_hash_direct_add().
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
 * (argument of @ref eina_hash_new too). The following picture ilustrates the
 * basic idea:
 *
 * @image html 01_hash-table.png
 * @image latex 01_hash-table.eps
 *
 * Adding an element to the hash table consists of:
 * @li Calculating the hash for that key (using the specified hash function);
 * @li Calculating the array position [hash mod N];
 * @li Adding the element to the rbtree at that position.
 *
 * The first two steps have constant time, proportional to the hash function
 * being used. Adding the key to the rbtree is proportional to the number
 * of keys on that bucket.
 *
 * The average cost of lookup depends on the number of keys per
 * bucket (load factor) of the table, if the distribution of the keys is
 * sufficiently uniform.
 *
 * @section hashtable_perf Performance
 *
 * As said before, the performance depends on the load factor. So trying to keep
 * the load factor as small as possible improves the hash table performance. But
 * increasing the buckets_power_size also increases the memory consumption.
 * The default hash table creation functions already have a good number of
 * buckets, enough for most cases. Particularly for strings, if just a few keys
 * (less than 30) are added to the hash table, @ref
 * eina_hash_string_small_new should be used, since it reduces the memory
 * consumption for the buckets, and you still won't have many collisions.
 * However, @ref eina_hash_string_small_new still uses the same hash calculation
 * function that @ref eina_hash_string_superfast_new uses, which is more complex than
 * @ref eina_hash_string_djb2_new. The latter has a faster hash computation
 * function, but that implies to a not so good distribution. But if just a
 * few keys are being added, this is not a problem, it still does not have many
 * collisions and is faster in calculating the hash than when a hash is created with
 * @ref eina_hash_string_small_new and @ref eina_hash_string_superfast_new.
 *
 * A simple comparison between them would be:
 *
 * @li @c djb2 - faster hash function - 256 buckets (higher memory consumption)
 * @li @c string_small - slower hash function but less collisions - 32 buckets
 * (lower memory consumption)
 * @li @c string_superfast - slower hash function but less collisions - 256 buckets
 * (higher memory consumption)
 *
 * Basically for a very small number of keys (10 or less), @c djb2 should be
 * used, or @c string_small if you have a restriction on memory usage. And for a
 * higher number of keys, @c string_superfast should always be preferred.
 *
 * If just stringshared keys are being added, use @ref
 * eina_hash_stringshared_new. If a lot of keys are added to the hash table
 * (e.g. more than 1000), then it's better to increase the buckets_power_size.
 * See @ref eina_hash_new for more details.
 *
 * When adding a new key to a hash table, use @ref eina_hash_add or @ref
 * eina_hash_direct_add (the latter is if this key is already stored elsewhere). If
 * the key may be already inside the hash table, instead of checking with
 * @ref eina_hash_find and then doing @ref eina_hash_add, one can use just @ref
 * eina_hash_set (this changes the data pointed by this key if it is
 * already present in the table).
 *
 * @{
 */

/**
 * @typedef Eina_Hash
 * @brief The structure type for a generic hash table.
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
 * @param Function The function used to calculate the length of the hash key
 */
#define EINA_KEY_LENGTH(Function) ((Eina_Key_Length)Function)

/**
 * @typedef Eina_Key_Cmp
 * Type for a function to compare two hash keys.
 */
typedef int          (*Eina_Key_Cmp)(const void *key1, int key1_length, const void *key2, int key2_length);
/**
 * @def EINA_KEY_CMP
 * @param Function The function used to compare the hash key
 */
#define EINA_KEY_CMP(Function)    ((Eina_Key_Cmp)Function)

/**
 * @typedef Eina_Key_Hash
 * Type for a function to create a hash key.
 */
typedef int          (*Eina_Key_Hash)(const void *key, int key_length);
/**
 * @def EINA_KEY_HASH
 * @param Function The function used to hash the key.
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
 * @details This function creates a new hash table using user-defined callbacks
 *          to manage the hash table. On failure, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. If @a key_cmp_cb or @a key_hash_cb
 *          are @c NULL, @c NULL is returned. If @a buckets_power_size is
 *          smaller than or equal to 2, or if it is greater than or equal to 17,
 *          @c NULL is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks The number of buckets created are 2 ^ @a buckets_power_size. This means
 *          that if @a buckets_power_size is 5, it creates 32 buckets. For a
 *          @a buckets_power_size of 8, it creates 256 buckets.
 *
 * @remarks Pre-defined functions are available to create a hash table. See
 *          eina_hash_string_djb2_new(), eina_hash_string_superfast_new(),
 *          eina_hash_string_small_new(), eina_hash_int32_new(),
 *          eina_hash_int64_new(), eina_hash_pointer_new(), and
 *          eina_hash_stringshared_new().
 *
 * @param[in] key_length_cb The function called when getting the size of the key
 * @param[in] key_cmp_cb The function called when comparing the keys
 * @param[in] key_hash_cb The function called when getting the values
 * @param[in] data_free_cb The function called on each value when the hash table is
 *                     freed, or when an item is deleted from it \n
 *                     @c NULL can be passed as a callback.
 * @param[in] buckets_power_size The size of the buckets
 * @return The new hash table
 *
 */
EAPI Eina_Hash *eina_hash_new(Eina_Key_Length key_length_cb,
                              Eina_Key_Cmp    key_cmp_cb,
                              Eina_Key_Hash   key_hash_cb,
                              Eina_Free_Cb    data_free_cb,
                              int             buckets_power_size) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(2, 3);

/**
 * @brief Redefines the callback that cleans the data of a hash.
 *
 * @since 1.1
 *
 * @since_tizen 2.3
 *
 * @remarks The argument received by @a data_free_cb is the data of the item being
 *          removed.
 *
 * @param[in] hash The given hash table
 * @param[in] data_free_cb The function called on each value when the hash
 *                     table is freed, or when an item is deleted from it \n
 *                     @c NULL can be passed as
 *                     callback to remove an existing callback.
 *
 * @see eina_hash_new
 */
EAPI void eina_hash_free_cb_set(Eina_Hash *hash, Eina_Free_Cb data_free_cb) EINA_ARG_NONNULL(1);

/**
 * @brief Creates a new hash table using the djb2 algorithm.
 *
 * @details This function creates a new hash table using the djb2 algorithm for
 *          table management and strcmp() to compare the keys. Values can then
 *          be looked up with pointers other than the original key pointer that
 *          is used to add values. On failure, this function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] data_free_cb The function called on each value when the hash table
 *                     is freed, or when an item is deleted from it \n
 *                     @c NULL can be passed as a callback.
 *
 * @return The new hash table
 */
EAPI Eina_Hash *eina_hash_string_djb2_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Creates a new hash table for use with strings.
 *
 * @details This function creates a new hash table using the superfast algorithm
 *          for table management and strcmp() to compare the keys. Values can
 *          then be looked up with pointers other than the original key pointer
 *          that is used to add values. On failure, this function returns
 *          @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] data_free_cb The function called on each value when the hash table
 *                     is freed, or when an item is deleted from it \n 
 *                     @c NULL can be passed as a callback.
 * @return The new hash table
 */
EAPI Eina_Hash *eina_hash_string_superfast_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Creates a new hash table for use with strings having a small bucket size.
 *
 * @details This function creates a new hash table using the superfast algorithm
 *          for table management and strcmp() to compare the keys, but with a
 *          smaller bucket size (compared to eina_hash_string_superfast_new()),
 *          which minimizes the memory used by the returned hash
 *          table. Values can then be looked up with pointers other than the
 *          original key pointer that is used to add values. On failure, this
 *          function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] data_free_cb  The function called on each value when the hash table
 *                      is freed, or when an item is deleted from it \n
 *                      @c NULL can be passed as a callback.
 * @return  The new hash table
 */
EAPI Eina_Hash *eina_hash_string_small_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Creates a new hash table for use with 32 bit integers.
 *
 * @details This function creates a new hash table where keys are 32 bit integers.
 *          When adding or looking up in the hash table, pointers to 32 bit integers
 *          must be passed. They can be addresses on the stack if you let
 *          eina_hash copy the key. Values can then
 *          be looked up with pointers other than the original key pointer that is
 *          used to add values. This method is not suitable to match string keys as
 *          it would only match the first character.
 *          On failure, this function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] data_free_cb  The function called on each value when the hash table
 *                      is freed, or when an item is deleted from it \n
 *                      @c NULL can be passed as a callback.
 * @return  The new hash table
 *
 */
EAPI Eina_Hash *eina_hash_int32_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Creates a new hash table for use with 64 bit integers.
 *
 * @details This function creates a new hash table where keys are 64 bit integers.
 *          When adding or looking up in the hash table, pointers to 64 bit integers
 *          must be passed. They can be addresses on the stack. Values can then
 *          be looked up with pointers other than the original key pointer that is
 *          used to add values. This method is not suitable to match string keys as
 *          it would only match the first character.
 *          On failure, this function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] data_free_cb  The function called on each value when the hash table
 *                      is freed, or when an item is deleted from it \n
 *                      @c NULL can be passed as a callback.
 * @return  The new hash table
 *
 */
EAPI Eina_Hash *eina_hash_int64_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Creates a new hash table for use with pointers.
 *
 * @details This function creates a new hash table using the int64/int32 algorithm for
 *          table management and dereferenced pointers to compare the
 *          keys. Values can then be looked up with pointers other than the
 *          original key pointer that is used to add values. This method may
 *          appear to be able to match string keys, actually it only matches
 *          the first character. On failure, this function returns @c NULL.
 *
 * @code
 * // For a hash that will have only one pointer to each structure
 * extern Eina_Hash *hash;
 * extern void *data;
 *
 * if (!eina_hash_find(hash, &data))
 *    eina_hash_add(hash, &data, data);
 * @endcode
 *
 * @since_tizen 2.3
 *
 * @param[in] data_free_cb  The function called on each value when the hash table
 *                      is freed, or when an item is deleted from it \n
 *                      @c NULL can be passed as a callback.
 * @return  The new hash table
 *
 */
EAPI Eina_Hash *eina_hash_pointer_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Creates a new hash table optimized for stringshared values.
 *
 * @details This function creates a new hash table optimized for stringshared
 *          values. Values CANNOT be looked up with pointers not
 *          equal to the original key pointer that is used to add a value. On failure,
 *          this function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * An Excerpt of a code that does NOT work with this type of hash:
 *
 * @code
 * extern Eina_Hash *hash;
 * extern const char *value;
 * const char *a = eina_stringshare_add("key");
 *
 * eina_hash_add(hash, a, value);
 * eina_hash_find(hash, "key")
 * @endcode
 *
 * @param[in] data_free_cb  The function called on each value when the hash table
 *                      is freed, or when an item is deleted from it \n
 *                      @c NULL can be passed as a callback.
 * @return  The new hash table
 *
 */
EAPI Eina_Hash *eina_hash_stringshared_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Adds an entry to the given hash table.
 *
 * @details This function adds @a key to @a hash. @a key is
 *          expected to be unique within the hash table. The key's uniqueness varies
 *          depending on the type of @a hash: a stringshared @ref Eina_Hash
 *          needs to have unique pointers (which implies unique strings).
 *          All other string hash types require the strings
 *          themselves to be unique. Pointer, int32 and int64 hashes need to have these
 *          values as unique. Failure to use sufficient uniqueness
 *          results in unexpected results when inserting data pointers accessed
 *          by eina_hash_find() and removed by eina_hash_del(). 
 *
 * @since_tizen 2.3
 *
 * @remarks	Key strings are case sensitive. If an error occurs, eina_error_get()
 *          should be used to determine if an allocation error occurred during
 *          this function. This function returns @c EINA_FALSE if an error
 *          occurs, otherwise it returns @c EINA_TRUE.
 *
 * @param[in] hash The given hash table \n
 *             It cannot be @c NULL.
 * @param[in] key A unique key \n
 *            It cannot be @c NULL.
 * @param[in] data The data to associate with the string given by @a key \n
 *             It cannot be @c NULL.
 * @return @c EINA_FALSE if an error occurs, otherwise @c EINA_TRUE
 *
 */
EAPI Eina_Bool  eina_hash_add(Eina_Hash  *hash,
                              const void *key,
                              const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Adds an entry to the given hash table without duplicating the string
 *        key.
 *
 * @details This function adds @a key to @a hash. @a key is
 *          expected to be unique within the hash table. The key's uniqueness varies
 *          depending on the type of @a hash: a stringshared @ref Eina_Hash
 *          needs to have unique pointers (which implies unique strings).
 *          All other string hash types require the strings
 *          themselves to be unique. Pointer, int32 and int64 hashes need to have these
 *          values as unique. Failure to use sufficient uniqueness
 *          results in unexpected results when inserting data pointers accessed
 *          by eina_hash_find() and removed by eina_hash_del(). 
 *
 * @since_tizen 2.3
 *
 * @remarks	This function does not make a copy of @a key, so it must be a string
 *          constant or should be stored elsewhere (in the object being added). Key
 *          strings are case sensitive. If an error occurs, eina_error_get()
 *          should be used to determine if an allocation error occurred during
 *          this function. This function returns @c EINA_FALSE if an error
 *          occurs, otherwise @c EINA_TRUE.
 *
 * @param[in] hash The given hash table \n
 *             It cannot be @c NULL.
 * @param[in] key A unique key \n
 *            It cannot be @c NULL.
 * @param[in] data The data to associate with the string given by @a key \n
 *             It cannot be @c NULL.
 * @return @c EINA_FALSE if an error occurs,  ptherwise @c EINA_TRUE
 *
 */
EAPI Eina_Bool eina_hash_direct_add(Eina_Hash  *hash,
                                    const void *key,
                                    const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Removes the entry identified by a key or data from the given
 *        hash table.
 *
 * @details This function removes the entry identified by @a key or @a data
 *          from @a hash. If a free function is given to the
 *          callback on creation, it is called for the data being
 *          deleted. If @a hash is @c NULL, the functions returns @c EINA_FALSE immediately.
 *          If @a key is @c NULL, then @a data is used to find the
 *          match to remove, otherwise @a key is used and @a data is not
 *          required and can be @c NULL. This function returns @c EINA_FALSE if
 *          an error occurs, otherwise it returns EINA_TRUE.
 *
 * @since_tizen 2.3
 *
 * @remarks If you already have the key, use
 *          eina_hash_del_by_key() or eina_hash_del_by_key_hash(). If you
 *          don't have the key, use eina_hash_del_by_data() directly.
 *
 * @param[in] hash The given hash table
 * @param[in] key  The key
 * @param[in] data The data pointer to remove if the key is @c NULL
 * @return @c EINA_FALSE if an error occurs, otherwise @c EINA_TRUE
 *
 */
EAPI Eina_Bool eina_hash_del(Eina_Hash  *hash,
                             const void *key,
                             const void *data) EINA_ARG_NONNULL(1);

/**
 * @brief Finds a specific entry in the given hash table.
 *
 * @details This function retrieves the entry associated to @a key in
 *          @a hash. If @a hash is @c NULL, this function returns
 *          @c NULL immediately. This function returns the data pointer on success,
 *          otherwise it returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] hash The given hash table
 * @param[in] key The key of the entry to find
 * @return The data pointer for the stored entry on success, otherwise @c NULL
 *
 */
EAPI void *eina_hash_find(const Eina_Hash *hash,
                          const void      *key) EINA_ARG_NONNULL(2);

/**
 * @brief Modifies the entry pointer at the specified key and returns the old
 *        entry.
 *
 * @details This function modifies the data of @a key with @a data in @a
 *          hash. If no entry is found, nothing is added to @a hash. On success
 *          this function returns the old entry, otherwise it returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] hash The given hash table
 * @param[in] key The key of the entry to modify
 * @param[in] data The data used to replace the old entry
 * @return The data pointer for the old stored entry on success,
 *         otherwise @c NULL
 */
EAPI void *eina_hash_modify(Eina_Hash  *hash,
                            const void *key,
                            const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Modifies the entry pointer at the specified key and returns the
 *        old entry or adds the entry if not found.
 *
 * @details This function modifies the data of @a key with @a data in @a
 *          hash. If no entry is found, @a data is added to @a hash with the
 *          key @a key. On success, this function returns the old entry,
 *          otherwise it returns @c NULL. To check for errors, use
 *          eina_error_get().
 *
 * @since_tizen 2.3
 *
 * @param[in] hash The given hash table
 * @param[in] key The key of the entry to modify
 * @param[in] data The data used to replace the old entry
 * @return The data pointer for the old stored entry on success,
 *         otherwise @c NULL
 */
EAPI void *eina_hash_set(Eina_Hash  *hash,
                         const void *key,
                         const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Changes the key associated with the data without triggering the
 *        free callback.
 *
 * @details This function allows for the move of data from one key to another,
 *          but does not call the Eina_Free_Cb associated with the hash table
 *          when destroying the old key.
 *
 * @since_tizen 2.3
 *
 * @param[in] hash    The given hash table
 * @param[in] old_key The current key associated with the data
 * @param[in] new_key The new key to associate the data with
 * @return @c EINA_FALSE in any case but success, otherwise @c EINA_TRUE on success
 *
 */
EAPI Eina_Bool eina_hash_move(Eina_Hash  *hash,
                              const void *old_key,
                              const void *new_key) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Frees the given hash table resources.
 *
 * @details This function frees up all the memory allocated to storing @a hash,
 *          and calls the free callback if it has been passed to the hash table
 *          at creation time. If no free callback has been passed, any entries
 *          in the table that the program has no more pointers for elsewhere
 *          may now be lost, so this should only be called if the program has
 *          already freed any allocated data in the hash table or has
 *          pointers for data in the table stored elsewhere as well. If @a hash
 *          is @c NULL, the function returns immediately.
 *
 * @since_tizen 2.3
 *
 * Example:
 * @code
 * extern Eina_Hash *hash;
 *
 * eina_hash_free(hash);
 * hash = NULL;
 * @endcode
 *
 * @param[in] hash The hash table to be freed
 *
 */
EAPI void      eina_hash_free(Eina_Hash *hash) EINA_ARG_NONNULL(1);

/**
 * @brief Frees the given hash table buckets resources.
 *
 * @details This function frees up all the memory allocated to storing the
 *          buckets of @a hash, and calls the free callback on all hash table
 *          buckets if it has been passed to the hash table at creation time,
 *          it then frees the buckets. If no free callback has been passed, no
 *          buckets value are freed. If @a hash is @c NULL, the function
 *          returns immediately.
 *
 * @since_tizen 2.3
 *
 * @param[in] hash The hash table whose buckets have to be freed
 *
 */
EAPI void      eina_hash_free_buckets(Eina_Hash *hash) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the number of entries in the given hash table.
 *
 * @details This function returns the number of entries in @a hash, or @c 0 on
 *          error. If @a hash is @c NULL, @c 0 is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] hash The given hash table
 * @return The number of entries in the hash table
 *
 */
EAPI int       eina_hash_population(const Eina_Hash *hash) EINA_ARG_NONNULL(1);

/**
 * @brief Adds an entry to the given hash table.
 *
 * @details This function adds @a key to @a hash. @a hash, @a key, and @a data
 *          cannot be @c NULL, in that case @c EINA_FALSE is returned. @a key is
 *          expected to be unique within the hash table. Otherwise,
 *          one cannot be sure which inserted data pointer is accessed
 *          by @ref eina_hash_find, and removed by @ref eina_hash_del. Do
 *          not forget to count '\\0' for string when setting the value of
 *          @a key_length. @a key_hash is expected to always match
 *          @a key. Otherwise, one cannot be sure to find it again with @ref
 *          eina_hash_find_by_hash. Key strings are case sensitive. If an error
 *          occurs, eina_error_get() should be used to determine if an
 *          allocation error occurred during this function. This function
 *          returns @c EINA_FALSE if an error occurs, otherwise it returns @c EINA_TRUE.
 *
 * @since_tizen 2.3
 *
 * @param[in] hash The given hash table \n
 *             It cannot be @c NULL.
 * @param[in] key A unique key \n
 *            It cannot be @c NULL.
 * @param[in] key_length The length of the key
 * @param[in] key_hash The hash that always matches the key
 * @param[in] data The data to associate with the string given by the key \n
 *             It cannot be @c NULL.
 * @return @c EINA_FALSE if an error occurs, otherwise @c EINA_TRUE
 *
 * @see eina_hash_add()
 */
EAPI Eina_Bool eina_hash_add_by_hash(Eina_Hash  *hash,
                                     const void *key,
                                     int         key_length,
                                     int         key_hash,
                                     const void *data) EINA_ARG_NONNULL(1, 2, 5);

/**
 * @brief Adds an entry to the given hash table and does not duplicate the string
 *        key.
 *
 * @details This function adds @a key to @a hash. @a hash, @a key, and @a data
 *          can be @c NULL, in that case @c EINA_FALSE is returned. @a key is
 *          expected to be unique within the hash table. Otherwise,
 *          one cannot be sure which inserted data pointer is going to be accessed
 *          by @ref eina_hash_find, and removed by @ref eina_hash_del. This
 *          function does not make a copy of @a key so it must be a string
 *          constant or should be stored elsewhere (in the object being added). Do
 *          not forget to count '\\0' for string when setting the value of
 *          @a key_length. @a key_hash is expected to always match
 *          @a key. Otherwise, one cannot be sure to find it again with @ref
 *          eina_hash_find_by_hash. Key strings are case sensitive. If an error
 *          occurs, eina_error_get() should be used to determine if an
 *          allocation error occurred during this function. This function
 *          returns @c EINA_FALSE if an error occurs, otherwise it returns @c EINA_TRUE.
 *
 * @since_tizen 2.3
 *
 * @param[in] hash The given hash table \n
 *             It cannot be @c NULL.
 * @param[in] key A unique key \n
 *            It cannot be @c NULL.
 * @param[in] key_length The length of @a key (don't forget to count
 *                   '\\0' for string).
 * @param[in] key_hash The hash that always matches the key.
 * @param[in] data The data to associate with the string given by @a key \n
 *             It cannot be @c NULL.
 * @return @c EINA_FALSE if an error occurs, otherwise @c EINA_TRUE
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
 *        hash table.
 *
 * @details This function removes the entry identified by @a key and
 *          @a key_hash from @a hash. If a free function is given to the
 *          callback on creation, it is called for the data being
 *          deleted. Do not forget to count '\\0' for string when setting the
 *          value of @a key_length. If @a hash or @a key is @c NULL, the
 *          functions return @c EINA_FALSE immediately. This function
 *          returns @c EINA_FALSE if an error occurs, otherwise it returns @c EINA_TRUE.
 *
 * @since_tizen 2.3
 *
 * @remarks  If you don't have the key_hash, use eina_hash_del_by_key() instead.
 * @remarks  If you don't have the key, use eina_hash_del_by_data() instead.
 *
 * @param[in] hash The given hash table \n
 *             It cannot be @c NULL.
 * @param[in] key The key \n
 *            It cannot be @c NULL.
 * @param[in] key_length The length of the key
 * @param[in] key_hash The hash that always matches the key
 * @return @c EINA_FALSE if an error occurs, otherwise @c EINA_TRUE
 */
EAPI Eina_Bool eina_hash_del_by_key_hash(Eina_Hash  *hash,
                                         const void *key,
                                         int         key_length,
                                         int         key_hash) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Removes the entry identified by a key from the given hash table.
 *
 * @details This function removes the entry identified by @a key from @a
 *          hash. The key length and hash is calculated automatically by
 *          using functions provided to the hash creation function. If a free
 *          function is given to the callback on creation, it is called
 *          for the data being deleted. If @a hash or @a key is @c NULL, the
 *          functions return @c EINA_FALSE immediately. This function
 *          returns @c EINA_FALSE if an error occurs, otherwise it returns @c EINA_TRUE.
 *
 * @since_tizen 2.3
 *
 * @remarks If you already have the key_hash, use eina_hash_del_by_key_hash()
 *          instead.
 * @remarks If you don't have the key, use eina_hash_del_by_data() instead.
 *
 * @remarks This version calculates the key length and hash by using functions
 *          provided to the hash creation function.
 *
 * @param[in] hash The given hash table \n
 *             It cannot be @c NULL.
 * @param[in] key  The key \n
 *             It cannot be @c NULL.
 * @return @c EINA_FALSE if an error occurs, otherwise @c EINA_TRUE
 *
 */
EAPI Eina_Bool eina_hash_del_by_key(Eina_Hash  *hash,
                                    const void *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Removes the entry identified by data from the given hash table.
 *
 * @details This function removes the entry identified by @a data from @a
 *          hash. If a free function is given to the callback on creation, it
 *          is called for the data being deleted. If @a hash or @a data
 *          is @c NULL, the functions return @c EINA_FALSE immediately. This
 *          function returns @c EINA_FALSE if an error occurs, otherwise it returns @c EINA_TRUE.
 *
 * @since_tizen 2.3
 *
 * @remarks If you already have the key, use eina_hash_del_by_key() or
 *          eina_hash_del_by_key_hash() instead.
 *
 * @remarks This version is slow since there is no quick access to nodes based on the data.
 *
 * @param[in] hash The given hash table \n
 *             It cannot be @c NULL.
 * @param[in] data The data value to search and remove \n
 *             It cannot be @c NULL.
 * @return @c EINA_FALSE if an error occurs, otherwise @c EINA_TRUE on success
 */
EAPI Eina_Bool eina_hash_del_by_data(Eina_Hash  *hash,
                                     const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Removes the entry identified by a key and a key hash or the
 *        data from the given hash table.
 *
 * @details This function removes the entry identified by @a key and
 *          @a key_hash, or @a data, from @a hash. If a free function is given to
 *          the callback on creation, it is called for the data being
 *          deleted. If @a hash is @c NULL, the functions return @c EINA_FALSE immediately.
 *          If @a key is @c NULL, then @a key_length and @a key_hash
 *          are ignored and @a data is used to find a match to remove,
 *          otherwise @a key and @a key_hash are used and @a data is not
 *          required and can be @c NULL. Do not forget to count '\\0' for
 *          string when setting the value of @a key_length. This function
 *          returns @c EINA_FALSE if an error occurs, otherwise it returns @c EINA_TRUE.
 *
 * @since_tizen 2.3
 *
 * @remarks If you already have the key, use eina_hash_del_by_key_hash().
 *          If you don't have the key, use eina_hash_del_by_data()
 *          directly.
 *
 * @remarks If @a key is @c NULL, then @a data is used to find a match to
 *          remove.
 *
 * @param[in] hash The given hash table \n
 *             It cannot be @c NULL.
 * @param[in] key The key
 * @param[in] key_length The length of the key
 * @param[in] key_hash The hash that always matches the key
 * @param[in] data The data pointer to remove if the key is @c NULL
 * @return @c EINA_FALSE if an error occurs, otherwise @c EINA_TRUE
 *
 */
EAPI Eina_Bool eina_hash_del_by_hash(Eina_Hash  *hash,
                                     const void *key,
                                     int         key_length,
                                     int         key_hash,
                                     const void *data) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieves a specific entry in the given hash table.
 *
 * @details This function retrieves the entry associated to @a key of length
 *          @a key_length in @a hash. @a key_hash is the hash that always matches
 *          @a key. It is ignored if @a key is @c NULL. Do not forget to count
 *          '\\0' for string when setting the value of @a key_length. If
 *          @a hash is @c NULL, this function returns @c NULL immediately. This
 *          function returns the data pointer on success, otherwise it returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] hash The given hash table \n
 *             It cannot be @c NULL.
 * @param[in] key The key of the entry to find
 * @param[in] key_length The length of the key
 * @param[in] key_hash The hash that always matches the key
 * @return The data pointer for the stored entry on success,
 *         otherwise @c NULL
 */
EAPI void *eina_hash_find_by_hash(const Eina_Hash *hash,
                                  const void      *key,
                                  int              key_length,
                                  int              key_hash) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Modifies the entry pointer at the specified key and returns
 *        the old entry.
 *
 * @since_tizen 2.3
 *
 * @param[in] hash The given hash table
 * @param[in] key The key of the entry to modify
 * @param[in] key_length The length of @a key (don't forget to count
 *                   '\\0' for string)
 * @param[in] key_hash The hash that always matches the key \n
 *                 It is ignored if @a key is @c NULL.
 * @param[in] data The data to replace the old entry, if it exists
 * @return The data pointer for the old stored entry, otherwise @c NULL if not
 *         found \n
 *         If an existing entry is not found, nothing is added to the
 *         hash.
 */
EAPI void *eina_hash_modify_by_hash(Eina_Hash  *hash,
                                    const void *key,
                                    int         key_length,
                                    int         key_hash,
                                    const void *data) EINA_ARG_NONNULL(1, 2, 5);

/**
 * @brief Returns a new iterator associated to hash keys.
 *
 * @details This function returns a newly allocated iterator associated to @a
 *          hash. If @a hash is not populated, this function still returns a
 *          valid iterator that always returns @c false on a call to
 *          eina_iterator_next(), thus keeping the API sane.
 *
 * @since_tizen 2.3
 *
 * @remarks If the memory cannot be allocated, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator is
 *          returned.
 *
 * @remarks If the hash structure changes, the iterator becomes
 *          invalid. That is, if you add or remove items this iterator's
 *          behavior is undefined and your program may crash.
 *
 * @param[in] hash The hash
 * @return A new iterator
 *
 */
EAPI Eina_Iterator *eina_hash_iterator_key_new(const Eina_Hash *hash) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a new iterator associated to hash data.
 *
 * @details This function returns a newly allocated iterator associated to
 *          @a hash. If @a hash is not populated, this function still returns a
 *          valid iterator that always returns @c false on a call to
 *          eina_iterator_next(), thus keeping the API sane.
 *
 * @since_tizen 2.3
 *
 * @remarks If the memory cannot be allocated, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator is
 *          returned.
 *
 * @remarks If the hash structure changes then the iterator becomes
 *          invalid. That is, if you add or remove items this iterator's behavior
 *          is undefined and your program may crash.
 *
 * @param[in] hash The hash
 * @return A new iterator
 *
 */
EAPI Eina_Iterator *eina_hash_iterator_data_new(const Eina_Hash *hash) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a new iterator associated to hash keys and data.
 *
 * @details This function returns a newly allocated iterator associated to @a
 *          hash. If @a hash is not populated, this function still returns a
 *          valid iterator that always returns @c false on a call to
 *          eina_iterator_next(), thus keeping the API sane.
 *
 * @since_tizen 2.3
 *
 * @remarks If the memory cannot be allocated, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator is
 *          returned.
 *
 * @remarks The iterator data provides values as Eina_Hash_Tuple that should not
 *          be modified.
 *
 * @remarks If the hash structure changes then the iterator becomes
 *          invalid. That is, if you add or remove items this iterator's
 *          behavior is undefined and your program may crash.
 *
 * @param[in] hash The hash
 * @return A new iterator
 *
 */
EAPI Eina_Iterator *eina_hash_iterator_tuple_new(const Eina_Hash *hash) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Calls a function on every member stored in the hash table.
 *
 * @details This function goes through every entry in the hash table @a hash and calls
 *          the function @a func on each member. The function should @b not modify the
 *          hash table contents if it returns @c 1. @b If the hash table contents are
 *          modified by this function or the function wishes to stop processing it, it must
 *          return @c 0, otherwise it must return @c 1 to keep processing.
 *
 * @since_tizen 2.3
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
 *
 * @param[in] hash The hash table whose members are walked
 * @param[in] func The function to call on each parameter
 * @param[in] fdata The data pointer to pass to the function being called
 */
EAPI void           eina_hash_foreach(const Eina_Hash  *hash,
                                      Eina_Hash_Foreach func,
                                      const void       *fdata) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Paul Hsieh (http://www.azillionmonkeys.com/qed/hash.html) hash 
 *        function used by WebCore (http://webkit.org/blog/8/hashtables-part-2/) 
 *
 * @since_tizen 2.3
 *
 * @param[in] key The key to hash
 * @param[in] len The length of the key
 * @return The hash value
 */
EAPI int eina_hash_superfast(const char *key,
                             int         len) EINA_ARG_NONNULL(1);

/**
 * @details The djb2 hash function
 *
 * @since_tizen 2.3
 *
 * @remarks  djb2 hash algorithm was first reported by dan bernstein, and was
 *           the old default hash function for evas.
 *
 * @remarks  This hash function first reported by dan bernstein many years ago
 *           in comp.lang.c
 *
 * @param[in] key  The key to hash
 * @param[in] len  The length of the key
 * @return The hash value
 */
static inline int eina_hash_djb2(const char *key,
                                 int         len) EINA_ARG_NONNULL(1);

/**
 * @details The djb2 hash function withoug length
 *
 * @since_tizen 2.3
 *
 * @remarks  djb2 hash algorithm was first reported by dan bernstein, and was
 *           the old default hash function for evas.
 *
 * @remarks  This hash function first reported by dan bernstein many years ago
 *           in comp.lang.c
 *
 * @param[in] key  The key to hash
 * @param[out] plen  The length of the key to be returned
 * @return The hash value
 */
static inline int eina_hash_djb2_len(const char *key,
                                     int        *plen) EINA_ARG_NONNULL(1, 2);

/**
 * @details The 32 bit integer hash function
 *
 * @since_tizen 2.3
 *
 * @remarks Hash function from
 *          http://www.concentric.net/~Ttwang/tech/inthash.htm
 *
 * @param[in] pkey  The key to hash
 * @param[in] len  The length of the key
 * @return The hash value
 */
static inline int eina_hash_int32(const unsigned int *pkey,
                                  int                 len) EINA_ARG_NONNULL(1);

/**
 * @details The 64 bit integer hash function
 *
 * @since_tizen 2.3
 *
 * @param[in] pkey  The key to hash
 * @param[in] len  The length of the key
 * @return The hash value
 */
static inline int eina_hash_int64(const unsigned long int *pkey,
                                  int                      len) EINA_ARG_NONNULL(1);

/**
 * @details The murmur3 hash function
 *
 * @since_tizen 2.3
 *
 * @remarks http://sites.google.com/site/murmurhash/
 *
 * @param[in] key  The key to hash
 * @param[in] len  The length of the key
 * @return The hash value
 */
static inline int eina_hash_murmur3(const char *key,
                           int         len) EINA_ARG_NONNULL(1);
#include "eina_inline_hash.x"

/**
 * @}
 */

#endif /*EINA_HASH_H_*/
