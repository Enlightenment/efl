#include "ecore_private.h"
#include "Ecore.h"

/* Return information about the list */
static void *_ecore_list_current(Ecore_List * list);

/* Adding functions */
static int _ecore_list_insert(Ecore_List * list, Ecore_List_Node *node);
static int _ecore_list_append_0(Ecore_List * list, Ecore_List_Node *node);
static int _ecore_list_prepend_0(Ecore_List * list, Ecore_List_Node *node);

/* Remove functions */
static void *_ecore_list_remove_0(Ecore_List * list);
static void *_ecore_list_remove_first(Ecore_List * list);
static void *_ecore_list_remove_last(Ecore_List * list);

/* Basic traversal functions */
static void *_ecore_list_next(Ecore_List * list);
static void *_ecore_list_goto_last(Ecore_List * list);
static void *_ecore_list_goto_first(Ecore_List * list);
static void *_ecore_list_goto(Ecore_List * list, void *data);
static void *_ecore_list_goto_index(Ecore_List *list, int index);

/* Iterative function */
static int _ecore_list_for_each(Ecore_List *list, Ecore_For_Each function);

/* Private double linked list functions */
static void *_ecore_dlist_previous(Ecore_DList * list);
static void *_ecore_dlist_remove_first(Ecore_DList *list);
static void *_ecore_dlist_goto_index(Ecore_DList *list, int index);

/* XXX: Begin deprecated code */
void *
_ecore_list_append(void *in_list, void *in_item)
{
   Ecore_Oldlist *l, *new_l;
   Ecore_Oldlist *list, *item;
   
   list = in_list;
   item = in_item;
   new_l = item;
   new_l->next = NULL;
   if (!list) 
     {
	new_l->prev = NULL;
	new_l->last = new_l;
	return new_l;
     }
   if (list->last) l = list->last;
   else for (l = list; l; l = l->next);
   l->next = new_l;
   new_l->prev = l;
   list->last = new_l;
   return list;
}

void *
_ecore_list_prepend(void *in_list, void *in_item)
{
   Ecore_Oldlist *new_l;
   Ecore_Oldlist *list, *item;
   
   list = in_list;
   item = in_item;   
   new_l = item;
   new_l->prev = NULL;
   if (!list) 
     {
	new_l->next = NULL;
	new_l->last = new_l;	
	return new_l;
     }
   new_l->next = list;
   list->prev = new_l;
   new_l->last = list->last;
   list->last = NULL;
   return new_l;
}

void *
_ecore_list_append_relative(void *in_list, void *in_item, void *in_relative)
{
   Ecore_Oldlist *l;
   Ecore_Oldlist *list, *item, *relative;
   
   list = in_list;
   item = in_item;
   relative = in_relative;
   for (l = list; l; l = l->next)
     {
	if (l == relative)
	  {
	     Ecore_Oldlist *new_l;
	     
	     new_l = item;
	     if (l->next)
	       {
		  new_l->next = l->next;
		  l->next->prev = new_l;
	       }

	     else new_l->next = NULL;
	     l->next = new_l;
	     new_l->prev = l;
	     if (!new_l->next)
	       list->last = new_l;
	     return list;
	  }
     }
   return _ecore_list_append(list, item);
}

void *
_ecore_list_prepend_relative(void *in_list, void *in_item, void *in_relative)
{
   Ecore_Oldlist *l;
   Ecore_Oldlist *list, *item, *relative;
   
   list = in_list;
   item = in_item;
   relative = in_relative;
   for (l = list; l; l = l->next)
     {
	if (l == relative)
	  {
	     Ecore_Oldlist *new_l;
	     
	     new_l = item;
	     new_l->prev = l->prev;
	     new_l->next = l;
	     l->prev = new_l;
	     if (new_l->prev)
	       {
		  new_l->prev->next = new_l;
		  if (!new_l->next)
		    list->last = new_l;
		  return list;
	       }
	     else
	       {
		  if (!new_l->next)
		    new_l->last = new_l;
		  else
		    {
		       new_l->last = list->last;
		       list->last = NULL;
		    }
		  return new_l;
	       }
	  }
     }
   return _ecore_list_prepend(list, item);
}

