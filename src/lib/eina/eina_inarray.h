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

#ifndef EINA_INARRAY_H_
#define EINA_INARRAY_H_

#include "eina_types.h"
#include "eina_iterator.h"
#include "eina_accessor.h"

/**
 * @defgroup Eina_Inline_Array_Group Inline Array
 * @ingroup Eina_Containers_Group
 * @since 1.2
 *
 * @brief Inline array is a container that stores the data itself, not the pointers to the data,
 *
 * this means there is no memory fragmentation, also for small data types(such
 * as char, short, int, and so on) it's more memory efficient.
 *
 * Usage of the inline array is very similar to that of other
 * @ref Eina_Containers_Group, like all arrays adding elements to the beginning
 * of the array is a lot more costly than appending, so those operations should
 * be minimized.
 *
 * @{
 */


/**
 * @typedef Eina_Inarray
 * @brief The structure type for the inlined array type.
 *
 * @since 1.2
 */
typedef struct _Eina_Inarray Eina_Inarray;

/**
 * @brief Inline array structure, use #Eina_Inarray typedef instead.
 *
 * @details Do not modify these fields directly, use eina_inarray_step_set() or
 *          eina_inarray_new() instead.
 *
 * @since 1.2
 */
struct _Eina_Inarray
{
#define EINA_ARRAY_VERSION 1
   int          version; /**< Should match the EINA_ARRAY_VERSION used when compiling your apps, provided for ABI compatibility */

   unsigned int member_size; /**< Byte size of each entry in the members */
   unsigned int len; /**< Number of elements used by the members */
   unsigned int max; /**< Number of elements allocated to the members */
   unsigned int step; /**< Amount to grow the number of members allocated */
   void *members; /**< Actual array of elements */
   EINA_MAGIC
};

/**
 * @brief Creates a new inline array.
 *
 * @details This creates a new array where members are inlined in a sequence. Each
 *          member has @a member_size bytes.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks If the @a step is @c 0, then a safe default is chosen.
 *
 * @remarks On failure, @c NULL is returned and #EINA_ERROR_OUT_OF_MEMORY is
 *          set. If @a member_size is zero, then @c NULL is returned.
 *
 * @param[in] member_size The size of each member in the array
 * @param[in] step The step size by which to resize the array, do this using the following
 *             extra amount
 * @return The new inline array table, otherwise @c NULL on failure
 *
 * @see eina_inarray_free()
 */
EAPI Eina_Inarray *eina_inarray_new(unsigned int member_size,
                                    unsigned int step) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Frees an array and its members.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array object
 *
 * @see eina_inarray_flush()
 *
 */
EAPI void eina_inarray_free(Eina_Inarray *array) EINA_ARG_NONNULL(1);

/**
 * @brief Initializes an inline array.
 *
 * @details This initializes an array. If the @a step is @c 0, then a safe default is
 *          chosen.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks This is useful for arrays inlined into other structures or
 *          allocated to a stack.
 *
 * @param[in] array The array object to initialize
 * @param[in] sizeof_eina_inarray The size of array object
 * @param[in] member_size The size of each member in the array
 * @param[in] step The step size by which to resize the array, do this using the following
 *             extra amount
 *
 * @see eina_inarray_flush()
 */
EAPI void eina_inarray_step_set(Eina_Inarray *array,
                                unsigned int sizeof_eina_inarray,
                                unsigned int member_size,
                                unsigned int step) EINA_ARG_NONNULL(1);

/**
 * @brief Removes every member from the array.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array object
 *
 */
EAPI void eina_inarray_flush(Eina_Inarray *array) EINA_ARG_NONNULL(1);

/**
 * @brief Copies the data as the last member of the array.
 *
 * @details This copies the given pointer contents at the end of the array. The
 *          pointer is not referenced, instead its contents are copied to the
 *          members array using the previously defined @c member_size.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array object
 * @param[in] data The data to be copied at the end
 * @return The index of the new member, otherwise @c -1 on errors
 *
 * @see eina_inarray_insert_at()
 *
 */
