#include "ecore_private.h"
#include "Ecore.h"
#include "Ecore_Data.h"

/* A macro for determining the highest node at given branch */
#define MAX_HEIGHT(node) (node ? MAX(node->max_left, node->max_right) : 0)

/* Utility functions for searching the tree and returning a node, or its
 * parent */
static Ecore_Tree_Node *tree_node_find(Ecore_Tree * tree, const void *key);
static Ecore_Tree_Node *tree_node_find_parent(Ecore_Tree * tree, const void *key);

/* Balancing functions, keep the tree balanced within a one node height
 * difference */
static int tree_node_balance(Ecore_Tree * Tree, Ecore_Tree_Node * top_node);
static int tree_node_rotate_right(Ecore_Tree * tree, Ecore_Tree_Node * top_node);
static int tree_node_rotate_left(Ecore_Tree * tree, Ecore_Tree_Node * top_node);

/* Fucntions for executing a specified function on each node of a tree */
static int tree_for_each_node(Ecore_Tree_Node * node, Ecore_For_Each for_each_func,
			      void *user_data);
static int tree_for_each_node_value(Ecore_Tree_Node * node,
				    Ecore_For_Each for_each_func, void *user_data);

/**
 * @brief Allocate a new tree structure.
 * @param compare_func: function used to compare the two values
 * @return Returns NULL if the operation fails, otherwise the new tree
 */
EAPI Ecore_Tree *
ecore_tree_new(Ecore_Compare_Cb compare_func)
{
   Ecore_Tree *new_tree;

   new_tree = ECORE_TREE(malloc(sizeof(Ecore_Tree)));
   if (!new_tree)
     return NULL;

   if (!ecore_tree_init(new_tree, compare_func))
     {
	IF_FREE(new_tree);
	return NULL;
     }

   return new_tree;
}

/**
 * @brief Initialize a tree structure to some sane initial values
 * @param new_tree: the new tree structure to be initialized
 * @param compare_func: the function used to compare node keys
 * @return Returns TRUE on successful initialization, FALSE on an error
 */
EAPI int 
ecore_tree_init(Ecore_Tree *new_tree, Ecore_Compare_Cb compare_func)
{
   CHECK_PARAM_POINTER_RETURN("new_tree", new_tree, FALSE);

   memset(new_tree, 0, sizeof(Ecore_Tree));

   if (!compare_func)
     new_tree->compare_func = ecore_direct_compare;
   else
     new_tree->compare_func = compare_func;

   return TRUE;
}

/*
 * @brief Add a function to be called at node destroy time
 * @param tree: the tree that will use this function when nodes are destroyed
 * @param free_func: the function that will be passed the node being freed
 * @return Returns TRUE on successful set, FALSE otherwise.
 */
EAPI int 
ecore_tree_free_value_cb_set(Ecore_Tree *tree, Ecore_Free_Cb free_value)
{
   CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

   tree->free_value = free_value;

   return TRUE;
}

/*
 * @brief Add a function to be called at node destroy time
 * @param tree: the tree that will use this function when nodes are destroyed
 * @param free_key: the function that will be passed the node being freed
 * @return Returns TRUE on successful set, FALSE otherwise.
 */
EAPI int 
ecore_tree_free_key_cb_set(Ecore_Tree *tree, Ecore_Free_Cb free_key)
{
   CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

   tree->free_key = free_key;

   return TRUE;
}

/*
 * @brief Initialize a new tree node
 * @return Returns FALSE if the operation fails, otherwise TRUE
 */
EAPI int 
ecore_tree_node_init(Ecore_Tree_Node *new_node)
{
   CHECK_PARAM_POINTER_RETURN("new_node", new_node, FALSE);

   new_node->key = NULL;
   new_node->value = NULL;

   new_node->parent = NULL;
   new_node->right_child = NULL;
   new_node->left_child = NULL;

   new_node->max_left = new_node->max_right = 0;

   return TRUE;
}

