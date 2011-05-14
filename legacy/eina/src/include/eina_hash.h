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
 * @addtogroup Eina_Hash_Group Hash Table
 *
 * @brief give a small description here : what it is for, what it does
 * , etc...
 * Different implementations exists depending on what to store: strings,
 * integers, pointers, stringshared or your own...
 * Eina hash tables can copy the keys when using eina_hash_add() or not when
 * using eina_hash_direct_add().
 *
 * Hash API. Give some hints about the use (functions that must be
 * used like init / shutdown), general use, etc... Give also a link to
 * tutorial below.
 *
 * @section hashtable_algo Algorithm
 *
 * Give here the algorithm used in the implementation
 *
 * @section hashtable_perf Performance
 *
 * Give some hints about performance if it is possible, and an image !
 *
 * @section hashtable_tutorial Tutorial
 *
 * Here is a fantastic tutorial about our hash table
 *
 * @{
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
#define EINA_KEY_LENGTH(Function) ((Eina_Key_Length)Function)
typedef int          (*Eina_Key_Cmp)(const void *key1, int key1_length, const void *key2, int key2_length);
#define EINA_KEY_CMP(Function)    ((Eina_Key_Cmp)Function)
typedef int          (*Eina_Key_Hash)(const void *key, int key_length);
#define EINA_KEY_HASH(Function)   ((Eina_Key_Hash)Function)
typedef Eina_Bool    (*Eina_Hash_Foreach)(const Eina_Hash *hash, const void *key, void *data, void *fdata);


/**
 * @brief Create a new hash table.
 *
 * @param key_length_cb The function called when getting the size of the key.
 * @param key_cmp_cb The function called when comparing the keys.
 * @param key_hash_cb The function called when getting the values.
 * @param data_free_cb The function called on each value when the hash
 * table is freed. @c NULL can be passed as callback.
 * @param buckets_power_size The size of the buckets.
 * @return The new hash table.
 *
 * This function creates a new hash table using user-defined callbacks
 * to manage the hash table. On failure, @c NULL is returned and
 * #EINA_ERROR_OUT_OF_MEMORY is set. If @p key_cmp_cb or @p key_hash_cb
 * are @c NULL, @c NULL is returned. If @p buckets_power_size is
 * smaller or equal than 2, or if it is greater or equal than 17,
 * @c NULL is returned.
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
 * @brief Create a new hash table using the djb2 algorithm.
 *
 * @param data_free_cb The function called on each value when the hash table
 * is freed. @c NULL can be passed as callback.
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
 * is freed. @c NULL can be passed as callback.
 * @return The new hash table.
 *
 * This function creates a new hash table using the superfast algorithm
 * for table management and strcmp() to compare the keys. Values can
 * then be looked up with pointers other than the original key pointer
 * that was used to add values. On failure, this function returns
 * @c NULL.
 */
EAPI Eina_Hash *eina_hash_string_superfast_new(Eina_Free_Cb data_free_cb);

/**
 * @brief Create a new hash table for use with strings with small bucket size.
 *
 * @param data_free_cb  The function called on each value when the hash table
 * is freed. @c NULL can be passed as callback.
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
 * is freed. @c NULL can be passed as callback.
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
 * is freed. @c NULL can be passed as callback.
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
 * is freed. @c NULL can be passed as callback.
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
 * is freed. @c NULL can be passed as callback.
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
 * @param hash The given hash table.
 * @param key A unique key.
 * @param data Data to associate with the string given by @p key.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function adds @p key to @p hash. @p hash, @p key and @p data
 * can be @c NULL, in that case #EINA_FALSE is returned. @p key is
 * expected to be unique within the hash table. Key uniqueness varies
 * depending on the type of @p hash: a stringshared @ref Eina_Hash
 * need only have unique pointers for keys, but the strings in the
 * pointers may be identical. All other hash types require the strings
 * themselves to be unique. Failure to use sufficient uniqueness will
 * result in unexpected results when inserting data pointers accessed
 * with eina_hash_find(), and removed with eina_hash_del(). Key
 * strings are case sensitive. If an error occurs, eina_error_get()
 * should be used to determine if an allocation error occurred during
 * this function. This function returns #EINA_FALSE if an error
 * occurred, #EINA_TRUE otherwise.
 */
EAPI Eina_Bool  eina_hash_add(Eina_Hash  *hash,
                              const void *key,
                              const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Add an entry to the given hash table without duplicating the string
 * key.
 *
 * @param hash The given hash table.  Can be @c NULL.
 * @param key A unique key.  Can be @c NULL.
 * @param data Data to associate with the string given by @p key.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function adds @p key to @p hash. @p hash, @p key and @p data
 * can be @c NULL, in that case #EINA_FALSE is returned. @p key is
 * expected to be unique within the hash table. Key uniqueness varies
 * depending on the type of @p hash: a stringshared @ref Eina_Hash
 * need only have unique pointers for keys, but the strings in the
 * pointers may be identical. All other hash types require the strings
 * themselves to be unique. Failure to use sufficient uniqueness will
 * result in unexpected results when inserting data pointers accessed
 * with eina_hash_find(), and removed with eina_hash_del(). This
 * function does not make a copy of @p key, so it must be a string
 * constant or stored elsewhere ( in the object being added). Key
 * strings are case sensitive. If an error occurs, eina_error_get()
 * should be used to determine if an allocation error occurred during
 * this function. This function returns #EINA_FALSE if an error
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
 * deleted. If @p hash is @c NULL, the functions returns immediately
 * #EINA_FALSE. If @p key is @c NULL, then @p data is used to find the a
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
                          const void      *key) EINA_ARG_NONNULL(1, 2);

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
 * otherwise it returns @c NULL. To check for errors, use
 * eina_error_get().
 */
EAPI void *eina_hash_set(Eina_Hash  *hash,
                         const void *key,
                         const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Change the key associated with a data without triggering the
 * free callback.
 *
 * @param hash    The given hash table.
 * @param old_key The current key associated with the data
 * @param new_key The new key to associate data with
 * @return EINA_FALSE in any case but success, EINA_TRUE on success.
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
 * error. If @p hash is @c NULL, 0 is returned.
 */
EAPI int       eina_hash_population(const Eina_Hash *hash) EINA_ARG_NONNULL(1);

/**
 * @brief Add an entry to the given hash table.
 *
 * @param hash The given hash table.
 * @param key A unique key.
 * @param key_length The length of the key.
 * @param key_hash The hash that will always match key.
 * @param data The data to associate with the string given by the key.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function adds @p key to @p hash. @p hash, @p key and @p data
 * can be @c NULL, in that case #EINA_FALSE is returned. @p key is
 * expected to be a unique string within the hash table. Otherwise,
 * one cannot be sure which inserted data pointer will be accessed
 * with @ref eina_hash_find, and removed with @ref eina_hash_del. Do
 * not forget to count '\\0' for string when setting the value of
 * @p key_length. @p key_hash is expected to always match
 * @p key. Otherwise, one cannot be sure to find it again with @ref
 * eina_hash_find_by_hash. Key strings are case sensitive. If an error
 * occurs, eina_error_get() should be used to determine if an
 * allocation error occurred during this function. This function
 * returns #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
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
 * @param hash The given hash table.  Can be @c NULL.
 * @param key A unique key.  Can be @c NULL.
 * @param key_length Should be the length of @p key (don't forget to count
 * '\\0' for string).
 * @param key_hash The hash that will always match key.
 * @param data Data to associate with the string given by @p key.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function adds @p key to @p hash. @p hash, @p key and @p data
 * can be @c NULL, in that case #EINA_FALSE is returned. @p key is
 * expected to be a unique string within the hash table. Otherwise,
 * one cannot be sure which inserted data pointer will be accessed
 * with @ref eina_hash_find, and removed with @ref eina_hash_del. This
 * function does not make a copy of @p key so it must be a string
 * constant or stored elsewhere (in the object being added). Do
 * not forget to count '\\0' for string when setting the value of
 * @p key_length. @p key_hash is expected to always match
 * @p key. Otherwise, one cannot be sure to find it again with @ref
 * eina_hash_find_by_hash. Key strings are case sensitive. If an error
 * occurs, eina_error_get() should be used to determine if an
 * allocation error occurred during this function. This function
 * returns #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
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
 * @param hash The given hash table.
 * @param key The key.
 * @param key_length The length of the key.
 * @param key_hash The hash that always match the key.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function removes the entry identified by @p key and
 * @p key_hash from @p hash. If a free function was given to the
 * callback on creation, it will be called for the data being
 * deleted. Do not forget to count '\\0' for string when setting the
 * value of @p key_length. If @p hash or @p key are @c NULL, the
 * functions returns immediately #EINA_FALSE. This function returns
 * #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
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
 * @param hash The given hash table.
 * @param key  The key.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function removes the entry identified by @p key from @p
 * hash. The key length and hash will be calculated automatically by
 * using functiond provided to has creation function. If a free
 * function was given to the callback on creation, it will be called
 * for the data being deleted. If @p hash or @p key are @c NULL, the
 * functions returns immediately #EINA_FALSE. This function returns
 * #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
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
 * @param hash The given hash table.
 * @param data The data value to search and remove.
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
 * @param hash The given hash table.
 * @param key The key.
 * @param key_length The length of the key.
 * @param key_hash The hash that always match the key.
 * @param data The data pointer to remove if the key is @c NULL.
 * @return #EINA_FALSE if an error occurred, #EINA_TRUE otherwise.
 *
 * This function removes the entry identified by @p key and
 * @p key_hash, or @p data, from @p hash. If a free function was given to
 * the  callback on creation, it will be called for the data being
 * deleted. If @p hash is @c NULL, the functions returns immediately
 * #EINA_FALSE. If @p key is @c NULL, then @p key_hash and @p key_hash
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
 * @param hash The given hash table.
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
 * If the memory can not be allocated, NULL is returned and
 * #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator is
 * returned.
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
 * If the memory can not be allocated, @c NULL is returned and
 * #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator is
 * returned.
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
 * If the memory can not be allocated, NULL is returned and
 * #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator is
 * returned.
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
 * hash table contents if it returns 1. @b If the hash table contents are
 * modified by this function or the function wishes to stop processing it must
 * return 0, otherwise return 1 to keep processing.
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
                                      Eina_Hash_Foreach cb,
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
static inline int eina_hash_int64(const unsigned long int *pkey,
                                  int                      len) EINA_ARG_NONNULL(1);

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

/**
 * @}
 */

#endif /*EINA_HASH_H_*/
