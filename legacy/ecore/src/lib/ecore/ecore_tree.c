/* ecore_tree.c

Copyright (C) 2001 Nathan Ingersoll         <ningerso@d.umn.edu>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies of the Software and its documentation and acknowledgment shall be
given in the documentation and software packages that this Software was
used.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <Ecore.h>

/* A macro for determining the highest node at given branch */
#define MAX_HEIGHT(node) (node ? MAX(node->max_left, node->max_right) : 0)

/* Utility functions for searching the tree and returning a node, or its
 * parent */
Ecore_Tree_Node *tree_node_find(Ecore_Tree * tree, void *key);
Ecore_Tree_Node *tree_node_find_parent(Ecore_Tree * tree, void *key);

/* Balancing functions, keep the tree balanced within a one node height
 * difference */
int tree_node_balance(Ecore_Tree * Tree, Ecore_Tree_Node * top_node);
int tree_node_rotate_right(Ecore_Tree * tree, Ecore_Tree_Node * top_node);
int tree_node_rotate_left(Ecore_Tree * tree, Ecore_Tree_Node * top_node);

/* Fucntions for executing a specified function on each node of a tree */
int tree_for_each_node(Ecore_Tree_Node * node, Ecore_For_Each for_each_func);
int tree_for_each_node_value(Ecore_Tree_Node * node,
			     Ecore_For_Each for_each_func);

/**
 * ecore_tree_new - allocate a new tree structure.
 * @compare_func: function used to compare the two values
 *
 * Returns NULL if the operation fails, otherwise a pointer to the new tree
 */
Ecore_Tree *ecore_tree_new(Ecore_Compare_Cb compare_func)
{
	Ecore_Tree *new_tree;

	new_tree = ECORE_TREE(malloc(sizeof(Ecore_Tree)));
	if (!new_tree)
		return NULL;

	if (!ecore_tree_init(new_tree, compare_func)) {
		IF_FREE(new_tree);
		return NULL;
	}

	return new_tree;
}

/**
 * ecore_tree_init - initialize a tree structure to some sane initial values
 * @new_tree: the new tree structure to be initialized
 * @compare_func: the function used to compare node keys
 *
 * Returns TRUE on successful initialization, FALSE on an error
 */
int ecore_tree_init(Ecore_Tree * new_tree, Ecore_Compare_Cb compare_func)
{
	CHECK_PARAM_POINTER_RETURN("new_tree", new_tree, FALSE);

	memset(new_tree, 0, sizeof(Ecore_Tree));

	if (!compare_func)
		new_tree->compare_func = ecore_direct_compare;
	else
		new_tree->compare_func = compare_func;

	ECORE_INIT_LOCKS(new_tree);

	return TRUE;
}

/*
 * ecore_tree_set_free_cb - add a function to be called at node destroy time
 * @tree: the tree that will use this function when nodes are destroyed
 * @free_func - the function that will be passed the node being freed
 *
 * Returns TRUE on successful set, FALSE otherwise.
 */
int ecore_tree_set_free_cb(Ecore_Tree * tree, Ecore_Free_Cb free_func)
{
	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

	ECORE_WRITE_LOCK(tree);
	tree->free_func = free_func;
	ECORE_WRITE_UNLOCK(tree);

	return TRUE;
}

/*
 * ecore_tree_node_init - initialize a new tree node
 *
 * Returns FALSE if the operation fails, otherwise TRUE
 */
int ecore_tree_node_init(Ecore_Tree_Node * new_node)
{
	CHECK_PARAM_POINTER_RETURN("new_node", new_node, FALSE);

	new_node->key = NULL;
	new_node->value = NULL;

	new_node->parent = NULL;
	new_node->right_child = NULL;
	new_node->left_child = NULL;

	new_node->max_left = new_node->max_right = 0;

	ECORE_INIT_LOCKS(new_node);

	return TRUE;
}

/*
 * Description: Allocate a new tree node
 * Parameters: None.
 * Returns: NULL if the operation fails, otherwise a pointer to the new node.
 */
Ecore_Tree_Node *ecore_tree_node_new()
{
	Ecore_Tree_Node *new_node;

	new_node = ECORE_TREE_NODE(malloc(sizeof(Ecore_Tree_Node)));
	if (!new_node)
		return NULL;

	if (!ecore_tree_node_init(new_node)) {
		IF_FREE(new_node);
		return NULL;
	}

	return new_node;
}