/*
 * @brief Allocate a new tree node
 * @return Returns NULL if the operation fails, otherwise the new node.
 */
EAPI Ecore_Tree_Node *
ecore_tree_node_new()
{
   Ecore_Tree_Node *new_node;

   new_node = ECORE_TREE_NODE(malloc(sizeof(Ecore_Tree_Node)));
   if (!new_node)
     return NULL;

   if (!ecore_tree_node_init(new_node))
     {
	IF_FREE(new_node);
	return NULL;
     }

   return new_node;
}

/*
 * @brief Free a tree node and it's children.
 * @param node: tree node to be free()'d
 * @param data_free: callback for destroying the data held in node
 * @return Returns TRUE if the node is destroyed successfully, FALSE if not.
 *
 * If you don't want the children free'd then you need to remove the node first.
 */
EAPI int 
ecore_tree_node_destroy(Ecore_Tree_Node *node, Ecore_Free_Cb value_free, Ecore_Free_Cb key_free)
{
   CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

   if (key_free)
     key_free(node->key);
   if (value_free)
     value_free(node->value);

   FREE(node);

   return TRUE;
}

/*
 * @brief Set the value of the node to value
 * @param node: the node to be set
 * @param value: the value to set the node to.
 * @return Returns TRUE if the node is set successfully, FALSE if not.
 */
EAPI int 
ecore_tree_node_value_set(Ecore_Tree_Node *node, void *value)
{
   CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

   node->value = value;

   return TRUE;
}

/*
 * @brief Get the value of the node
 * @param node: the node that contains the desired value
 * @return Returns NULL if an error, otherwise the value associated with node
 */
EAPI void *
ecore_tree_node_value_get(Ecore_Tree_Node *node)
{
   void *ret;

   CHECK_PARAM_POINTER_RETURN("node", node, NULL);
   ret = node->value;

   return ret;
}

/*
 * @brief Set the value of the node's key  to key
 * @param node: the node to be set
 * @param key: the value to set it's key to.
 * @return Returns TRUE if the node is set successfully, FALSE if not.
 */
EAPI int 
ecore_tree_node_key_set(Ecore_Tree_Node *node, void *key)
{
   CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

   node->key = key;

   return TRUE;
}

/*
 * @brief Get the value of the node's key
 * @param node: the node that contains the desired key
 *
 * @return Returns NULL if an error occurs, otherwise the key is returned
 */
EAPI void *
ecore_tree_node_key_get(Ecore_Tree_Node *node)
{
   void *ret;

   CHECK_PARAM_POINTER_RETURN("node", node, NULL);
   ret = node->key;

   return ret;
}

/**
 * @brief Free the tree and it's stored data
 * @param tree: the tree to destroy
 *
 * @return Returns TRUE if tree destroyed successfully, FALSE if not.
 */
EAPI int 
ecore_tree_destroy(Ecore_Tree *tree)
{
   Ecore_Tree_Node *node;

   CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

   while ((node = tree->tree))
     {
	ecore_tree_node_remove(tree, node);
	ecore_tree_node_destroy(node, tree->free_value, tree->free_key);
     }

   FREE(tree);

   return TRUE;
}

/**
 * @brief Return the node corresponding to key
 * @param tree: the tree to search
 * @param key: the key to search for in the tree
 *
 * @return Returns the node corresponding to the key if found, otherwise NULL.
 */
EAPI Ecore_Tree_Node *
ecore_tree_get_node(Ecore_Tree *tree, const void *key)
{
   Ecore_Tree_Node *ret;

   CHECK_PARAM_POINTER_RETURN("tree", tree, NULL);

   ret = tree_node_find(tree, key);

   return ret;
}

/**
 * @brief Return the value corresponding to key
 * @param tree: the tree to search
 * @param key: the key to search for in @a tree
 * @return Returns the value corresponding to the key if found, otherwise NULL.
 */
