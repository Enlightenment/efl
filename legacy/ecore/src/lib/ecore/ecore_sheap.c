#include "ecore_private.h"
#include "Ecore.h"
#include "Ecore_Data.h"

#define HEAP_INCREMENT 4096

#define PARENT(i) (i / 2)
#define LEFT(i) (2 * i)
#define RIGHT(i) (2 * i + 1)

static void _ecore_sheap_heapify(Ecore_Sheap *heap, int i);
static void _ecore_sheap_update_data(Ecore_Sheap *heap);

/**
 * Allocate and initialize a new binary heap
 * @param compare The function for comparing keys, NULL for direct comparison
 * @param size    The number of elements to allow in the heap
 * @return A pointer to the newly allocated binary heap on success, NULL on
 * failure.
 */
EAPI Ecore_Sheap *
ecore_sheap_new(Ecore_Compare_Cb compare, int size)
{
   Ecore_Sheap *heap = NULL;

   heap = (Ecore_Sheap *)malloc(sizeof(Ecore_Sheap));
   if (!heap)
     return NULL;
   memset(heap, 0, sizeof(Ecore_Sheap));

   if (!ecore_sheap_init(heap, compare, size))
     {
	FREE(heap);
	return NULL;
     }

   return heap;
}

/**
 * Initialize a binary heap to default values
 * @param heap    The heap to initialize
 * @param compare The function for comparing keys, NULL for direct comparison
 * @param size    The number of elements to allow in the heap
 * @return        TRUE on success, FALSE on failure
 */
EAPI int 
ecore_sheap_init(Ecore_Sheap *heap, Ecore_Compare_Cb compare, int size)
{
   CHECK_PARAM_POINTER_RETURN("heap", heap, FALSE);

   heap->space = size;
   if (!compare)
     heap->compare = ecore_direct_compare;
   else
     heap->compare = compare;
   heap->order = ECORE_SORT_MIN;

   heap->data = (void **)malloc(heap->space * sizeof(void *));
   if (!heap->data)
     return FALSE;
   memset(heap->data, 0, heap->space * sizeof(void *));

   return TRUE;
}

/**
 * Free up the memory used by the heap
 *
 * Frees the memory used by @a heap, calls the destroy function on each data
 * item if necessary.
 *
 * @param  heap The heap to be freed
 */
EAPI void 
ecore_sheap_destroy(Ecore_Sheap *heap)
{
   int i;

   CHECK_PARAM_POINTER("heap", heap);

   /*
    * Free data in heap
    */
   if (heap->free_func)
     for (i = 0; i < heap->size; i++)
       heap->free_func(heap->data[i]);

   FREE(heap->data);

   FREE(heap);
}

/**
 * Set the function for freeing data.
 * @param  heap      The heap that will use this function when nodes are
 *                   destroyed.
 * @param  free_func The function that will free the key data.
 * @return @c TRUE on successful set, @c FALSE otherwise.
 */
EAPI int 
ecore_sheap_free_cb_set(Ecore_Sheap *heap, Ecore_Free_Cb free_func)
{
   CHECK_PARAM_POINTER_RETURN("heap", heap, FALSE);

   heap->free_func = free_func;

   return TRUE;
}

/**
 * Insert new data into the heap.
 * @param  heap The heap to insert @a data.
 * @param  data The data to add to @a heap.
 * @return TRUE on success, NULL on failure. Increases the size of the heap if
 *         it becomes larger than available space.
 */