/*
 * Description: Free a tree node and it's children. If you don't want the
 * 		children free'd then you need to remove the node first.
 * Parameters: 1. node - tree node to be free()'d
 * 	       2. data_free - callback for destroying the data held in node
 * Returns: TRUE if the node is destroyed successfully, FALSE if not.
 */
int ecore_tree_node_destroy(Ecore_Tree_Node * node, Ecore_Free_Cb data_free)
{
	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	ECORE_WRITE_LOCK(node);
	if (data_free)
		data_free(node->value);
	ECORE_WRITE_UNLOCK(node);

	ECORE_DESTROY_LOCKS(node);

	FREE(node);

	return TRUE;
}

/*
 * ecore_tree_node_value_set - set the value of the node to value
 * @node: the node to be set
 * @value: the value to set the node to.
 *
 * Returns TRUE if the node is set successfully, FALSE if not.
 */
int ecore_tree_node_value_set(Ecore_Tree_Node * node, void *value)
{
	CHECK_PARAM_POINTER_RETURN("node", node,
				   FALSE);

	ECORE_WRITE_LOCK(node);
	node->value = value;
	ECORE_WRITE_UNLOCK(node);

	return TRUE;
}

/*
 * Description: Get the value of the node
 * Parameters: 1. node - the node that contains the desired value
 * Returns: NULL if an error, otherwise the value associated with node
 */
void *ecore_tree_node_value_get(Ecore_Tree_Node * node)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);
	ECORE_READ_LOCK(node);
	ret = node->value;
	ECORE_READ_UNLOCK(node);

	return ret;
}

/*
 * ecore_tree_node_key_set - set the value of the node's key  to key
 * @node: the node to be set
 * @key: the value to set it's key to.
 *
 * Returns TRUE if the node is set successfully, FALSE if not.
 */
int ecore_tree_node_key_set(Ecore_Tree_Node * node, void *key)
{
	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	ECORE_WRITE_LOCK(node);
	node->key = key;
	ECORE_WRITE_UNLOCK(node);

	return TRUE;
}

/*
 * ecore_tree_node_key_get - get the value of the node's key 
 * @node: the node that contains the desired key
 *
 * Returns NULL if an error occurs, otherwise the key is returned
 */
void *ecore_tree_node_key_get(Ecore_Tree_Node * node)
{
	void *ret;

	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);
	ECORE_READ_LOCK(node);
	ret = node->key;
	ECORE_READ_UNLOCK(node);

	return ret;
}

/**
 * ecore_tree_destroy - free the tree and it's stored data
 * @tree: the tree to destroy
 *
 * Returns TRUE if tree destroyed successfully, FALSE if not.
 */
int ecore_tree_destroy(Ecore_Tree * tree)
{
	Ecore_Tree_Node *node;

	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

	ECORE_WRITE_LOCK(tree);
	while ((node = tree->tree)) {
		ecore_tree_remove_node(tree, node);
		ecore_tree_node_destroy(node, tree->free_func);
	}
	ECORE_WRITE_UNLOCK(tree);
	ECORE_DESTROY_LOCKS(tree);

	FREE(tree);

	return TRUE;
}

/**
 * ecore_tree_get_node - return the node corresponding to key
 * @tree: the tree to search
 * @key: the key to search for in the tree
 *
 * Returns the node corresponding to the key if found, otherwise NULL.
 */
Ecore_Tree_Node *ecore_tree_get_node(Ecore_Tree * tree, void *key)
{
	Ecore_Tree_Node *ret;

	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

	ECORE_READ_LOCK(tree);
	ret = tree_node_find(tree, key);
	ECORE_READ_UNLOCK(tree);

	return ret;
}

/**
 * ecore_tree_get - return the value corresponding to key
 * @tree: the tree to search
 * @key: the key to search for in @tree
 *
 * Returns the value corresponding to the key if found, otherwise NULL.
 */
void *ecore_tree_get(Ecore_Tree * tree, void *key)
{
	void *ret;
	Ecore_Tree_Node *node;

	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

	ECORE_READ_LOCK(tree);
	node = tree_node_find(tree, key);
	ECORE_READ_UNLOCK(tree);

	ECORE_READ_LOCK(node);
	ret = (node ? node->value : NULL);
	ECORE_READ_UNLOCK(node);

	return ret;
}

/**
 * ecore_tree_get_closest_larger - find the closest value greater >= key
 * @tree: the tree to search 
 * @key: the key to search for in @tree
 *
 * Returns NULL if no valid nodes, otherwise the node >= key
 */