void *
_ecore_list_remove(void *in_list, void *in_item)
{
   Ecore_Oldlist *return_l;
   Ecore_Oldlist *list, *item;

   /* checkme */
   if(!in_list)
     return in_list;

   list = in_list;
   item = in_item;
   if (!item) return list;
   if (item->next)
     item->next->prev = item->prev;
   if (item->prev)
     {
	item->prev->next = item->next;
	return_l = list;
     }
   else
     {
	return_l = item->next;
	if (return_l)
	  return_l->last = list->last;
     }
   if (item == list->last)
     list->last = item->prev;
   item->next = NULL;
   item->prev = NULL;
   return return_l;
}

void *
_ecore_list_find(void *in_list, void *in_item)
{
   Ecore_Oldlist *l;
   Ecore_Oldlist *list, *item;
   
   list = in_list;
   item = in_item;   
   for (l = list; l; l = l->next)
     {
	if (l == item) return item;
     }
   return NULL;
}
/* XXX: End deprecated code */

/**
 * @brief Create and initialize a new list.
 * @return Returns a new initialized list on success, NULL on failure.
 */
Ecore_List *ecore_list_new()
{
	Ecore_List *list;

	list = (Ecore_List *)malloc(sizeof(Ecore_List));
	if (!list)
		return NULL;

	if (!ecore_list_init(list)) {
		FREE(list);
		return NULL;
	}

	return list;
}

/**
 * @brief Initialize a list to some sane starting values.
 * @param list: the list to initialize
 * @return Returns FALSE if an error occurs, TRUE if successful
 */
int ecore_list_init(Ecore_List *list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	memset(list, 0, sizeof(Ecore_List));

	ECORE_INIT_LOCKS(list);

	return TRUE;
}

/**
 * @brief Free a list and all of it's nodes.
 * @param list: the list to be freed
 * @return Returns no value
 */
void ecore_list_destroy(Ecore_List * list)
{
	void *data;

	CHECK_PARAM_POINTER("list", list);

	ECORE_WRITE_LOCK(list);

	while (list->first) {
		data = _ecore_list_remove_first(list);
		if (list->free_func)
			list->free_func(data);
	}

	ECORE_WRITE_UNLOCK(list);
	ECORE_DESTROY_LOCKS(list);

	FREE(list);
}

/**
 * @brief Set the function for freeing data
 * @param list: the list that will use this function when nodes are destroyed.
 * @param free_func: the function that will free the key data
 * @return Returns TRUE on successful set, FALSE otherwise.
 */
int ecore_list_set_free_cb(Ecore_List * list, Ecore_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	list->free_func = free_func;

	ECORE_WRITE_UNLOCK(list);

	return TRUE;
}

/**
 * @brief Checks the list for any nodes.
 * @param list: the list to check for nodes
 * @return Returns TRUE if no nodes in list, FALSE if the list contains nodes
 */
int ecore_list_is_empty(Ecore_List * list)
{
	int ret = TRUE;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_READ_LOCK(list);

	if (list->nodes)
		ret = FALSE;

	ECORE_READ_UNLOCK(list);

	return ret;
}

/**
 * @brief Returns the number of the current node
 * @param list: the list to return the number of the current node
 * @return Returns the number of the current node in the list.
 */
int ecore_list_index(Ecore_List * list)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_READ_LOCK(list);

	ret = list->index;

	ECORE_READ_UNLOCK(list);

	return ret;
}

/**
 * @brief Find the number of nodes in the list.
 * @param list: the list to find the number of nodes
 * @return Returns the number of nodes in the list.
 */
int ecore_list_nodes(Ecore_List * list)
{
	int ret = 0;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_READ_LOCK(list);

	ret = list->nodes;

	ECORE_READ_UNLOCK(list);

	return ret;
}

/**
 * @brief Append data to the list.
 * @param list: the list to append @data
 * @param data: the data to append to @list.
 * @return Returns FALSE if an error occurs, TRUE if appended successfully
 */
