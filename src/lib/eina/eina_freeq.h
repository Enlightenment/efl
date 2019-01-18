#ifndef EINA_FREEQ_H_
#define EINA_FREEQ_H_

#include <stdlib.h>

#include "eina_config.h"

#include "eina_types.h"

/**
 * @addtogroup Eina_FreeQ_Group Free Queue Group
 * @ingroup Eina
 *
 * @brief This provides a mechanism to defer actual freeing of memory
 * data at some time in the future. The main free queue will be driven
 * by the EFL main loop and ensure data is eventually freed.
 *
 * For debugging and tuning you may set the following environment variables,
 * applicable only to free queues of the default type:
 *
 * EINA_FREEQ_BYPASS=1/0
 *
 * Set this environment variable to 1 to immediately bypass the free queue and
 * have all items submitted free with their free function immediately. Set
 * it to 0 to force the free queue to work and delay freeing of items.
 * Note that you can override this by setting count or mem max by
 * eina_freeq_count_max_set() or eina_freeq_mem_max_set() which will
 * disable bypass for that specific free queue. Once bypass is disabled
 * it cannot be re-enabled.
 *
 * EINA_FREEQ_FILL_MAX=N
 *
 * This sets the maximum number of bytes to N an item in the free queue may
 * be in size for the free queue to fill it with debugging values like
 * 0x55 in every byte, to ensure you can see what memory has been freed
 * or not when debugging in tools like gdb. Note that this value is
 * actually one greater than the actual maximum, so if this is set to 100
 * a memory blob of 100 bytes will not be filled but one of 99 bytes in
 * size will be.
 *
 * EINA_FREEQ_TOTAL_MAX=N
 *
 * This sets the maximum number of items allowed to N on a free queue by
 * default before it starts emptying the free queue out to make room.
 *
 * EINA_FREEQ_MEM_MAX=N
 *
 * This sets the maximum total number of Kb (Kilobytes) of memory allowed
 * on a free queue by default to N Kb worth of data.
 *
 * EINA_FREEQ_FILL=N
 *
 * This sets the byte value to write to every byte of an allocation that
 * is added to the free queue when it is added to mark the data as invalid.
 * The default value is 0x55 (85). Memory is only filled if the size of
 * the allocation is less than the max that you can adjust with
 * EINA_FREEQ_FILL_MAX.
 *
 * EINA_FREEQ_FILL_FREED=N
 *
 * Memory just before it is actually passed to the free function to be freed
 * will be filled with this pattern value in every byte. The default value
 * is 0x77 (119). Memory is only filled if the size of the allocation is
 * less than the max that you can adjust with EINA_FREEQ_FILL_MAX.
 * 
 * @{
 *
 * @since 1.19
 * 
 * @typedef Eina_FreeQ
 *
 * A queue of pointers to free in the future. You may create custom free
 * queues of your own to defer freeing, use the main free queue where the
 * mainloop will free things as it iterates, or eina will free everything
 * on shut down.
 * 
 */
typedef struct _Eina_FreeQ Eina_FreeQ;

/** @brief Type of free queues
 *
 * @since 1.19
 */
typedef enum _Eina_FreeQ_Type
{
   /** @brief Default type of free queue.
    *
    * Default free queue, any object added to it should be considered freed
    * immediately. Use this kind of freeq for debugging and additional memory
    * safety purposes only.
    *
    * As this type of free queue is thread-safe, the free functions used must
    * also be thread-safe (e.g.. libc free()).
    *
    * @since 1.19
    */
   EINA_FREEQ_DEFAULT,

   /** @brief Postponed type of free queue.
    *
    * Postponed free queues behave differently in that objects added to it
    * are not to be considered freed immediately, but rather they are
    * short-lived. Use this to return temporary objects that may be used only
    * in the local scope. The queued objects lifetime ends as soon as the
    * execution comes back to the loop. Objects added to this kind of free
    * queue should be accessed exclusively from the same thread that adds them.
    *
    * If a thread does not have a loop attached, the application may leak all
    * those objects. At the moment of writing this means only the main loop
    * should use such a free queue.
    *
    * By default those queues have no memory limit, and will be entirely
    * flushed when the execution comes back to the loop.
    *
    * This type of free queue is not thread-safe and should be considered local
    * to a single thread.
    *
    * @since 1.19
    */
   EINA_FREEQ_POSTPONED,
} Eina_FreeQ_Type;