EAPI void *
ecore_tree_get(Ecore_Tree *tree, const void *key)
{
   void *ret;
   Ecore_Tree_Node *node;

   CHECK_PARAM_POINTER_RETURN("tree", tree, NULL);

   node = tree_node_find(tree, key);
   ret = (node ? node->value : NULL);

   return ret;
}

/**
 * @brief Find the closest value greater than or equal to the key.
 * @param  tree The tree to search.
 * @param  key  The key to search for in @a tree.
 * @return NULL if no valid nodes, otherwise the node greater than or
 *         equal to the key
 */
EAPI void *
ecore_tree_closest_larger_get(Ecore_Tree *tree, const void *key)
{
   Ecore_Tree_Node *node;

   CHECK_PARAM_POINTER_RETURN("tree", tree, NULL);

   node = tree_node_find(tree, key);
   if (node)
     return node;

   node = tree_node_find_parent(tree, key);
   if (!node)
     return NULL;

   if (tree->compare_func(node->key, key) < 0)
     return NULL;

   return node;
}

/**
 * @brief Find the closest value <= key
 * @param tree the tree to search
 * @param key  the key to search for in tree
 * @return Returns NULL if no valid nodes, otherwise the node <= key
 */
EAPI void *
ecore_tree_closest_smaller_get(Ecore_Tree *tree, const void *key)
{
   Ecore_Tree_Node *node;

   CHECK_PARAM_POINTER_RETURN("tree", tree, NULL);

   node = tree_node_find(tree, key);
   if (node)
     return node;

   node = tree_node_find_parent(tree, key);
   if (node)
     node = node->right_child;

   return node;
}

/**
 * Set the value associated with key to @a value.
 * @param  tree  The tree that contains the key/value pair.
 * @param  key   The key to identify which node to set a value.
 * @param  value Value to set the found node.
 * @return TRUE if successful, FALSE if not.
 */
EAPI int 
ecore_tree_set(Ecore_Tree *tree, void *key, void *value)
{
   Ecore_Tree_Node *node = NULL;

   CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

   node = tree_node_find(tree, key);
   if (!node)
     {
	node = ecore_tree_node_new();
	ecore_tree_node_key_set(node, key);
	if (!ecore_tree_node_add(tree, node))
	  return FALSE;
     }
   else 
     {
        if (tree->free_key) 
	  tree->free_key(key);
        if (node->value && tree->free_value)
	  tree->free_value(node->value);
     }

   ecore_tree_node_value_set(node, value);

   for (; node; node = node->parent)
     tree_node_balance(tree, node);

   return TRUE;
}

/**
 * Place a node in the tree.
 * @param tree The tree to add @a node.
 * @param node The node to add to @a tree.
 * @return TRUE on a successful add, FALSE otherwise.
 */
EAPI int 
ecore_tree_node_add(Ecore_Tree *tree, Ecore_Tree_Node *node)
{
   Ecore_Tree_Node *travel = NULL;

   CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);
   CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

   /* Find where to put this new node. */
   if (!tree->tree)
     tree->tree = node;
   else
     {
	travel = tree_node_find_parent(tree, node->key);
	node->parent = travel;

	/* The node is less than travel */
	if (tree->compare_func(node->key, travel->key) < 0)
	  {
	     travel->right_child = node;
	     travel->max_left = 1;
	     /* The node is greater than travel */
	  }
	else
	  {
	     travel->left_child = node;
	     travel->max_right = 1;
	  }
     }

   return TRUE;
}

/**
 * Remove the node from the tree.
 * @param  tree The tree to remove @a node from.
 * @param  node The node to remove from @a tree.
 * @return TRUE on a successful remove, FALSE otherwise.
 */