void *ecore_tree_get_closest_larger(Ecore_Tree * tree, void *key)
{
	Ecore_Tree_Node *node;

	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

	ECORE_READ_LOCK(tree);
	node = tree_node_find(tree, key);
	ECORE_READ_UNLOCK(tree);

	if (node)
		return node;

	ECORE_READ_LOCK(tree);
	node = tree_node_find_parent(tree, key);

	if (!node) {
		ECORE_READ_UNLOCK(tree);
		return NULL;
	}

	ECORE_READ_LOCK(node);
	if (tree->compare_func(node->key, key) < 0)
		return NULL;
	ECORE_READ_UNLOCK(node);
	ECORE_READ_UNLOCK(tree);

	return node;
}

/**
 * ecore_tree_get_closest_smaller - find the closest value <= key
 * @tree: the tree to search
 * @key: the key to search for in tree
 *
 * Returns NULL if no valid nodes, otherwise the node <= key
 */
void *ecore_tree_get_closest_smaller(Ecore_Tree * tree, void *key)
{
	Ecore_Tree_Node *node;

	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

	ECORE_READ_LOCK(tree);
	node = tree_node_find(tree, key);
	ECORE_READ_UNLOCK(tree);

	if (node)
		return node;

	ECORE_READ_LOCK(tree);
	node = tree_node_find_parent(tree, key);
	ECORE_READ_LOCK(tree);

	if (node)
		node = node->right_child;

	return node;
}

/**
 * ecore_tree_set - set the value associated with key to @value
 * @tree: the tree that contains the key/value pair
 * @key: the key to identify which node to set a value
 * @value: value to set the found node
 *
 * Returns TRUE if successful, FALSE if not.
 */
int ecore_tree_set(Ecore_Tree * tree, void *key, void *value)
{
	Ecore_Tree_Node *node = NULL;

	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

	ECORE_READ_LOCK(tree);
	node = tree_node_find(tree, key);
	ECORE_READ_UNLOCK(tree);

	if (!node) {
		node = ecore_tree_node_new();
		ecore_tree_node_key_set(node, key);
		if (!ecore_tree_add_node(tree, node))
			return FALSE;
	}
	ecore_tree_node_value_set(node, value);

	ECORE_WRITE_LOCK(tree);
	for (; node; node = node->parent)
		tree_node_balance(tree, node);
	ECORE_WRITE_UNLOCK(tree);

	return TRUE;
}

/**
 * ecore_tree_add_node - place a node in the tree
 * @tree: the tree to add @node
 * @node: the node to add to @tree
 *
 * Returns TRUE on a successful add, FALSE otherwise.
 */
int ecore_tree_add_node(Ecore_Tree * tree, Ecore_Tree_Node * node)
{
	Ecore_Tree_Node *travel = NULL;

	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);
	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	/* Find where to put this new node. */
	if (!tree->tree) {
		tree->tree = node;
	} else {
		travel = tree_node_find_parent(tree, node->key);
		node->parent = travel;

		/* The node is less than travel */
		if (tree->compare_func(node->key, travel->key) < 0) {
			travel->right_child = node;
			travel->max_left = 1;
			/* The node is greater than travel */
		} else {
			travel->left_child = node;
			travel->max_right = 1;
		}
	}

	return TRUE;
}


/**
 * ecore_tree_remove_node - remove the node from the tree
 * @tree: the tree to remove @node
 * @node: the node to remove from @tree
 *
 * Returns TRUE on a successful remove, FALSE otherwise.
 */
