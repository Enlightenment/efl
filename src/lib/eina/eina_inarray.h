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

#ifndef EINA_INARRAY_H_
#define EINA_INARRAY_H_

#include "eina_types.h"
#include "eina_iterator.h"
#include "eina_accessor.h"

/**
 * @page eina_inarray_example_01 Eina inline array usage
 * @dontinclude eina_inarray_01.c
 *
 * This example creates an inline array of chars, adds some elements, prints
 * them, re-purposes the array to store ints, adds some elements and prints that.
 *
 * We are going to start with a function to compare ints. We need this because the '>'
 * operator is not a function and can't be used where Eina_Compare_Cb is needed.
 * @skip int
 * @until }
 *
 * And then move on to the code we actually care about, starting with variable
 * declarations and eina initialization:
 * @until eina_init
 *
 * Creating an inline array is very simple, we just need to know what type we
 * want to store:
 * @until inarray_new
 * @note The second parameter(the step) is left at zero which means that eina
 * chooses an appropriate value, this should @b only be changed if it's
 * known, beforehand, that how many elements the array has.
 *
 * Once we have an array we can start adding elements to it. Because the
 * insertion function expects a memory address we have to put the value we want
 * to store in a variable (this should be no problem since in the real world usage
 * that's usually where the value is anyways):
 * @until push
 * @note Because the inline array copies the value given to it we can later
 * change @c ch, which we do, without affecting the contents of the array.
 *
 * So let's add some more elements:
 * @until push
 * @until push
 * @until push
 *
 * We then iterate over our array and print every position of it. The thing
 * to note here is not so much the values, which are the expected 'a', 'b',
 * 'c', and 'd', but rather the memory address of these values, they are
 * sequential:
 * @until printf
 * @until printf
 *
 * We are now going to use our array to store ints, so we need to first erase every member
 * currently on the array:
 * @until _flush
 *
 * And then to be able to store a different type on the same array, we use the
 * eina_inarray_step_set() function, which is just like the eina_inarray_new()
 * function except it receives an already allocated memory. This time we're going
 * to ask eina to use a step of size 4 because that's how many elements we are going to
 * put in the array:
 * @until _step_set
 * @note Strictly speaking the reason to call eina_inarray_step_set() is not
 * because we're storing a different type, but because our types have
 * different sizes. Eina inline arrays don't actually know anything about types,
 * they only deal with blocks of memory of a given size.
 * @note Since eina_inarray_step_set() receives already allocated memory, you can (and
 * it is in fact a good practice) use inline arrays that are not declared as pointers:
 * @code
 * Eina_Inarray arr;
 * eina_inarray_step_set(&arr, sizeof(arr), sizeof(int), 4);
 * @endcode
 *
 * And now to add our integer values to the array:
 * @until push
 * @until push
 * @until push
 *
 * Just to change things up a bit we've left out the 99 value, but we still
 * add it in such a way that it keeps the array ordered. There are many ways to do
 * this, we could use eina_inarray_insert_at(), or we could change the value
 * of the last member using eina_inarray_replace_at() and then append the values
 * in the right order, but for no particular reason we're going to use
 * eina_inarray_insert_sorted() instead:
 * @until insert_sorted
 *
 * We then print the size of our array, and the array itself, much like last
 * time the values are not surprising, and neither should it be that the memory
 * addresses are contiguous:
 * @until printf
 * @until printf
 *
 * Once done we free our array and shutdown eina:
 * @until }
 *
 * The source for this example: @ref eina_inarray_01_c
 */

/**
 * @page eina_inarray_01_c eina_inarray_01.c
 * @include eina_inarray_01.c
 * @example eina_inarray_01.c
 */

/**
 * @page eina_inarray_example_02 Eina inline array of strings
 * @dontinclude eina_inarray_02.c
 *
 * This example creates an inline array of strings, adds some elements, and
 * then prints them. This example is based on @ref eina_array_01_example_page and
 * @ref eina_inarray_example_01.
 *
 * We start with some variable declarations and eina initialization:
 * @skip int
 * @until eina_init
 *
 * We then create the array much like we did on @ref eina_inarray_example_01 :
 * @until inarray_new
 *
 * The point is this example significantly differs from the first eina inline
 * array example. We are not going to add the strings themselves to the array since
 * their size varies, we are going to store a pointer to the strings instead. We therefore
 * use @c char** to populate our inline array:
 * @until }
 *
 * The source for this example: @ref eina_inarray_02_c
 */