EAPI int eina_inarray_push(Eina_Inarray *array,
                           const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Copies the data to the array at a position found by the comparison function.
 *
 * @details This copies the given pointer contents at the array position defined by the
 *          given @a compare function. The pointer is not referenced, instead
 *          its contents are copied to the members array using the previously
 *          defined @c member_size.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The data given to the @a compare function is a pointer to the member
 *          memory itself, do no change it.
 *
 * @param[in] array The array object
 * @param[in] data The data to be copied
 * @param[in] compare The compare function
 * @return The index of the new member, otherwise @c -1 on errors
 *
 * @see eina_inarray_insert_sorted()
 * @see eina_inarray_insert_at()
 * @see eina_inarray_push()
 */
EAPI int eina_inarray_insert(Eina_Inarray *array,
                             const void *data,
                             Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Copies the data to the array at a position found by the comparison function.
 *
 * @details This copies the given pointer contents at the array position defined by the
 *          given @a compare function. The pointer is not referenced, instead
 *          its contents are copied to the members array using the previously
 *          defined @c member_size.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The data given to the @a compare function is a pointer to the member
 *          memory itself, do no change it.
 *
 * @remarks This variation optimizes the insertion position assuming that the array
 *          is already sorted by doing a binary search.
 *
 * @param[in] array The array object
 * @param[in] data The data to be copied
 * @param[in] compare The compare function
 * @return The index of the new member, otherwise @c -1 on errors
 *
 * @see eina_inarray_sort()
 */
EAPI int eina_inarray_insert_sorted(Eina_Inarray *array,
                                    const void *data,
                                    Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Finds data and removes the matching member.
 *
 * @details This finds data in the array and removes it. Data may be an existing
 *          member of the array (then optimized) or the contents are matched
 *          using memcmp().
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array object
 * @param[in] data The data to be found and removed
 * @return The index of the removed member, otherwise @c -1 on errors
 *
 * @see eina_inarray_pop()
 * @see eina_inarray_remove_at()
 */
EAPI int eina_inarray_remove(Eina_Inarray *array,
                             const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Removes the last member of the array.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The data could be considered valid only until any other operation touched the Inarray.
 *
 * @param[in] array The array object
 * @return The data poped out of the array
 *
 */
EAPI void *eina_inarray_pop(Eina_Inarray *array) EINA_ARG_NONNULL(1);

/**
 * @brief Gets the member at the given position.
 *
 * @details This gets the member given that its position in the array is provided. It is a pointer to
 *          its current memory, then it can be invalidated with functions that
 *          change the array such as eina_inarray_push(),
 *          eina_inarray_insert_at(), or eina_inarray_remove_at(), or variants.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array object
 * @param[in] position The member position
 * @return A pointer to current the member memory
 *
 * @see eina_inarray_lookup()
 * @see eina_inarray_lookup_sorted()
 */
EAPI void *eina_inarray_nth(const Eina_Inarray *array,
                            unsigned int position) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Copies the data at the given position in the array.
 *
 * @details This copies the given pointer contents at the given @a position in the
 *          array. The pointer is not referenced, instead its contents are
 *          copied to the members array using the previously defined
 *          @c member_size.
 *
 *          All the members from @a position to the end of the array are
 *          shifted to the end.
 *
 *          If @a position is equal to the end of the array (equal to
 *          eina_inarray_count()), then the member is appended.
 *
 *          If @a position is bigger than the array length, it fails.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array object
 * @param[in] position The position to insert the member at
 * @param[in] data The data to be copied at the position
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_inarray_insert_at(Eina_Inarray *array,
                                      unsigned int position,
                                      const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Opens a space at the given position, returning its pointer.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks This is similar to eina_inarray_insert_at(), but useful if the
 *          members contents are still unknown or unallocated. It makes
 *          room for the required number of items and returns the pointer to the
 *          first item, similar to malloc(member_count * member_size), with the
 *          guarantee that all the memory is within the members array.
 *
 *          The new member memory is undefined, it's not automatically zeroed.
 *
 * @remarks All the members from @a position to the end of the array are
 *          shifted to the end.
 *
 *          If @a position is equal to the end of the array (equal to
 *          eina_inarray_count()), then the member is appended.
 *
 *          If @a position is bigger than the array length, it fails.
 *
 * @param[in] array The array object
 * @param[in] position The position to insert first member at (open/allocate space)
 * @param[in] member_count The number of times @c member_size bytes are allocated
 * @return A pointer to the first member memory allocated, otherwise @c NULL on errors
 *
 */
EAPI void *eina_inarray_alloc_at(Eina_Inarray *array,
                                 unsigned int position,
                                 unsigned int member_count) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Copies the data to the given position.
 *
 * @details This copies the given pointer contents at the given @a position in the
 *          array. The pointer is not referenced, instead its contents are
 *          copied to the members array using the previously defined
 *          @c member_size.
 *
 *          If @a position does not exist, it fails.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array object
 * @param[in] position The position to copy the member at
 * @param[in] data The data to be copied at the position
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool eina_inarray_replace_at(Eina_Inarray *array,
                                       unsigned int position,
                                       const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Removes a member from the given position.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The member is removed from an array and members after it are moved
 *          towards the array head.
 *
 * @param[in] array The array object
 * @param[in] position The position from which to remove a member
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE on failure
 *
 * @see eina_inarray_pop()
 * @see eina_inarray_remove()
 *
 */
EAPI Eina_Bool eina_inarray_remove_at(Eina_Inarray *array,
                                      unsigned int position) EINA_ARG_NONNULL(1);

/**
 * @brief Reverses members in the array.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks If you do not want to change the array, just walk through its elements
 *          backwards, then use the EINA_INARRAY_REVERSE_FOREACH() macro.
 *
 * @param[in] array The array object
 *
 * @see EINA_INARRAY_REVERSE_FOREACH()
 */
EAPI void eina_inarray_reverse(Eina_Inarray *array) EINA_ARG_NONNULL(1);

/**
 * @brief Applies a quick sort to the array.
 *
 * @details This applies a quick sort to the @a array.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks The data given to the @a compare function is a pointer to the member
 *          memory itself, do no change it.
 *
 * @param[in] array The array object
 * @param[in] compare The compare function
 *
 * @see eina_inarray_insert_sorted()
 */
EAPI void eina_inarray_sort(Eina_Inarray *array,
                            Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Searches for a member (linear walk).
 *
 * @details This walks through an array by linearly looking for the given data compared by
 *          the @a compare function.
 *
 *          The data given to the @a compare function is a pointer to the member
 *          memory itself, do no change it.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array object
 * @param[in] data The member to search using the @a compare function
 * @param[in] compare The compare function
 * @return The member index, otherwise @c -1 if not found
 *
 * @see eina_inarray_lookup_sorted()
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 */
EAPI int eina_inarray_search(const Eina_Inarray *array,
                             const void *data,
                             Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Searches for member (binary search walk).
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks Uses a binary search for the given data as compared by the @a compare function.
 *
 *          The data given to the @a compare function is a pointer to the member
 *          memory itself, do no change it.
 *
 * @param[in] array The array object
 * @param[in] data The member to search using the @a compare function
 * @param[in] compare The compare function
 * @return The member index, otherwise @c -1 if not found
 *
 */
EAPI int eina_inarray_search_sorted(const Eina_Inarray *array,
                                    const void *data,
                                    Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Calls @a function for each array member.
 *
 * @details This calls @a function for every given data in @a array.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks This is a safe way to iterate over an array. @a function should return @c EINA_TRUE
 *          as long as you want the function to continue iterating, by
 *          returning @c EINA_FALSE it stops and returns @c EINA_FALSE as the result.
 *
 *          The data given to @a function is a pointer to the member memory
 *          itself.
 *
 * @param[in] array The array object
 * @param[in] function The callback function
 * @param[in] user_data The user data given to a callback @a function
 * @return @c EINA_TRUE if it successfully iterates all the items of the array
 *
 * @see EINA_INARRAY_FOREACH()
 */
EAPI Eina_Bool eina_inarray_foreach(const Eina_Inarray *array,
                                    Eina_Each_Cb function,
                                    const void *user_data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Removes all the members that match.
 *
 * @details This removes all the entries in @a array, where the @a match function
 *          returns @c EINA_TRUE.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array object
 * @param[in] match The match function
 * @param[in] user_data The user data given to callback @a match
 * @return The number of removed entries, otherwise @c -1 on error
 *
 */
EAPI int eina_inarray_foreach_remove(Eina_Inarray *array,
                                     Eina_Each_Cb match,
                                     const void *user_data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Counts the number of members in an array.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array object
 * @return The number of members in the array
 *
 */
EAPI unsigned int eina_inarray_count(const Eina_Inarray *array) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a new iterator associated to an array.
 *
 * @details This function returns a newly allocated iterator associated to
 *          @a array.
 *
 *          If the memory cannot be allocated, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator is
 *          returned.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks If the array structure changes then the iterator becomes
 *          invalid. That is, if you add or remove members this
 *          iterator's behavior is undefined and your program may crash.
 *
 * @param[in] array The array object
 * @return A new iterator
 *
 */
EAPI Eina_Iterator *eina_inarray_iterator_new(const Eina_Inarray *array) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Returns a new reversed iterator associated to an array.
 *
 * @details This function returns a newly allocated iterator associated to
 *          @a array.
 *
 *          If the memory cannot be allocated, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid iterator is
 *          returned.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks Unlike eina_inarray_iterator_new(), this walks through the array backwards.
 *
 *
 * @remarks If the array structure changes then the iterator becomes
 *          invalid. That is, if you add or remove nodes this iterator's
 *          behavior is undefined and your program may crash.
 *
 * @param[in] array The array object
 * @return A new iterator
 *
 */
EAPI Eina_Iterator *eina_inarray_iterator_reversed_new(const Eina_Inarray *array) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Returns a new accessor associated to an array.
 *
 * @details This function returns a newly allocated accessor associated to
 *          @a array.
 *
 *          If the memory cannot be allocated, @c NULL is returned
 *          and #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, a valid accessor is
 *          returned.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array object
 * @return A new accessor
 *
 */
EAPI Eina_Accessor *eina_inarray_accessor_new(const Eina_Inarray *array) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @def EINA_INARRAY_FOREACH
 * @brief Walks through an array linearly from head to tail.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks @a itr must be a pointer with sizeof(itr*) == array->member_size.
 *
 * @remarks This is fast as it does direct pointer access, but it does
 *          not check for @c NULL pointers or invalid array objects.
 *          Use eina_inarray_foreach() to do that.
 *
 * @remarks Do not modify an array as you walk through it. If that is desired,
 *          then use eina_inarray_foreach_remove().
 *
 * @param array The array object
 * @param itr An iterator pointer
 *
 */
#define EINA_INARRAY_FOREACH(array, itr)                                \
  for ((itr) = (array)->members;					\
       (itr) < (((typeof(*itr)*)(array)->members) + (array)->len);	\
       (itr)++)

/**
 * @def EINA_INARRAY_REVERSE_FOREACH
 * @brief Walks through an array linearly from tail to head.
 *
 * @since 1.2
 *
 * @since_tizen 2.3
 *
 * @remarks @a itr must be a pointer with sizeof(itr*) == array->member_size.
 *
 * @remarks This is fast as it does direct pointer access, but it does
 *          not check for @c NULL pointers or invalid array objects.
 *
 * @remarks Do not modify an array as you walk through it. If that is desired,
 *          then use eina_inarray_foreach_remove().
 *
 * @param array The array object
 * @param itr An iterator pointer
 *
 */
#define EINA_INARRAY_REVERSE_FOREACH(array, itr)                        \
  for ((itr) = ((((typeof(*(itr))*)(array)->members) + (array)->len) - 1); \
       (((itr) >= (typeof(*(itr))*)(array)->members)			\
        && ((array)->members != NULL));					\
       (itr)--)

/**
 * @}
 */

#endif /*EINA_INARRAY_H_*/