int ecore_tree_remove_node(Ecore_Tree * tree, Ecore_Tree_Node * node)
{
	Ecore_Tree_Node *traverse;

	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);
	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	/*
	 * Find the nearest value to the balanced one.
	 */
	if (node->left_child) {
		traverse = node->left_child;

		/* Now work our way down right side of the traverse node.
		 * This will give us the node with the next closest value
		 * to the current node. If traverse had no right node, then
		 * this will stop at node's left node. */
		while (traverse->right_child) {
			traverse = traverse->right_child;
		}

		/*
		 * Hook any dropped leaves into the moved nodes spot
		 */
		if (traverse->parent)
			traverse->parent->left_child = traverse->left_child;
	}
	else if (node->right_child) {
		traverse = node->right_child;

		/* Now work our way down left side of the traverse node.
		 * This will give us the node with the next closest value
		 * to the current node. If traverse had no left node, then
		 * this will stop at node's right node. */
		while (traverse->left_child) {
			traverse = traverse->left_child;
		}

		/*
		 * Hook any dropped leaves into the moved nodes spot
		 */
		if (traverse->right_child)
			traverse->right_child->parent = traverse->parent;

		if (traverse->parent)
			traverse->parent->right_child = traverse->right_child;
		else
			tree->tree = traverse->right_child;
	}
	else
		traverse = NULL;

	if (traverse) {

		/*
		 * Ensure that we don't get a recursive reference.
		 */
		if (node->right_child && node->right_child != traverse) {
			node->right_child->parent = traverse;
			traverse->right_child = node->right_child;
		}

		if (node->left_child && node->left_child != traverse) {
			node->left_child->parent = traverse;
			traverse->left_child = node->left_child;
		}

		/*
		 * Unlink the node to be moved from it's parent.
		 */
		if (traverse->parent) {
			if (traverse->parent->left_child == traverse)
				traverse->parent->left_child = NULL;
			else
				traverse->parent->right_child = NULL;
		}
		traverse->parent = node->parent;
	}

	if (node->parent) {
		if (node == node->parent->left_child)
			node->parent->left_child = traverse;
		else
			node->parent->right_child = traverse;
	}

	if (tree->tree == node)
		tree->tree = traverse;

	node->parent = node->left_child = node->right_child = NULL;

	/*
	 * Rebalance the tree to ensure short search paths.
	 */
	while (traverse) {
		tree_node_balance(tree, traverse);
		traverse = traverse->parent;
	}

	return TRUE;
}

/**
 * ecore_tree_remove - remove the key from the tree
 * @tree: the tree to remove @key
 * @key: the key to remove from @tree
 *
 * Returns TRUE on a successful remove, FALSE otherwise.
 */
int ecore_tree_remove(Ecore_Tree * tree, void *key)
{
	Ecore_Tree_Node *node;

	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);
	if (!tree->tree)
		return FALSE;

	/* Find the node we need to remove */
	node = tree_node_find(tree, key);
	if (!node)
		return FALSE;

	if (!ecore_tree_remove_node(tree, node))
		return FALSE;

	ecore_tree_node_destroy(node, tree->free_func);

	return TRUE;
}

/**
 * ecore_tree_is_empty - test to see if the tree has any nodes
 * @tree: the tree to check for nodes
 *
 * Returns TRUE if no nodes exist, FALSE otherwise
 */
int ecore_tree_is_empty(Ecore_Tree * tree)
{
	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

	if (!tree->tree)
		return TRUE;

	return FALSE;
}

/**
 * ecore_tree_for_each_node_value - execute function for each value in the tree
 * @tree: the tree to traverse
 * @for_each_func: the function to execute for each value in the tree
 *
 * Returns TRUE on success, FALSE on failure.
 */
int ecore_tree_for_each_node_value(Ecore_Tree * tree,
				 Ecore_For_Each for_each_func)
{
	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);
	CHECK_PARAM_POINTER_RETURN("for_each_func", for_each_func, FALSE);

	if (!tree->tree)
		return FALSE;

	return tree_for_each_node_value(tree->tree, for_each_func);
}

/**
 * ecore_tree_for_each_node - execute the function for each node in the tree
 * @tree: the tree to traverse
 * @for_each_func: the function to execute for each node
 *
 * Returns TRUE on success, FALSE on failure.
 */
int ecore_tree_for_each_node(Ecore_Tree * tree, Ecore_For_Each for_each_func)
{
	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);
	CHECK_PARAM_POINTER_RETURN("for_each_func", for_each_func, FALSE);

	if (!tree->tree)
		return FALSE;

	return tree_for_each_node(tree->tree, for_each_func);
}

/* Find the parent for the key */
Ecore_Tree_Node *tree_node_find_parent(Ecore_Tree * tree, void *key)
{
	Ecore_Tree_Node *parent, *travel;

	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

	parent = tree_node_find(tree, key);
	if (parent)
		parent = parent->parent;

	travel = tree->tree;
	if (!travel)
		return NULL;

	while (!parent) {
		int compare;

		if ((compare = tree->compare_func(key, travel->key)) < 0) {
			if (!travel->right_child)
				parent = travel;
			travel = travel->right_child;
		} else {
			if (!travel->left_child)
				parent = travel;
			travel = travel->left_child;
		}
	}

	return parent;
}

/* Search for the node with a specified key */
Ecore_Tree_Node *tree_node_find(Ecore_Tree * tree, void *key)
{
	int compare;
	Ecore_Tree_Node *node;

	CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

	node = tree->tree;
	while (node && (compare = tree->compare_func(key, node->key)) != 0) {

		if (compare < 0) {
			if (!node->right_child) {
				return NULL;
			}

			node = node->right_child;
		} else {
			if (!node->left_child) {
				return NULL;
			}

			node = node->left_child;
		}
	}

	return node;
}