/**
 * @page eina_inarray_02_c eina_inarray_02.c
 * @include eina_inarray_02.c
 * @example eina_inarray_02.c
 */

/**
 * @page eina_inarray_example_03 Eina inline array insert, sort and search
 * @dontinclude eina_inarray_03.c
 *
 * This example creates an inline array of integers, and demonstrates the
 * difference between eina_inarray_insert and eina_inarray_sort, and
 * eina_inarray_search and eina_inarray_search_sort.
 * @ref eina_inarray_example_01.
 *
 * We start with some variable declarations and eina initialization:
 * @skip int
 * @until eina_init
 *
 * We then create the array much like we did on @ref eina_inarray_example_01 :
 * @until inarray_new
 *
 * We then add an element using eina_inarray_insert and print. Then remove that
 * element and add it again using eina_inarray_insert_sorted and print. This
 * shows the 2 different positions the element gets added. Then search for an
 * element in the unsorted array using eina_inarray_search, then sorts the
 * array and then searches the same element using eina_inarray_search_sorted.
 * @until }
 *
 * The source for this example: @ref eina_inarray_03_c
 */

/**
 * @page eina_inarray_03_c eina_inarray_03.c
 * @include eina_inarray_03.c
 * @example eina_inarray_03.c
 */

/**
 * @defgroup Eina_Inline_Array_Group Inline Array
 * @ingroup Eina_Containers_Group
 * @since 1.2
 *
 * @brief Inline array is a container that stores the data itself, not the pointers to the data.
 *
 * This means there is no memory fragmentation, also for small data types (such
 * as char, short, int, and so on) it's more memory efficient.
 *
 * Usage of the inline array is very similar to that of other
 * @ref Eina_Containers_Group, like all arrays adding elements to the beginning
 * of the array is a lot more costly than appending, so those operations should
 * be minimized.
 *
 * Examples:
 * @li @ref eina_inarray_example_01
 * @li @ref eina_inarray_example_02
 *
 * @{
 */


/**
 * @typedef Eina_Inarray
 * @brief Type for the inlined array.
 *
 * @since 1.2
 */
typedef struct _Eina_Inarray Eina_Inarray;

/**
 * @brief Inline array structure.
 *
 * @note Use #Eina_Inarray instead.
 *
 * @note Do not modify these fields directly, use eina_inarray_step_set() or
 *       eina_inarray_new() instead.
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
 * @details This creates a new array where members are inlined in a sequence. Each
 *          member has @a member_size bytes.
 *
 * @param[in] member_size The size of each member in the array
 * @param[in] step The step size by which to resize the array, do this using the following
 *             extra amount
 * @return The new inline array table, otherwise @c NULL on failure
 *
 * @note If the @a step is @c 0, then a safe default is chosen.
 *
 * @note On failure, @c NULL is returned. If @p member_size is zero, then @c NULL is returned.
 *
 * @see eina_inarray_free()
 *
 * @since 1.2
 */
EAPI Eina_Inarray *eina_inarray_new(unsigned int member_size,
                                    unsigned int step) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Frees an array and its members.
 *
 * @param[in] array The array object
 *
 * @see eina_inarray_flush()
 *
 * @since 1.2
 */
EAPI void eina_inarray_free(Eina_Inarray *array) EINA_ARG_NONNULL(1);

/**
 * @brief Initializes an inline array.
 * @details This initializes an array. If the @p step is @c 0, then a safe default is
 *          chosen.
 *
 * @param[in] array The array object to initialize
 * @param[in] sizeof_eina_inarray The size of array object
 * @param[in] member_size The size of each member in the array
 * @param[in] step The step size by which to resize the array, do this using the following
 *            extra amount
 *
 * @note This is useful for arrays inlined into other structures or
 *       allocated to a stack.
 *
 * @see eina_inarray_flush()
 *
 * @since 1.2
 */
EAPI void eina_inarray_step_set(Eina_Inarray *array,
                                unsigned int sizeof_eina_inarray,
                                unsigned int member_size,
                                unsigned int step) EINA_ARG_NONNULL(1);

/**
 * @brief Removes every member from the array.
 *
 * @param[in] array The array object
 *
 * @since 1.2
 */
EAPI void eina_inarray_flush(Eina_Inarray *array) EINA_ARG_NONNULL(1);

/**
 * @brief Copies the data as the last member of the array.
 * @details This copies the given pointer contents at the end of the array. The
 *          pointer is not referenced, instead its contents are copied to the
 *          members array using the previously defined @c member_size.
 *
 * @param[in] array The array object
 * @param[in] data The data to be copied at the end
 * @return The index of the new member, otherwise @c -1 on errors
 *
 * @see eina_inarray_insert_at()
 *
 * @since 1.2
 */