EAPI int 
ecore_sheap_insert(Ecore_Sheap *heap, void *data)
{
   int i;
   void *temp;
   int parent;
   int position;

   CHECK_PARAM_POINTER_RETURN("heap", heap, FALSE);

   /*
    * Increase the size of the allocated data area if there isn't enough
    * space available to add this data
    */
   if (heap->size >= heap->space)
     return FALSE;

   heap->sorted = FALSE;

   /*
    * Place the data at the end of the heap initially. Then determine the
    * parent and position in the array of it's parent.
    */
   heap->data[heap->size] = data;
   position = heap->size;
   heap->size++;
   i = heap->size;
   parent = PARENT(i) - 1;

   /*
    * Check the order of the heap to decide where to place the inserted
    * data. The loop is placed inside the if statement to reduce the
    * number of branching decisions that must be predicted.
    */
   if (heap->order == ECORE_SORT_MIN)
     {
	while ((position > 0) && heap->compare(heap->data[parent],
					       heap->data[position]) > 0)
	  {

	     /*
	      * Swap the data with it's parents to move it up in
	      * the heap.
	      */
	     temp = heap->data[position];
	     heap->data[position] = heap->data[parent];
	     heap->data[parent] = temp;

	     /*
	      * Now determine the new position for the next
	      * iteration of the loop, as well as it's parents
	      * position.
	      */
	     i = PARENT(i);
	     position = i - 1;
	     parent = PARENT(i) - 1;
	  }
     }
   else
     {
	while ((position > 0) && heap->compare(heap->data[parent],
					       heap->data[position]) < 0)
	  {

	     /*
	      * Swap the data with it's parents to move it up in
	      * the heap.
	      */
	     temp = heap->data[position];
	     heap->data[position] = heap->data[PARENT(i) - 1];
	     heap->data[PARENT(i) - 1] = temp;

	     /*
	      * Now determine the new position for the next
	      * iteration of the loop, as well as it's parents
	      * position.
	      */
	     i = PARENT(i);
	     position = i - 1;
	     parent = PARENT(i) - 1;
	  }
     }

   return TRUE;
}

/**
 * Extract the item at the top of the heap
 * @param  heap The heap to remove the top item
 * @return The top item of the heap on success, NULL on failure.
 * @note   The extract function maintains the heap properties after the
 *         extract.
 */
EAPI void *
ecore_sheap_extract(Ecore_Sheap *heap)
{
   void *extreme;

   if (heap->size < 1)
     return NULL;

   heap->sorted = FALSE;

   extreme = heap->data[0];
   heap->size--;
   heap->data[0] = heap->data[heap->size];

   _ecore_sheap_heapify(heap, 1);

   return extreme;
}

/**
 * Examine the item at the top of the heap
 * @param  heap The heap to examine the top item
 * @return The top item of the heap on success, NULL on failure.
 * @note   The function does not alter the heap.
 */
EAPI void *
ecore_sheap_extreme(Ecore_Sheap *heap)
{
   if (heap->size < 1)
     return NULL;

   return heap->data[0];
}

/**
 * Change the value of the specified item in the heap
 * @param heap   The heap to search for the item to change
 * @param item   The item in the heap to change
 * @param newval The new value assigned to the item in the heap
 * @return       TRUE on success, FALSE on failure.
 * @note         The heap does not free the old data since it must be passed
 *               in, so the caller can perform the free if desired.
 */
EAPI int 
ecore_sheap_change(Ecore_Sheap *heap, void *item, void *newval)
{
   int i;

   CHECK_PARAM_POINTER_RETURN("heap", heap, FALSE);

   for (i = 0; i < heap->size && heap->compare(heap->data[i], item); i++);

   if (i < heap->size)
     heap->data[i] = newval;
   else
     return FALSE;

   /*
    * FIXME: This is not the correct procedure when a change occurs.
    */
   _ecore_sheap_heapify(heap, 1);

   return TRUE;
}

/**
 * Change the comparison function for the heap
 * @param  heap    The heap to change comparison function
 * @param  compare The new function for comparing nodes
 * @return TRUE on success, FALSE on failure.
 *
 * The comparison function is changed to @compare and the heap is heapified
 * by the new comparison.
 */