inline int ecore_list_append(Ecore_List * list, void *data)
{
	int ret;
	Ecore_List_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	node = ecore_list_node_new();
	node->data = data;

	ECORE_WRITE_LOCK(list);

	ret = _ecore_list_append_0(list, node);

	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* For adding items to the end of the list */
static int _ecore_list_append_0(Ecore_List * list, Ecore_List_Node *end)
{
	if (list->last) {
		ECORE_WRITE_LOCK(list->last);
		list->last->next = end;
		ECORE_WRITE_UNLOCK(list->last);
	}

	list->last = end;

	if (list->first == NULL) {
		list->first = end;
		list->index = 1;
	}

	list->nodes++;

	return TRUE;
}

/**
 * @brief Prepend data to the beginning of the list
 * @param list: the list to prepend @data
 * @param data: the data to prepend to @list
 * @return Returns FALSE if an error occurs, TRUE if prepended successfully
 */
inline int ecore_list_prepend(Ecore_List * list, void *data)
{
	int ret;
	Ecore_List_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	node = ecore_list_node_new();
	node->data = data;

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_prepend_0(list, node);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* For adding items to the beginning of the list */
static int _ecore_list_prepend_0(Ecore_List * list, Ecore_List_Node *start)
{
	/* Put it at the beginning of the list */
	ECORE_WRITE_LOCK(start);
	start->next = list->first;
	ECORE_WRITE_UNLOCK(start);

	list->first = start;

	/* If no last node, then the first node is the last node */
	if (list->last == NULL)
		list->last = list->first;

	list->nodes++;
	list->index++;

	return TRUE;
}

/**
 * @brief Insert data at the current point in the list
 * @param list: the list to hold the inserted @data
 * @param data: the data to insert into @list
 * @return Returns FALSE on an error, TRUE on success
 */
inline int ecore_list_insert(Ecore_List * list, void *data)
{
	int ret;
	Ecore_List_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	node = ecore_list_node_new();
	node->data = data;

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_insert(list, node);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* For adding items in front of the current position in the list */
static int _ecore_list_insert(Ecore_List * list, Ecore_List_Node *new_node)
{
	/*
	 * If the current point is at the beginning of the list, then it's the
	 * same as prepending it to the list.
	 */
	if (list->current == list->first)
		return _ecore_list_prepend_0(list, new_node);

	if (list->current == NULL) {
		int ret_value;

		ret_value = _ecore_list_append_0(list, new_node);
		list->current = list->last;

		return ret_value;
	}

	/* Setup the fields of the new node */
	ECORE_WRITE_LOCK(new_node);
	new_node->next = list->current;
	ECORE_WRITE_UNLOCK(new_node);

	/* And hook the node into the list */
	_ecore_list_goto_index(list, ecore_list_index(list) - 1);

	ECORE_WRITE_LOCK(list->current);
	list->current->next = new_node;
	ECORE_WRITE_UNLOCK(list->current);

	/* Now move the current item to the inserted item */
	list->current = new_node;
	list->index++;
	list->nodes++;

	return TRUE;
}

/**
 * @brief Remove the current item from the list.
 * @param list: the list to remove the current item
 * @return Returns a pointer to the removed data on success, NULL on failure.
 */
inline void *ecore_list_remove(Ecore_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_remove_0(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* Remove the current item from the list */
static void *_ecore_list_remove_0(Ecore_List * list)
{
	void *ret = NULL;
	Ecore_List_Node *old;

	if (!list)
		return FALSE;

	if (ecore_list_is_empty(list))
		return FALSE;

	if (!list->current)
		return FALSE;

	if (list->current == list->first)
		return _ecore_list_remove_first(list);

	if (list->current == list->last)
		return _ecore_list_remove_last(list);

	old = list->current;

	_ecore_list_goto_index(list, list->index - 1);

	ECORE_WRITE_LOCK(list->current);
	ECORE_WRITE_LOCK(old);

	list->current->next = old->next;
	old->next = NULL;
	ret = old->data;
	old->data = NULL;

	_ecore_list_next(list);

	ECORE_WRITE_UNLOCK(old);
	ECORE_WRITE_UNLOCK(list->current);

	ecore_list_node_destroy(old, NULL);
	list->nodes--;

	return ret;
}

/**
 * @brief Remove and free the data in lists current position
 * @param list: the list to remove and free the current item
 * @return Returns TRUE on success, FALSE on error
 */
int ecore_list_remove_destroy(Ecore_List *list)
{
	void *data;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	data = _ecore_list_remove_0(list);
	if (list->free_func)
		list->free_func(data);

	ECORE_WRITE_UNLOCK(list);

	return TRUE;
}

/**
 * @brief Remove the first item from the list.
 * @param list: the list to remove the current item
 * @return Returns a pointer to the removed data on success, NULL on failure.
 */
inline void *ecore_list_remove_first(Ecore_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_remove_first(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* Remove the first item from the list */
static void *_ecore_list_remove_first(Ecore_List * list)
{
	void *ret = NULL;
	Ecore_List_Node *old;

	if (!list)
		return FALSE;

	ECORE_WRITE_UNLOCK(list);
	if (ecore_list_is_empty(list))
		return FALSE;
	ECORE_WRITE_LOCK(list);

	if (!list->first)
		return FALSE;

	old = list->first;

	list->first = list->first->next;

	if (list->current == old)
		list->current = list->first;
	else
		list->index--;

	if (list->last == old)
		list->last = list->first;

	ECORE_WRITE_LOCK(old);
	ret = old->data;
	old->data = NULL;
	ECORE_WRITE_UNLOCK(old);

	ecore_list_node_destroy(old, NULL);
	list->nodes--;

	return ret;
}

/**
 * @brief Remove the last item from the list.
 * @param list: the list to remove the last node from
 * @return Returns a pointer to the removed data on success, NULL on failure.
 */
inline void *ecore_list_remove_last(Ecore_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_remove_last(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* Remove the last item from the list */
static void *_ecore_list_remove_last(Ecore_List * list)
{
	void *ret = NULL;
	Ecore_List_Node *old, *prev;

	if (!list)
		return FALSE;

	if (ecore_list_is_empty(list))
		return FALSE;

	if (!list->last)
		return FALSE;

	old = list->last;
	if (list->current == old)
		list->current = NULL;

	if (list->first == old)
		list->first = NULL;
	for (prev = list->first; prev && prev->next != old; prev = prev->next);
	if (prev) {
		prev->next = NULL;
		list->last = prev;
		if (list->current == old) {
			list->current = NULL;
		}
	}


	ECORE_WRITE_LOCK(old);
	if (old) {
		old->next = NULL;
		ret = old->data;
		old->data = NULL;
	}
	ECORE_WRITE_UNLOCK(old);

	ecore_list_node_destroy(old, NULL);
	list->nodes--;

	return ret;
}

/**
 * @brief Move the current item to the index number
 * @param list: the list to move the current item
 * @param index: the position to move the current item
 * @return Returns a pointer to new current item on success, NULL on failure.
 */
inline void *ecore_list_goto_index(Ecore_List * list, int index)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_goto_index(list, index);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* This is the non-threadsafe version, use this inside internal functions that
 * already lock the list */
static void *_ecore_list_goto_index(Ecore_List *list, int index)
{
	int i;

	if (!list)
		return FALSE;

	if (ecore_list_is_empty(list))
		return FALSE;

	if (index > ecore_list_nodes(list) || index < 0)
		return FALSE;

	_ecore_list_goto_first(list);

	for (i = 1; i < index && _ecore_list_next(list); i++);

	list->index = i;

	return list->current->data;
}

/**
 * @brief Move the current item to the node that contains data
 * @param list: the list to move the current item in
 * @param data: the data to find and set the current item to
 *
 * @return Returns a pointer to @a data on success, NULL on failure.
 */
inline void *ecore_list_goto(Ecore_List * list, void *data)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_goto(list, data);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* Set the current position to the node containing data */
static void *_ecore_list_goto(Ecore_List * list, void *data)
{
	int index;
	Ecore_List_Node *node;

	if (!list)
		return NULL;

	index = 1;

	node = list->first;
	ECORE_READ_LOCK(node);
	while (node && node->data) {
		Ecore_List_Node *next;

		if (node->data == data)
			break;

		next = node->next;
		ECORE_READ_UNLOCK(node);

		node = next;

		ECORE_READ_LOCK(node);
		index++;
	}

	ECORE_READ_UNLOCK(node);
	if (!node)
		return NULL;

	list->current = node;
	list->index = index;

	return list->current->data;
}

/**
 * @brief Move the current pointer to the first item in the list
 * @param list: the list to move the current pointer in
 * @return Returns a pointer to the first item on success, NULL on failure
 */
inline void *ecore_list_goto_first(Ecore_List *list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	ret = _ecore_list_goto_first(list);

	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* Set the current position to the start of the list */
static void *_ecore_list_goto_first(Ecore_List * list)
{
	if (!list || !list->first)
		return NULL;

	list->current = list->first;
	list->index = 1;

	return list->current->data;
}

/**
 * @brief Move the pointer to current to the last item in the list
 * @param list: the list to move the current pointer in
 * @return Returns a pointer to the last item on success, NULL on failure.
 */
inline void *ecore_list_goto_last(Ecore_List * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_goto_last(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* Set the current position to the end of the list */
static void *_ecore_list_goto_last(Ecore_List * list)
{
	if (!list || !list->last)
		return NULL;

	list->current = list->last;
	list->index = list->nodes;

	return list->current->data;
}

/**
 * @brief Retrieve the data in the current node
 * @param list: the list to retrieve the current data from
 * @return Returns the data at current position, can be NULL.
 */
inline void *ecore_list_current(Ecore_List * list)
{
	void *ret;

	ECORE_READ_LOCK(list);
	ret = _ecore_list_current(list);
	ECORE_READ_UNLOCK(list);

	return ret;
}

/* Return the data of the current node without incrementing */
static void *_ecore_list_current(Ecore_List * list)
{
	void *ret;

	if (!list->current)
		return NULL;

	ECORE_READ_LOCK(list->current);
	ret = list->current->data;
	ECORE_READ_UNLOCK(list->current);

	return ret;
}

/**
 * @brief Retrieve the data at the current node and move to the next
 * @param list: the list to move to the next item
 *
 * @return Returns the current item in the list on success, NULL on failure.
 */
inline void *ecore_list_next(Ecore_List * list)
{
	void *data;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	data = _ecore_list_next(list);
	ECORE_WRITE_UNLOCK(list);

	return data;
}

/* Return the data contained in the current node and go to the next node */
static void *_ecore_list_next(Ecore_List * list)
{
	void *data;
	Ecore_List_Node *ret;
	Ecore_List_Node *next;

	if (!list->current)
		return NULL;

	ECORE_READ_LOCK(list->current);
	ret = list->current;
	next = list->current->next;
	ECORE_READ_UNLOCK(list->current);

	list->current = next;
	list->index++;

	ECORE_READ_LOCK(ret);
	data = ret->data;
	ECORE_READ_UNLOCK(ret);

	return data;
}

/**
 * @brief Remove all nodes from the list
 * @param list: the list that will have it's nodes removed
 * @return Returns TRUE on success, FALSE on error.
 *
 * The data for each item on the list is not freed by ecore_list_clear.
 */
int ecore_list_clear(Ecore_List * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	while (!ecore_list_is_empty(list))
		_ecore_list_remove_first(list);

	ECORE_WRITE_UNLOCK(list);

	return TRUE;
}

/**
 * @brief Execute function for each node in the list.
 * @param list: the list to retrieve nodes from.
 * @param function: The function to pass each node from the list to.
 *
 * @return Returns TRUE on success, FALSE on failure.
 */
int ecore_list_for_each(Ecore_List *list, Ecore_For_Each function)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_READ_LOCK(list);
	ret = _ecore_list_for_each(list, function);
	ECORE_READ_UNLOCK(list);

	return ret;
}

/* The real meat of executing the function for each data node */
static int _ecore_list_for_each(Ecore_List *list, Ecore_For_Each function)
{
	void *value;

	if (!list || !function)
		return FALSE;

	_ecore_list_goto_first(list);
	while ((value = _ecore_list_next(list)) != NULL)
		function(value);

	return TRUE;
}

/* Initialize a node to starting values */
int ecore_list_node_init(Ecore_List_Node * node)
{

	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	node->next = NULL;
	node->data = NULL;

	ECORE_INIT_LOCKS(node);

	return TRUE;
}

/* Allocate and initialize a new list node */
Ecore_List_Node *ecore_list_node_new()
{
	Ecore_List_Node *new_node;

	new_node = malloc(sizeof(Ecore_List_Node));

	if (!ecore_list_node_init(new_node)) {
		FREE(new_node);
		return NULL;
	}

	return new_node;
}

/* Here we actually call the function to free the data and free the node */
int ecore_list_node_destroy(Ecore_List_Node * node, Ecore_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	ECORE_WRITE_LOCK(node);

	if (free_func && node->data)
		free_func(node->data);

	ECORE_WRITE_UNLOCK(node);
	ECORE_DESTROY_LOCKS(node);

	FREE(node);

	return TRUE;
}

/**
 * @brief Create and initialize a new list.
 * @return Returns a new initialized list on success, NULL on failure.
 */
Ecore_DList *ecore_dlist_new()
{
	Ecore_DList *list = NULL;

	list = (Ecore_DList *)malloc(sizeof(Ecore_DList));
	if (!list)
		return NULL;

	if (!ecore_dlist_init(list)) {
		IF_FREE(list);
		return NULL;
	}

	return list;
}

/**
 * @brief Initialize a list to some sane starting values.
 * @param list: the list to initialize
 * @return Returns FALSE if an error occurs, TRUE if successful
 */
int ecore_dlist_init(Ecore_DList *list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	memset(list, 0, sizeof(Ecore_DList));

	ECORE_INIT_LOCKS(list);

	return TRUE;
}

/**
 * @brief Free a list and all of it's nodes.
 * @param list: the list to be freed
 *
 * @return Returns no value
 */
void ecore_dlist_destroy(Ecore_DList * list)
{
	void *data;
	CHECK_PARAM_POINTER("list", list);

	ECORE_WRITE_LOCK(list);

	while (list->first) {
		data = _ecore_dlist_remove_first(list);
		if (list->free_func)
			list->free_func(data);
	}

	ECORE_WRITE_UNLOCK(list);
	ECORE_DESTROY_LOCKS(list);

	FREE(list);
}

/**
 * @brief Set the function for freeing data
 * @param list: the list that will use this function when nodes are destroyed.
 * @param free_func: the function that will free the key data
 * @return Returns TRUE on success, FALSE on failure.
 */
int ecore_dlist_set_free_cb(Ecore_DList * list, Ecore_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	return ecore_list_set_free_cb(ECORE_LIST(list), free_func);
}

/**
 * @brief Checks the list for any nodes.
 * @param list: the list to check for nodes
 *
 * @return Returns TRUE if no nodes in list, FALSE if the list contains nodes
 */
int ecore_dlist_is_empty(Ecore_DList * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	return ecore_list_is_empty(ECORE_LIST(list));
}

/**
 * @brief Returns the number of the current node
 * @param list: the list to return the number of the current node
 * @return Returns the number of the current node in the list.
 */
inline int ecore_dlist_index(Ecore_DList * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	return ecore_list_index(ECORE_LIST(list));
}

/**
 * @brief Append data to the list.
 * @param list: the list to append @data
 * @param data: the data to append to @list
 *
 * @return Returns FALSE if an error occurs, TRUE if appended successfully
 */
int ecore_dlist_append(Ecore_DList * list, void *data)
{
	int ret;
	Ecore_DList_Node *prev;
	Ecore_DList_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	node = ecore_dlist_node_new();
	ECORE_LIST_NODE(node)->data = data;

	prev = ECORE_DLIST_NODE(ECORE_LIST(list)->last);
	ret = _ecore_list_append_0(ECORE_LIST(list), ECORE_LIST_NODE(node));
	if (ret) {
		node->previous = prev;
	}

	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @brief Prepend data to the beginning of the list
 * @param list: the list to prepend @data
 * @param data: the data to prepend to @list
 * @return Returns FALSE if an error occurs, TRUE if prepended successfully
 */
int ecore_dlist_prepend(Ecore_DList * list, void *data)
{
	int ret;
	Ecore_DList_Node *prev;
	Ecore_DList_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	node = ecore_dlist_node_new();
	ECORE_LIST_NODE(node)->data = data;

	prev = ECORE_DLIST_NODE(ECORE_LIST(list)->first);
	ret = _ecore_list_prepend_0(ECORE_LIST(list), ECORE_LIST_NODE(node));
	if (ret && prev)
		prev->previous = node;

	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @brief Insert data at the current point in the list
 * @param list: the list to hold the inserted @data
 * @param data: the data to insert into @list
 * @return Returns FALSE on an error, TRUE on success
 */
int ecore_dlist_insert(Ecore_DList * list, void *data)
{
	int ret;
	Ecore_DList_Node *prev;
	Ecore_DList_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	prev = ECORE_DLIST_NODE(ECORE_LIST(list)->current);
	if (!prev)
		prev = ECORE_DLIST_NODE(ECORE_LIST(list)->last);

	if (prev)
		prev = prev->previous;

	node = ecore_dlist_node_new();
	ECORE_LIST_NODE(node)->data = data;

	ret = _ecore_list_insert(list, ECORE_LIST_NODE(node));
	if (!ret) {
		ECORE_WRITE_UNLOCK(list);
		return ret;
	}

	if (ECORE_LIST_NODE(node)->next)
		ECORE_DLIST_NODE(ECORE_LIST_NODE(node)->next)->previous = node;

	if (prev)
		node->previous = prev;

	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @brief Remove the current item from the list.
 * @param list: the list to remove the current item
 * @return Returns a pointer to the removed data on success, NULL on failure.
 */
void *ecore_dlist_remove(Ecore_DList * list)
{
	void *ret;
	Ecore_List_Node *node;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);

	if (list->current) {
		node = list->current->next;
		ECORE_DLIST_NODE(node)->previous =
			ECORE_DLIST_NODE(ECORE_LIST(list)->current)->previous;
	}
	ret = _ecore_list_remove_0(list);

	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @brief Remove the first item from the list.
 * @param list: the list to remove the current item
 * @return Returns a pointer to the removed data on success, NULL on failure.
 */
void *ecore_dlist_remove_first(Ecore_DList * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_dlist_remove_first(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @brief Remove and free the data at the current position
 * @param list: the list to remove the data from
 *
 * @return Returns TRUE on success, FALSE on error
 */
int ecore_dlist_remove_destroy(Ecore_DList *list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	return ecore_list_remove_destroy(list);
}

static void *_ecore_dlist_remove_first(Ecore_DList *list)
{
	void *ret;

	if (!list)
		return FALSE;

	ret = _ecore_list_remove_first(list);
	if (ret && ECORE_LIST(list)->first)
		ECORE_DLIST_NODE(ECORE_LIST(list)->first)->previous = NULL;

	return ret;
}

/**
 * @brief Remove the last item from the list
 * @param list: the list to remove the last node from
 * @return Returns a pointer to the removed data on success, NULL on failure.
 */
void *ecore_dlist_remove_last(Ecore_DList * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_remove_last(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @brief Move the current item to the index number
 * @param list: the list to move the current item
 * @param index: the position to move the current item
 * @return Returns node at specified index on success, NULL on error
 */
void *ecore_dlist_goto_index(Ecore_DList * list, int index)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_dlist_goto_index(list, index);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/* This is the non-threadsafe version, use this inside internal functions that
 * already lock the list */
static void *_ecore_dlist_goto_index(Ecore_DList *list, int index)
{
	int i, increment;

	if (!list)
		return FALSE;

	if (ecore_list_is_empty(ECORE_LIST(list)))
		return FALSE;

	if (index > ecore_list_nodes(ECORE_LIST(list)) || index < 1)
		return FALSE;

	if (ECORE_LIST(list)->index > ECORE_LIST(list)->nodes)
		_ecore_list_goto_last(ECORE_LIST(list));

	if (index < ECORE_LIST(list)->index)
		increment = -1;
	else
		increment = 1;

	for (i = ECORE_LIST(list)->index; i != index; i += increment) {
		if (increment > 0)
			_ecore_list_next(list);
		else
			_ecore_dlist_previous(list);
	}

	return _ecore_list_current(list);
}

/**
 * @brief Move the current item to the node that contains data
 * @param list: the list to move the current item in
 * @param data: the data to find and set the current item to
 *
 * @return Returns specified data on success, NULL on error
 */
void *ecore_dlist_goto(Ecore_DList * list, void *data)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_goto(ECORE_LIST(list), data);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @brief Move the current pointer to the first item in the list
 * @param list: the list to change the current to the first item
 *
 * @return Returns a pointer to the first item on success, NULL on failure.
 */
void *ecore_dlist_goto_first(Ecore_DList *list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_goto_first(list);
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @brief Move the pointer to the current item to the last item
 * @param list: the list to move the current item pointer to the last
 * @return Returns a pointer to the last item in the list , NULL if empty.
 */
void *ecore_dlist_goto_last(Ecore_DList * list)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ECORE_WRITE_LOCK(list);
	ret = _ecore_list_goto_last(ECORE_LIST(list));
	ECORE_WRITE_UNLOCK(list);

	return ret;
}

/**
 * @brief Return the data in the current list item
 * @param list: the list to the return the current data
 * @return Returns value of the current data item, NULL if no current item
 */
void *ecore_dlist_current(Ecore_DList * list)
{
	void *ret;

	ECORE_READ_LOCK(list);
	ret = _ecore_list_current(ECORE_LIST(list));
	ECORE_READ_UNLOCK(list);

	return ret;
}

/**
 * @brief Move to the next item in the list and return current item
 * @param list: the list to move to the next item in.
 * @return Returns data in the current list node, or NULL on error
 */
void *ecore_dlist_next(Ecore_DList * list)
{
	void *data;

	ECORE_WRITE_LOCK(list);
	data = _ecore_list_next(list);
	ECORE_WRITE_UNLOCK(list);

	return data;
}

/**
 * @brief Move to the previous item and return current item
 * @param list: the list to move to the previous item in.
 * @return Returns data in the current list node, or NULL on error
 */
void *ecore_dlist_previous(Ecore_DList * list)
{
	void *data;

	ECORE_WRITE_LOCK(list);
	data = _ecore_dlist_previous(list);
	ECORE_WRITE_UNLOCK(list);

	return data;
}

static void *_ecore_dlist_previous(Ecore_DList * list)
{
	void *data = NULL;

	if (!list)
		return NULL;

	if (ECORE_LIST(list)->current) {
		data = ECORE_LIST(list)->current->data;
		ECORE_LIST(list)->current = ECORE_LIST_NODE(ECORE_DLIST_NODE(
				ECORE_LIST(list)->current)->previous);
		ECORE_LIST(list)->index--;
	}
	else
		_ecore_list_goto_last(ECORE_LIST(list));

	return data;
}

/**
 * @brief Remove all nodes from the list.
 * @param list: the list to remove all nodes from
 *
 * @return Returns TRUE on success, FALSE on errors
 */
int ecore_dlist_clear(Ecore_DList * list)
{
	CHECK_PARAM_POINTER_RETURN("list", list, FALSE);

	ecore_list_clear(ECORE_LIST(list));

	return TRUE;
}

/*
 * @brief Initialize a node to sane starting values
 * @param node: the node to initialize
 * @return Returns TRUE on success, FALSE on errors
 */
int ecore_dlist_node_init(Ecore_DList_Node * node)
{
	int ret;

	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	ret = ecore_list_node_init(ECORE_LIST_NODE(node));
	if (ret)
		node->previous = NULL;

	return ret;
}

/*
 * @brief Allocate and initialize a new list node
 * @return Returns NULL on error, new list node on success
 */
Ecore_DList_Node *ecore_dlist_node_new()
{
	Ecore_DList_Node *new_node;

	new_node = malloc(sizeof(Ecore_DList_Node));

	if (!new_node)
		return NULL;

	if (!ecore_dlist_node_init(new_node)) {
		FREE(new_node);
		return NULL;
	}

	return new_node;
}

/*
 * @brief Call the data's free callback function, then free the node
 * @param node: the node to be freed
 * @param free_func: the callback function to execute on the data
 * @return Returns TRUE on success, FALSE on error
 */
int ecore_dlist_node_destroy(Ecore_DList_Node * node, Ecore_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("node", node,
			FALSE);

	return ecore_list_node_destroy(ECORE_LIST_NODE(node), free_func);
}