EAPI int eina_inarray_push(Eina_Inarray *array,
                           const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Allocates new item at the end of the array.
 *
 * @param[in] array The array object
 * @param[in] size  The number of new item to allocate
 *
 * @note The returned pointer is only valid until you use any other eina_inarray
 *       function.
 *
 * @since 1.8
 */
EAPI void *eina_inarray_grow(Eina_Inarray *array, unsigned int size);

/**
 * @brief Copies the data to the array at a position found by the comparison function.
 * @details This copies the given pointer contents at the array position defined by the
 *          given @a compare function. The pointer is not referenced, instead
 *          its contents are copied to the members array using the previously
 *          defined @c member_size.
 *
 * @param[in] array The array object
 * @param[in] data The data to be copied
 * @param[in] compare The compare function
 * @return The index of the new member, otherwise @c -1 on errors
 *
 * @note The data given to the @p compare function is a pointer to the member
 *       memory itself, do no change it.
 *
 * @see eina_inarray_insert_sorted()
 * @see eina_inarray_insert_at()
 * @see eina_inarray_push()
 *
 * @since 1.2
 */
EAPI int eina_inarray_insert(Eina_Inarray *array,
                             const void *data,
                             Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Copies the data to the array at a position found by the comparison function.
 * @details This copies the given pointer contents at the array position defined by the
 *          given @p compare function. The pointer is not referenced, instead
 *          its contents are copied to the members array using the previously
 *          defined @p member_size.
 *
 * @param[in] array The array object
 * @param[in] data The data to be copied
 * @param[in] compare The compare function
 * @return The index of the new member, otherwise @c -1 on errors
 *
 * @note The data given to the @p compare function is a pointer to the member
 *       memory itself, do no change it.
 *
 * @note This variation optimizes the insertion position assuming that the array
 *       is already sorted by doing a binary search.
 *
 * @see eina_inarray_sort()
 *
 * @since 1.2
 */
EAPI int eina_inarray_insert_sorted(Eina_Inarray *array,
                                    const void *data,
                                    Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Finds data and removes the matching member.
 * @details This finds data in the array and removes it. Data may be an existing
 *          member of the array (then optimized) or the contents are matched
 *          using memcmp().
 *
 * @param[in] array The array object
 * @param[in] data The data to be found and removed
 * @return The index of the removed member, otherwise @c -1 on errors
 *
 * @see eina_inarray_pop()
 * @see eina_inarray_remove_at()
 *
 * @since 1.2
 */
EAPI int eina_inarray_remove(Eina_Inarray *array,
                             const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Removes the last member of the array.
 *
 * @param[in] array The array object
 * @return The data popped out of the array
 *
 * @note The data could be considered valid only until any other operation touched the Inarray.
 *
 * @since 1.2
 */
EAPI void *eina_inarray_pop(Eina_Inarray *array) EINA_ARG_NONNULL(1);

/**
 * @brief Gets the member at the given position.
 * @details This gets the member given that its position in the array is provided. It is a pointer to
 *          its current memory, then it can be invalidated with functions that
 *          change the array such as eina_inarray_push(),
 *          eina_inarray_insert_at(), or eina_inarray_remove_at(), or variants.
 *
 * @param[in] array The array object
 * @param[in] position The member position
 * @return A pointer to current the member memory
 *
 * @since 1.2
 */
EAPI void *eina_inarray_nth(const Eina_Inarray *array,
                            unsigned int position) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Copies the data at the given position in the array.
 * @details This copies the given pointer contents at the given @p position in the
 *          array. The pointer is not referenced, instead its contents are
 *          copied to the members array using the previously defined
 *          @p member_size.
 *
 * @param[in] array The array object
 * @param[in] position The position to insert the member at
 * @param[in] data The data to be copied at the position
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE on failure
 *
 * @note All the members from @a position to the end of the array are
 *       shifted to the end.
 *
 * @note If @a position is equal to the end of the array (equal to
 *       eina_inarray_count()), then the member is appended.
 *
 * @note If @a position is bigger than the array length, it fails.
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_inarray_insert_at(Eina_Inarray *array,
                                      unsigned int position,
                                      const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Opens a space at the given position, returning its pointer.
 *
 * @param[in] array The array object
 * @param[in] position The position to insert first member at (open/allocate space)
 * @param[in] member_count The number of times @c member_size bytes are allocated
 * @return A pointer to the first member memory allocated, otherwise @c NULL on errors
 *
 * @note This is similar to eina_inarray_insert_at(), but useful if the
 *       members contents are still unknown or unallocated. It makes
 *       room for the required number of items and returns the pointer to the
 *       first item, similar to malloc(member_count * member_size), with the
 *       guarantee that all the memory is within the members array.
 *
 * @note The new member memory is undefined, it's not automatically zeroed.
 *
 * @note All the members from @a position to the end of the array are
 *       shifted to the end.
 *
 * @note If @a position is equal to the end of the array (equal to
 *       eina_inarray_count()), then the member is appended.
 *
 * @note If @a position is bigger than the array length, it fails.
 *
 * @since 1.2
 */
EAPI void *eina_inarray_alloc_at(Eina_Inarray *array,
                                 unsigned int position,
                                 unsigned int member_count) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Copies the data to the given position.
 * @details This copies the given pointer contents at the given @p position in the
 *          array. The pointer is not referenced, instead its contents are
 *          copied to the members array using the previously defined
 *          @p member_size.
 *
 * @param[in] array The array object
 * @param[in] position The position to copy the member at
 * @param[in] data The data to be copied at the position
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE on failure
 *
 * @note If @p position does not exist, it fails.
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_inarray_replace_at(Eina_Inarray *array,
                                       unsigned int position,
                                       const void *data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Removes a member from the given position.
 *
 * @param[in] array The array object
 * @param[in] position The position from which to remove a member
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE on failure
 *
 * @note The member is removed from an array and members after it are moved
 *       towards the array head.
 *
 * @see eina_inarray_pop()
 * @see eina_inarray_remove()
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_inarray_remove_at(Eina_Inarray *array,
                                      unsigned int position) EINA_ARG_NONNULL(1);

/**
 * @brief Reverses members in the array.
 *
 * @param[in] array The array object
 *
 * @note If you do not want to change the array, just walk through its elements
 *       backwards, then use the EINA_INARRAY_REVERSE_FOREACH() macro.
 *
 * @see EINA_INARRAY_REVERSE_FOREACH()
 *
 * @since 1.2
 */
EAPI void eina_inarray_reverse(Eina_Inarray *array) EINA_ARG_NONNULL(1);

/**
 * @brief Applies a quick sort to the array.
 * @details This applies a quick sort to the @a array.
 *
 * @param[in] array The array object
 * @param[in] compare The compare function
 *
 * @note The data given to the @a compare function is a pointer to the member
 *       memory itself, do no change it.
 *
 * @see eina_inarray_insert_sorted()
 *
 * @since 1.2
 */
EAPI void eina_inarray_sort(Eina_Inarray *array,
                            Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Searches for a member (linear walk).
 * @details This walks through an array by linearly looking for the given data compared by
 *          the @p compare function.
 *
 * @param[in] array The array object
 * @param[in] data The member to search using the @p compare function
 * @param[in] compare The compare function
 * @return The member index, otherwise @c -1 if not found
 *
 * @note The data given to the @p compare function is a pointer to the member
 *       memory itself, do no change it.
 *
 * @since 1.2
 */
EAPI int eina_inarray_search(const Eina_Inarray *array,
                             const void *data,
                             Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Searches for member (binary search walk).
 *
 * @param[in] array The array object
 * @param[in] data The member to search using the @p compare function
 * @param[in] compare The compare function
 * @return The member index, otherwise @c -1 if not found
 *
 * @note Uses a binary search for the given data as compared by the @p compare function.
 *
 * @note The data given to the @p compare function is a pointer to the member
 *       memory itself, do no change it.
 *
 * @since 1.2
 */
EAPI int eina_inarray_search_sorted(const Eina_Inarray *array,
                                    const void *data,
                                    Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Calls @p function for each array member.
 * @details This calls @p function for every given data in @p array.
 *
 * @param[in] array The array object
 * @param[in] function The callback function
 * @param[in] user_data The user data given to a callback @a function
 * @return #EINA_TRUE if it successfully iterates all the items of the array
 *
 * @note This is a safe way to iterate over an array. @p function should return #EINA_TRUE
 *       as long as you want the function to continue iterating, by
 *       returning #EINA_FALSE it stops and returns #EINA_FALSE as the result.
 *
 * @note The data given to @p function is a pointer to the member memory itself.
 *
 * @see EINA_INARRAY_FOREACH()
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_inarray_foreach(const Eina_Inarray *array,
                                    Eina_Each_Cb function,
                                    const void *user_data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Removes all the members that match.
 * @details This removes all the entries in @p array, where the @p match function
 *          returns #EINA_TRUE.
 *
 * @param[in] array The array object
 * @param[in] match The match function
 * @param[in] user_data The user data given to callback @p match
 * @return The number of removed entries, otherwise @c -1 on error
 *
 * @since 1.2
 */
EAPI int eina_inarray_foreach_remove(Eina_Inarray *array,
                                     Eina_Each_Cb match,
                                     const void *user_data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Resizes array to new size
 *
 * @param[in] array The array object
 * @param[in] new_size New size for resize
 * @return #EINA_TRUE if it resized the array successfully.
 *
 * @since 1.10
 */
EAPI Eina_Bool eina_inarray_resize(Eina_Inarray *array, unsigned int new_size);

/**
 * @brief Counts the number of members in an array.
 *
 * @param[in] array The array object
 * @return The number of members in the array
 *
 * @since 1.2
 */
EAPI unsigned int eina_inarray_count(const Eina_Inarray *array) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a new iterator associated to an array.
 * @details This function returns a newly allocated iterator associated to
 *          @p array.
 *
 * @param[in] array The array object
 * @return A new iterator
 *
 * @note If the memory cannot be allocated, @c NULL is returned.
 *       Otherwise, a valid iterator is returned.
 *
 * @warning If the array structure changes then the iterator becomes
 *          invalid. That is, if you add or remove members this
 *          iterator's behavior is undefined and your program may crash.
 *
 * @since 1.2
 */
EAPI Eina_Iterator *eina_inarray_iterator_new(const Eina_Inarray *array) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Returns a new reversed iterator associated to an array.
 * @details This function returns a newly allocated iterator associated to
 *          @p array.
 *
 * @param[in] array The array object
 * @return A new iterator
 *
 * @note Unlike eina_inarray_iterator_new(), this walks through the array backwards.
 *
 * @note If the memory cannot be allocated, @c NULL is returned.
 *       Otherwise, a valid iterator is returned.
 *
 * @warning If the array structure changes then the iterator becomes
 *          invalid. That is, if you add or remove nodes this iterator's
 *          behavior is undefined and your program may crash.
 *
 * @since 1.2
 */
EAPI Eina_Iterator *eina_inarray_iterator_reversed_new(const Eina_Inarray *array) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Returns a new accessor associated to an array.
 * @details This function returns a newly allocated accessor associated to
 *          @p array.
 *
 * @param[in] array The array object
 * @return A new accessor
 *
 * @note If the memory cannot be allocated, @c NULL is returned
 *       Otherwise, a valid accessor is returned.
 *
 * @since 1.2
 */
EAPI Eina_Accessor *eina_inarray_accessor_new(const Eina_Inarray *array) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @def EINA_INARRAY_FOREACH
 * @brief Walks through an array linearly from head to tail.
 *
 * @param[in] array The array object
 * @param[in] itr An iterator pointer
 *
 * @note @p itr must be a pointer with sizeof(itr*) == array->member_size.
 *
 * @note This is fast as it does direct pointer access, but it does
 *       not check for @c NULL pointers or invalid array objects.
 *       Use eina_inarray_foreach() to do that.
 *
 * @note Do not modify an array as you walk through it. If that is desired,
 *       then use eina_inarray_foreach_remove().
 *
 * @since 1.2
 */
#define EINA_INARRAY_FOREACH(array, itr)                                \
  for ((itr) = (array)->members;					\
       (itr) < (((__typeof__(*itr)*)(array)->members) + (array)->len);	\
       (itr)++)

/**
 * @def EINA_INARRAY_REVERSE_FOREACH
 * @brief Walks through an array linearly from tail to head.
 *
 * @param[in] array The array object
 * @param[in] itr An iterator pointer
 *
 * @note @p itr must be a pointer with sizeof(itr*) == array->member_size.
 *
 * @note This is fast as it does direct pointer access, but it does
 *       not check for @c NULL pointers or invalid array objects.
 *
 * @note Do not modify an array as you walk through it. If that is desired,
 *       then use eina_inarray_foreach_remove().
 *
 * @since 1.2
 */
#define EINA_INARRAY_REVERSE_FOREACH(array, itr)                        \
  for ((itr) = ((((__typeof__(*(itr))*)(array)->members) + (array)->len) - 1); \
       (((itr) >= (__typeof__(*(itr))*)(array)->members)		\
	&& ((array)->members != NULL));					\
       (itr)--)

/**
 * @}
 */

#endif /*EINA_INARRAY_H_*/