/**
 * @brief Create a new free queue to defer freeing of data with
 *
 * @return A new free queue
 * @since 1.19
 */
EAPI Eina_FreeQ *
eina_freeq_new(Eina_FreeQ_Type type);

/**
 * @brief Free a free queue and anything that is queued in it.
 *
 * @param[in,out] fq The free queue to free and clear.
 * 
 * @since 1.19
 */
EAPI void
eina_freeq_free(Eina_FreeQ *fq);

/**
 * @brief Query the type of a free queue.
 *
 * @param[in] fq The free queue to inspect.
 *
 * @since 1.19
 */
EAPI Eina_FreeQ_Type
eina_freeq_type_get(Eina_FreeQ *fq);

/**
 * @brief Get the main loop free queue.
 *
 * @return The main loop free queue.
 * 
 * @since 1.19
 */
EAPI Eina_FreeQ *
eina_freeq_main_get(void);

/**
 * @brief Set the maximum number of free pointers this queue is allowed
 *
 * @param[in,out] fq The free queue to alter
 * @param[in] count The maximum number of items allowed, negative values
 *            mean no limit
 *
 * This will alter the maximum number of pointers allowed in the given free
 * queue. If more items are added to the free queue than are allowed,
 * excess items will be freed to make room for the new items. If count is
 * changed, then excess items may be cleaned out at the time this API is
 * called.
 *
 * @note Setting a maximum count on a postponed free queue leads to undefined
 *       behavior.
 *
 * @since 1.19
 */
EAPI void
eina_freeq_count_max_set(Eina_FreeQ *fq, int count);

/**
 * @brief Get the maximum number of free pointers this queue is allowed
 *
 * @param[in] fq The free queue to query
 * @return The maximum number of free items allowed or -1 for infinity
 * 
 * @since 1.19
 */
EAPI int
eina_freeq_count_max_get(Eina_FreeQ *fq);

/**
 * @brief Set the maximum amount of memory allowed
 *
 * @param[in,out] fq The free queue to alter
 * @param[in] mem The maximum memory in bytes
 *
 * This will alter the maximum amount of memory allowed for pointers stored
 * in the free queue. The size used is the size give, so items given that
 * are 0 sized will not contribute to this limit. If items with a total
 * memory footprint are added to the free queue, items will be cleaned out
 * until the total is below this limit. Changing the limit may involve
 * cleaning out excess items from the free queue until the total amount of
 * memory used by items in the queue is below or at the limit.
 *
 * @note Setting a memory limit on a postponed free queue leads to undefined
 *       behavior.
 * 
 * @since 1.19
 */
EAPI void
eina_freeq_mem_max_set(Eina_FreeQ *fq, size_t mem);

/**
 * @brief Get the maximum amount of memory allowed
 *
 * @param[in] fq The free queue to query
 * @return The maximum amount of memory in bytes
 *
 * @since 1.19
 */
EAPI size_t
eina_freeq_mem_max_get(Eina_FreeQ *fq);

/**
 * @brief Clear out all queued items to be freed by freeing them
 *
 * @param[in,out] fq The free queue to clear
 *
 * This will free and thus remove all queued items from the free queue when
 * this function is called. When it returns the free queue should be
 * empty.
 * 
 * @since 1.19
 */
EAPI void
eina_freeq_clear(Eina_FreeQ *fq);

/**
 * @brief Reduce the number of items in the free queue by up to @p count
 *
 * @param[in,out] fq The free queue to reduce in item count
 * @param[in] count The number of items to try and free
 *
 * This will attempt to free up to @p count items from the given free queue
 * and thus reduce the amount of memory it is holding on to. This function
 * will return once it has removed @p count items or there are no more items
 * to remove from the queue.
 * 
 * @since 1.19
 */
EAPI void
eina_freeq_reduce(Eina_FreeQ *fq, int count);