EAPI int 
ecore_tree_node_remove(Ecore_Tree *tree, Ecore_Tree_Node *node)
{
   Ecore_Tree_Node *traverse;

   CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);
   CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

   /*
    * Find the nearest value to the balanced one.
    */
   if (node->left_child)
     {
	traverse = node->left_child;

	/* Now work our way down right side of the traverse node.
	 * This will give us the node with the next closest value
	 * to the current node. If traverse had no right node, then
	 * this will stop at node's left node. */
	while (traverse->right_child)
	  {
	     traverse = traverse->right_child;
	  }

	/*
	 * Hook any dropped leaves into the moved nodes spot
	 */
	if (traverse->parent)
	  traverse->parent->left_child = traverse->left_child;
     }
   else if (node->right_child)
     {
	traverse = node->right_child;

	/* Now work our way down left side of the traverse node.
	 * This will give us the node with the next closest value
	 * to the current node. If traverse had no left node, then
	 * this will stop at node's right node. */
	while (traverse->left_child)
	  {
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

   if (traverse)
     {

	/*
	 * Ensure that we don't get a recursive reference.
	 */
	if (node->right_child && node->right_child != traverse)
	  {
	     node->right_child->parent = traverse;
	     traverse->right_child = node->right_child;
	  }

	if (node->left_child && node->left_child != traverse)
	  {
	     node->left_child->parent = traverse;
	     traverse->left_child = node->left_child;
	  }

	/*
	 * Unlink the node to be moved from it's parent.
	 */
	if (traverse->parent)
	  {
	     if (traverse->parent->left_child == traverse)
	       traverse->parent->left_child = NULL;
	     else
	       traverse->parent->right_child = NULL;
	  }
	traverse->parent = node->parent;
     }

   if (node->parent)
     {
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
   while (traverse)
     {
	tree_node_balance(tree, traverse);
	traverse = traverse->parent;
     }

   return TRUE;
}

/**
 * Remove the key from the tree.
 * @param  tree The tree to remove @a key.
 * @param  key  The key to remove from @a tree.
 * @return TRUE on a successful remove, FALSE otherwise.
 */
EAPI int 
ecore_tree_remove(Ecore_Tree *tree, const void *key)
{
   Ecore_Tree_Node *node;

   CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);
   if (!tree->tree)
     return FALSE;

   /* Find the node we need to remove */
   node = tree_node_find(tree, key);
   if (!node)
     return FALSE;

   if (!ecore_tree_node_remove(tree, node))
     return FALSE;

   ecore_tree_node_destroy(node, tree->free_value, tree->free_key);

   return TRUE;
}

/**
 * @brief Test to see if the tree has any nodes
 * @param tree: the tree to check for nodes
 * @return Returns TRUE if no nodes exist, FALSE otherwise
 */
EAPI int 
ecore_tree_empty_is(Ecore_Tree *tree)
{
   CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);

   if (!tree->tree)
     return TRUE;

   return FALSE;
}

/**
 * @brief Execute function for each value in the tree
 * @param tree: the tree to traverse
 * @param for_each_func: the function to execute for each value in the tree
 * @param user_data: data passed to each for_each_func call
 * @return Returns TRUE on success, FALSE on failure.
 */
EAPI int 
ecore_tree_for_each_node_value(Ecore_Tree *tree, Ecore_For_Each for_each_func, void *user_data)
{
   CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);
   CHECK_PARAM_POINTER_RETURN("for_each_func", for_each_func, FALSE);

   if (!tree->tree)
     return FALSE;

   return tree_for_each_node_value(tree->tree, for_each_func, user_data);
}

/**
 * @brief Execute the function for each node in the tree
 * @param tree: the tree to traverse
 * @param for_each_func: the function to execute for each node
 * @param user_data: data passed to each for_each_func call
 * @return Returns TRUE on success, FALSE on failure.
 */
EAPI int 
ecore_tree_for_each_node(Ecore_Tree *tree, Ecore_For_Each for_each_func, void *user_data)
{
   CHECK_PARAM_POINTER_RETURN("tree", tree, FALSE);
   CHECK_PARAM_POINTER_RETURN("for_each_func", for_each_func, FALSE);

   if (!tree->tree)
     return FALSE;

   return tree_for_each_node(tree->tree, for_each_func, user_data);
}