EAPI int 
ecore_sheap_compare_set(Ecore_Sheap *heap, Ecore_Compare_Cb compare)
{
   CHECK_PARAM_POINTER_RETURN("heap", heap, FALSE);

   if (!compare)
     heap->compare = ecore_direct_compare;
   else
     heap->compare = compare;

   _ecore_sheap_update_data(heap);

   return TRUE;
}

/**
 * Change the order of the heap
 * @param  heap  The heap to change the order
 * @param  order The new order of the heap
 *
 * Changes the heap order of @heap and re-heapifies the data to this new
 * order. The default order is a min heap.
 */
EAPI void 
ecore_sheap_order_set(Ecore_Sheap *heap, char order)
{
   CHECK_PARAM_POINTER("heap", heap);

   heap->order = order;

   _ecore_sheap_update_data(heap);
}

/**
 * Sort the data in the heap
 * @param  heap The heap to be sorted
 *
 * Sorts the data in the heap into the order that is used for the heap's
 * data.
 */
EAPI void 
ecore_sheap_sort(Ecore_Sheap *heap)
{
   int i = 0;
   void **new_data;

   CHECK_PARAM_POINTER("heap", heap);

   new_data = (void **)malloc(heap->size * sizeof(void *));

   /*
    * Extract the heap and insert into the new data array in order.
    */
   while (heap->size > 0)
     new_data[i++] = ecore_sheap_extract(heap);

   /*
    * Free the old data array and update the heap with the new data, also
    * mark as sorted.
    */
   FREE(heap->data);
   heap->data = new_data;
   heap->size = i;
   heap->sorted = TRUE;
}

/*
 * Access the item at the ith position in the heap
 * @param  heap The heap to access the internal data
 * @param  i    The index of the data within the heap
 * @return The data located at the ith position within @heap on success,
 *         NULL on failure.
 * @note   The data is guaranteed to be in sorted order.
 */
EAPI inline void *
ecore_sheap_item(Ecore_Sheap *heap, int i)
{
   if (i >= heap->size)
     return NULL;

   /*
    * Make sure the data is sorted so we return the correct value.
    */
   if (!heap->sorted)
     ecore_sheap_sort(heap);

   return heap->data[i];
}

/*
 * Regain the heap properties starting at position i
 * @param  heap The heap to regain heap properties
 * @param  i    The position to start heapifying
 */
static void 
_ecore_sheap_heapify(Ecore_Sheap *heap, int i)
{
   int extreme;
   int left = LEFT(i);
   int right = RIGHT(i);

   if (heap->order == ECORE_SORT_MIN)
     {
	if (left <= heap->size && heap->compare(heap->data[left - 1],
						heap->data[i - 1]) < 0)
	  extreme = left;
	else
	  extreme = i;

	if (right <= heap->size && heap->compare(heap->data[right - 1],
						 heap->data[extreme - 1]) < 0)
	  extreme = right;
     }
   else
     {
	if (left <= heap->size && heap->compare(heap->data[left - 1],
						heap->data[i - 1]) > 0)
	  extreme = left;
	else
	  extreme = i;

	if (right <= heap->size && heap->compare(heap->data[right - 1],
						 heap->data[extreme - 1]) > 0)
	  extreme = right;
     }

   /*
    * If the data needs to be swapped down the heap, recurse on
    * heapifying it's new placement.
    */
   if (extreme != i)
     {
	void *temp;

	temp = heap->data[extreme - 1];
	heap->data[extreme - 1] = heap->data[i - 1];
	heap->data[i - 1] = temp;

	_ecore_sheap_heapify(heap, extreme);
     }
}

static void 
_ecore_sheap_update_data(Ecore_Sheap *heap)
{
   int i, old_size;
   void **data;

   /*
    * Track the old values from the heap
    */
   old_size = heap->size;
   data = heap->data;

   heap->size = 0;
   heap->data = malloc(heap->space * sizeof(void *));

   for (i = 0; i < old_size; i++)
     ecore_sheap_insert(heap, data[i]);

   FREE(data);
}