/**
 * @brief Return if there are any items pending a free in the free queue
 *
 * @param[in] fq The free queue to query
 * @raturn EINA_TRUE if there are items to free, EINA_FALSE otherwise
 * 
 * @since 1.19
 */
EAPI Eina_Bool
eina_freeq_ptr_pending(Eina_FreeQ *fq);

/**
 * @brief Add a pointer with free function and size to the free queue
 *
 * @param[in,out] fq The free queue to add the pointer to
 * @param[in] ptr The pointer to free
 * @param[in] free_func The function used to free the pointer with
 * @param[in] size The size of the data the pointer points to
 *
 * This adds the given @p ptr pointer to the queue to be freed later on.
 * The function @p free_func will be used, or if this is NULL, it is assumed
 * the libc free() function will be used then instead. The @p size parameter
 * determines the size of the data pointed to, but if this is 0 then no
 * assumptions are made about size and the pointer is considered opaque. A
 * zero sized pointer will not contribute to the total memory usage of
 * items in the queue as well. If @p size is supplied it must be correct
 * as the memory may be written to for debugging purposes or otherwise
 * inspected or checksummed. Once a pointer is added to the free queue
 * with this API the memory should be considered freed as if the real
 * @p free_func was called immediately (and it may actually be called
 * immediately if certain environment variables are set). A free queue exists
 * to move the cost of freeing to another point in time when it is more
 * convenient to do so as well as provide some robustness for badly
 * written code that may access memory after freeing. Note that when using
 * tools like valgrind, eina detects this and will also immediately free
 * the data so valgrind's own memory checkers can detect use after free
 * as normal.
 *
 * Note: The free function must not access the CONTENT of the memory to be
 * freed, or at least consider it invalid and full of garbage. It is already
 * invalid at the moment it is added to the free queue. Just the actual
 * free function may be delayed. The free function may also not access other
 * memory already freed before being added to the free queue. They may do
 * tricks like use memory headers that are outside the memory region to be
 * freed (pass in pointer char *x, then char *header_address = x - 16 to get
 * header information) as this header is not considered part of the free data.
 * This note does not apply if you use a size of 0 for the pointer, but then
 * you lose canary debugging ability when using 0 sized pointers on the free
 * queue.
 * 
 * @since 1.19
 */
EAPI void
eina_freeq_ptr_add(Eina_FreeQ *fq, void *ptr, void (*free_func) (void *ptr), size_t size);

/**
 * @brief Add a pointer to the main free queue
 *
 * @param[in] ptr The pointer to free
 * @param[in] free_func The function used to free the pointer with
 * @param[in] size The size of the data the pointer points to
 * 
 * This is the same as eina_freeq_ptr_add() but the main free queue is
 * fetched by eina_freeq_main_get().
 * 
 * @since 1.19
 */
static inline void
eina_freeq_ptr_main_add(void *ptr, void (*free_func) (void *ptr), size_t size)
{
   eina_freeq_ptr_add(eina_freeq_main_get(), ptr, free_func, size);
}

/**
 * @brief Convenience macro for well known structures and types
 *
 * @param[in] ptr The pointer to free
 * 
 * This is the same as eina_freeq_ptr_main_add() but the free function is
 * assumed to be the libc free() function, and size is provided by
 * sizeof(*ptr), so it will not work on void pointers or will be inaccurate
 * for pointers to arrays. For arrays please use EINA_FREEQ_ARRAY_FREE()
 * 
 * @since 1.19
 */
#define EINA_FREEQ_FREE(ptr) eina_freeq_ptr_main_add(ptr, NULL, sizeof(*(ptr)))

/**
 * @brief Convenience macro for well known structures and types
 *
 * @param[in] ptr The pointer to free
 * 
 * This is the same as eina_freeq_ptr_main_add() but the free function is
 * assumed to be the libc free() function, and size is provided by
 * sizeof(*ptr), so it will not work on void pointers. Total size is multiplied
 * by the count @p n so it should work well for arrays of types.
 * 
 * @since 1.19
 */
#define EINA_FREEQ_N_FREE(ptr, n) eina_freeq_ptr_main_add(ptr, NULL, sizeof(*(ptr)) * n)

/**
 * @}
 */

#endif