/* Find the parent for the key */
static Ecore_Tree_Node *
tree_node_find_parent(Ecore_Tree *tree, const void *key)
{
   Ecore_Tree_Node *parent, *travel;

   CHECK_PARAM_POINTER_RETURN("tree", tree, NULL);

   parent = tree_node_find(tree, key);
   if (parent)
     parent = parent->parent;

   travel = tree->tree;
   if (!travel)
     return NULL;

   while (!parent)
     {
	int compare;

	if ((compare = tree->compare_func(key, travel->key)) < 0)
	  {
	     if (!travel->right_child)
	       parent = travel;
	     travel = travel->right_child;
	  }
	else
	  {
	     if (!travel->left_child)
	       parent = travel;
	     travel = travel->left_child;
	  }
     }

   return parent;
}

/* Search for the node with a specified key */
static Ecore_Tree_Node *
tree_node_find(Ecore_Tree *tree, const void *key)
{
   int compare;
   Ecore_Tree_Node *node;

   CHECK_PARAM_POINTER_RETURN("tree", tree, NULL);

   node = tree->tree;
   while (node && (compare = tree->compare_func(key, node->key)) != 0)
     {

	if (compare < 0)
	  {
	     if (!node->right_child)
	       return NULL;

	     node = node->right_child;
	  }
	else
	  {
	     if (!node->left_child)
	       return NULL;

	     node = node->left_child;
	  }
     }

   return node;
}

/* Balance the tree with respect to node */
static int 
tree_node_balance(Ecore_Tree *tree, Ecore_Tree_Node *top_node)
{
   int balance;

   CHECK_PARAM_POINTER_RETURN("top_node", top_node, FALSE);

   /* Get the height of the left branch. */
   if (top_node->right_child)
     top_node->max_left = MAX_HEIGHT(top_node->right_child) + 1;
   else
     top_node->max_left = 0;

   /* Get the height of the right branch. */
   if (top_node->left_child)
     top_node->max_right = MAX_HEIGHT(top_node->left_child) + 1;
   else
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
static int 
tree_node_rotate_right(Ecore_Tree *tree, Ecore_Tree_Node *top_node)
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
   else
     {
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
static int 
tree_node_rotate_left(Ecore_Tree *tree, Ecore_Tree_Node *top_node)
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
   else
     {
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
 * @brief Execute a function for each node below this point in the tree.
 * @param node: the highest node in the tree the function will be executed for
 * @param for_each_func: the function to pass the nodes as data into
 * @param user_data: data passed to each for_each_func call
 * @return Returns FALSE if an error condition occurs, otherwise TRUE
 */
static int 
tree_for_each_node(Ecore_Tree_Node * node, Ecore_For_Each for_each_func, void *user_data)
{
   CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

   if (node->right_child)
     tree_for_each_node(node->right_child, for_each_func, user_data);

   if (node->left_child)
     tree_for_each_node(node->left_child, for_each_func, user_data);

   for_each_func(node, user_data);

   return TRUE;
}

/*
 * @brief Execute a function for each node below this point in the tree.
 * @param node: the highest node in the tree the function will be executed for
 * @param for_each_func: the function to pass the nodes values as data
 * @return Returns FALSE if an error condition occurs, otherwise TRUE
 */
static int 
tree_for_each_node_value(Ecore_Tree_Node *node, Ecore_For_Each for_each_func, void *user_data)
{
   CHECK_PARAM_POINTER_RETURN("node", node, FALSE);

   if (node->right_child)
     tree_for_each_node_value(node->right_child, for_each_func, user_data);

   if (node->left_child)
     tree_for_each_node_value(node->left_child, for_each_func, user_data);

   for_each_func(node->value, user_data);

   return TRUE;
}