/* Balance the tree with respect to node */
int tree_node_balance(Ecore_Tree * tree, Ecore_Tree_Node * top_node)
{
	int balance;

	CHECK_PARAM_POINTER_RETURN("top_node", top_node, FALSE);

	/* Get the height of the left branch. */
	if (top_node->right_child) {
		top_node->max_left = MAX_HEIGHT(top_node->right_child) + 1;
	} else
		top_node->max_left = 0;

	/* Get the height of the right branch. */
	if (top_node->left_child) {
		top_node->max_right = MAX_HEIGHT(top_node->left_child) + 1;
	} else
		top_node->max_right = 0;

	/* Determine which side has a larger height. */
	balance = top_node->max_right - top_node->max_left;

	/* if the left side has a height advantage >1 rotate right */
	if (balance < -1)
		tree_node_rotate_right(tree, top_node);
	/* else if the left side has a height advantage >1 rotate left */
	else if (balance > 1)
		tree_node_rotate_left(tree, top_node);

	return TRUE;
}

/* Tree is overbalanced to the left, so rotate nodes to the right. */
int tree_node_rotate_right(Ecore_Tree * tree, Ecore_Tree_Node * top_node)
{
	Ecore_Tree_Node *temp;

	CHECK_PARAM_POINTER_RETURN("top_node", top_node, FALSE);

	/* The left branch's right branch becomes the nodes left branch,
	 * the left branch becomes the top node, and the node becomes the
	 * right branch. */
	temp = top_node->right_child;
	top_node->right_child = temp->left_child;
	temp->left_child = top_node;

	/* Make sure the nodes know who their new parents are and the tree
	 * structure knows the start of the tree. */
	temp->parent = top_node->parent;
	if (temp->parent == NULL)
		tree->tree = temp;
	else {
		if (temp->parent->left_child == top_node)
			temp->parent->left_child = temp;
		else
			temp->parent->right_child = temp;
	}
	top_node->parent = temp;

	/* And recalculate node heights */
	tree_node_balance(tree, top_node);
	tree_node_balance(tree, temp);

	return TRUE;
}

/* The tree is overbalanced to the right, so we rotate nodes to the left */
int tree_node_rotate_left(Ecore_Tree * tree, Ecore_Tree_Node * top_node)
{
	Ecore_Tree_Node *temp;

	CHECK_PARAM_POINTER_RETURN("top_node", top_node, FALSE);

	/*
	 * The right branch's left branch becomes the nodes right branch,
	 * the right branch becomes the top node, and the node becomes the
	 * left branch.
	 */
	temp = top_node->left_child;
	top_node->left_child = temp->right_child;
	temp->right_child = top_node;

	/* Make sure the nodes know who their new parents are. */
	temp->parent = top_node->parent;
	if (temp->parent == NULL)
		tree->tree = temp;
	else {
		if (temp->parent->left_child == top_node)
			temp->parent->left_child = temp;
		else
			temp->parent->right_child = temp;
	}
	top_node->parent = temp;

	/* And recalculate node heights */
	tree_node_balance(tree, top_node);
	tree_node_balance(tree, temp);

	return TRUE;
}

/*
 * Description: Execute a function for each node below this point in the tree.
 * Parameters: 1. node - the highest node in the tree the function will be
 *                       executed for
 *             2. for_each_func - the function to pass the nodes as data into
 * Returns: FALSE if an error condition occurs, otherwise TRUE
 */
int tree_for_each_node(Ecore_Tree_Node * node, Ecore_For_Each for_each_func)
{
	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	if (node->right_child)
		tree_for_each_node(node->right_child, for_each_func);

	if (node->left_child)
		tree_for_each_node(node->left_child, for_each_func);

	for_each_func(node);

	return TRUE;
}


/*
 * Description: Execute a function for each node below this point in the tree.
 * Parameters: 1. node - the highest node in the tree the function will be
 *                    executed for
 *             2. for_each_func - the function to pass the nodes values as data
 * Returns: FALSE if an error condition occurs, otherwise TRUE
 */
int tree_for_each_node_value(Ecore_Tree_Node * node,
			     Ecore_For_Each for_each_func)
{
	CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

	if (node->right_child)
		tree_for_each_node_value(node->right_child, for_each_func);

	if (node->left_child)
		tree_for_each_node_value(node->left_child, for_each_func);

	for_each_func(node->value);

	return TRUE;
}
