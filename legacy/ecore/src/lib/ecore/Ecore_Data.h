#ifndef _ECORE_DATA_H
# define _ECORE_DATA_H

#ifdef EAPI
#undef EAPI
#endif
#ifdef WIN32
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

/**
 * @file Ecore_Data.h
 * @brief Contains threading, list, hash, debugging and tree functions.
 */

# ifdef __cplusplus
extern "C" {
# endif

# ifdef __sgi
#  define __FUNCTION__ "unknown"
#  ifndef __cplusplus
#   define inline
#  endif
# endif
   
   EAPI extern const unsigned int ecore_prime_table[];
   
   typedef void (*Ecore_For_Each) (void *value, void *user_data);
# define ECORE_FOR_EACH(function) ((Ecore_For_Each)function)
   
   typedef void (*Ecore_Free_Cb) (void *data);
# define ECORE_FREE_CB(func) ((Ecore_Free_Cb)func)
   
   typedef unsigned int (*Ecore_Hash_Cb) (void *key);
# define ECORE_HASH_CB(function) ((Ecore_Hash_Cb)function)
   
   typedef int (*Ecore_Compare_Cb) (void *data1, void *data2);
# define ECORE_COMPARE_CB(function) ((Ecore_Compare_Cb)function)
   
   typedef struct _ecore_list Ecore_List;
# define ECORE_LIST(list) ((Ecore_List *)list)
   
   typedef struct _ecore_list_node Ecore_List_Node;
# define ECORE_LIST_NODE(node) ((Ecore_List_Node *)node)
   
   struct _ecore_list_node {
      void *data;
      struct _ecore_list_node *next;
   };
   
   struct _ecore_list {
      Ecore_List_Node *first;	/* The first node in the list */
      Ecore_List_Node *last;	/* The last node in the list */
      Ecore_List_Node *current;	/* The current node in the list */
      
      Ecore_Free_Cb free_func;  /* The callback to free data in nodes */
      
      int nodes;		/* The number of nodes in the list */
      int index;		/* The position from the front of the
				 list of current node */
   };
   
   EAPI int ecore_direct_compare(void *key1, void *key2);
   EAPI int ecore_str_compare(void *key1, void *key2);
   
   EAPI unsigned int ecore_direct_hash(void *key);
   EAPI unsigned int ecore_str_hash(void *key);
   
   /* Creating and initializing new list structures */
   EAPI Ecore_List *ecore_list_new(void);
   EAPI int ecore_list_init(Ecore_List *list);
   
   /* Adding items to the list */
   EAPI int ecore_list_append(Ecore_List * list, void *_data);
   EAPI int ecore_list_prepend(Ecore_List * list, void *_data);
   EAPI int ecore_list_insert(Ecore_List * list, void *_data);
   
   /* Removing items from the list */
   EAPI int ecore_list_remove_destroy(Ecore_List *list);
   EAPI void *ecore_list_remove(Ecore_List * list);
   EAPI void *ecore_list_remove_first(Ecore_List * list);
   EAPI void *ecore_list_remove_last(Ecore_List * list);
   
   /* Retrieve the current position in the list */
   EAPI void *ecore_list_current(Ecore_List * list);
   EAPI void *ecore_list_first(Ecore_List * list);
   EAPI void *ecore_list_last(Ecore_List * list);
   EAPI int ecore_list_index(Ecore_List * list);
   EAPI int ecore_list_nodes(Ecore_List * list);
   
   /* Traversing the list */
   EAPI int ecore_list_for_each(Ecore_List *list, Ecore_For_Each function,
				void *user_data);
   EAPI void *ecore_list_goto_first(Ecore_List * list);
   EAPI void *ecore_list_goto_last(Ecore_List * list);
   EAPI void *ecore_list_goto_index(Ecore_List * list, int index);
   EAPI void *ecore_list_goto(Ecore_List * list, void *_data);
   
   /* Traversing the list and returning data */
   EAPI void *ecore_list_next(Ecore_List * list);
   
   /* Check to see if there is any data in the list */
   EAPI int ecore_list_is_empty(Ecore_List * list);
   
   /* Remove every node in the list without freeing the list itself */
   EAPI int ecore_list_clear(Ecore_List * list);
   /* Free the list and it's contents */
   EAPI void ecore_list_destroy(Ecore_List *list);
   
   /* Creating and initializing list nodes */
   EAPI Ecore_List_Node *ecore_list_node_new(void);
   EAPI int ecore_list_node_init(Ecore_List_Node *newNode);
   
   /* Destroying nodes */
   EAPI int ecore_list_node_destroy(Ecore_List_Node * _e_node, Ecore_Free_Cb free_func);
   
   EAPI int ecore_list_set_free_cb(Ecore_List * list, Ecore_Free_Cb free_func);
   
   typedef Ecore_List Ecore_DList;
# define ECORE_DLIST(dlist) ((Ecore_DList *)dlist)
   
   typedef struct _ecore_dlist_node Ecore_DList_Node;
# define ECORE_DLIST_NODE(dlist) ((Ecore_DList_Node *)dlist)
   
   struct _ecore_dlist_node {
      Ecore_List_Node single;
      Ecore_DList_Node *previous;
   };
   
   /* Creating and initializing new list structures */
   EAPI Ecore_DList *ecore_dlist_new(void);
   EAPI int ecore_dlist_init(Ecore_DList *list);
   EAPI void ecore_dlist_destroy(Ecore_DList *list);
   
   /* Adding items to the list */
   EAPI int ecore_dlist_append(Ecore_DList * _e_dlist, void *_data);
   EAPI int ecore_dlist_prepend(Ecore_DList * _e_dlist, void *_data);
   EAPI int ecore_dlist_insert(Ecore_DList * _e_dlist, void *_data);
   
   /* Info about list's state */
   EAPI void *ecore_dlist_current(Ecore_DList *list);
   EAPI int ecore_dlist_index(Ecore_DList *list);
# define ecore_dlist_nodes(list) ecore_list_nodes(ECORE_LIST(list))
   
   /* Removing items from the list */
   EAPI void *ecore_dlist_remove(Ecore_DList * _e_dlist);
   EAPI void *ecore_dlist_remove_first(Ecore_DList * _e_dlist);
   EAPI int ecore_dlist_remove_destroy(Ecore_DList *list);
   EAPI void *ecore_dlist_remove_last(Ecore_DList * _e_dlist);
   
   /* Traversing the list */
# define ecore_dlist_for_each(list, function, user_data) \
   ecore_list_for_each(ECORE_LIST(list), function, user_data)
   EAPI void *ecore_dlist_goto_first(Ecore_DList * _e_dlist);
   EAPI void *ecore_dlist_goto_last(Ecore_DList * _e_dlist);
   EAPI void *ecore_dlist_goto_index(Ecore_DList * _e_dlist, int index);
   EAPI void *ecore_dlist_goto(Ecore_DList * _e_dlist, void *_data);
   
   /* Traversing the list and returning data */
   EAPI void *ecore_dlist_next(Ecore_DList * list);
   EAPI void *ecore_dlist_previous(Ecore_DList * list);
   
   /* Check to see if there is any data in the list */
   EAPI int ecore_dlist_is_empty(Ecore_DList * _e_dlist);
   
   /* Remove every node in the list without free'ing it */
   EAPI int ecore_dlist_clear(Ecore_DList * _e_dlist);
   
   /* Creating and initializing list nodes */
   EAPI int ecore_dlist_node_init(Ecore_DList_Node * node);
   EAPI Ecore_DList_Node *ecore_dlist_node_new(void);
   
   /* Destroying nodes */
   EAPI int ecore_dlist_node_destroy(Ecore_DList_Node * node, Ecore_Free_Cb free_func);
   
   EAPI int ecore_dlist_set_free_cb(Ecore_DList * dlist, Ecore_Free_Cb free_func);
   
   
   
   /*
    * Hash Table Implementation:
    * 
    * Traditional hash table implementation. I had tried a list of tables
    * approach to save on the realloc's but it ended up being much slower than
    * the traditional approach.
    */
   
   typedef struct _ecore_hash_node Ecore_Hash_Node;
# define ECORE_HASH_NODE(hash) ((Ecore_Hash_Node *)hash)
   
   struct _ecore_hash_node {
      Ecore_Hash_Node *next; /* Pointer to the next node in the bucket list */
      void *key;	     /* The key for the data node */
      void *value;	     /* The value associated with this node */
   };
   
   typedef struct _ecore_hash Ecore_Hash;
# define ECORE_HASH(hash) ((Ecore_Hash *)hash)
   
   struct _ecore_hash {
      Ecore_Hash_Node **buckets;
      int size;		/* An index into the table of primes to
			 determine size */
      int nodes;		/* The number of nodes currently in the hash */

      int index;    /* The current index into the bucket table */
      
      Ecore_Compare_Cb compare;	/* The function used to compare node values */
      Ecore_Hash_Cb hash_func;	/* The function used to compare node values */
      
      Ecore_Free_Cb free_key;	/* The callback function to free key */
      Ecore_Free_Cb free_value;	/* The callback function to determine hash */
   };
   
   /* Create and initialize a hash */
   EAPI Ecore_Hash *ecore_hash_new(Ecore_Hash_Cb hash_func, Ecore_Compare_Cb compare);
   EAPI int ecore_hash_init(Ecore_Hash *hash, Ecore_Hash_Cb hash_func, Ecore_Compare_Cb compare);
   
   /* Functions related to freeing the data in the hash table */
   EAPI int ecore_hash_set_free_key(Ecore_Hash *hash, Ecore_Free_Cb function);
   EAPI int ecore_hash_set_free_value(Ecore_Hash *hash, Ecore_Free_Cb function);
   EAPI void ecore_hash_destroy(Ecore_Hash *hash);

   EAPI int ecore_hash_for_each_node(Ecore_Hash *hash, Ecore_For_Each for_each_func,
				     void *user_data);
   EAPI Ecore_List *ecore_hash_keys(Ecore_Hash *hash);
   
   /* Retrieve and store data into the hash */
   EAPI void *ecore_hash_get(Ecore_Hash *hash, void *key);
   EAPI int ecore_hash_set(Ecore_Hash *hash, void *key, void *value);
   EAPI void *ecore_hash_remove(Ecore_Hash *hash, void *key);
   EAPI void ecore_hash_dump_graph(Ecore_Hash *hash);


   typedef struct _ecore_path_group Ecore_Path_Group;
   struct _ecore_path_group
     {
	int id;
	char *name;
	Ecore_List *paths;
     };
   
   /*
    * Create a new path group
    */
   EAPI int ecore_path_group_new(char *group_name);
   
   /*
    * Destroy a previous path group
    */
   EAPI void ecore_path_group_del(int group_id);
   
   /*
    * Add a directory to be searched for files
    */
   EAPI void ecore_path_group_add(int group_id, char *path);
   
   /*
    * Remove a directory to be searched for files
    */
   EAPI void ecore_path_group_remove(int group_id, char *path);
   
   /*
    * Find the absolute path if it exists in the group of paths
    */
   EAPI char * ecore_path_group_find(int group_id, char *name);
   
   /*
    * Get a list of all the available files in a path set
    */
   EAPI Ecore_List * ecore_path_group_available(int group_id);
   
   
   typedef struct _ecore_plugin Ecore_Plugin;
   struct _ecore_plugin
     {
	int group;
	char *name;
	void *handle;
     };
   
   /*
    * Load the specified plugin
    */
   EAPI Ecore_Plugin *ecore_plugin_load(int group_id, char *plugin);
   
   /*
    * Unload the specified plugin
    */
   EAPI void ecore_plugin_unload(Ecore_Plugin * plugin);
   
   /*
    * Lookup the specified symbol for the plugin
    */
   EAPI void *ecore_plugin_call(Ecore_Plugin * plugin, char *symbol_name);
   
   EAPI Ecore_List *ecore_plugin_get_available(int group_id);


# define ECORE_SHEAP_MIN 0
# define ECORE_SHEAP_MAX 1

   typedef struct _ecore_heap Ecore_Sheap;
# define ECORE_HEAP(heap) ((Ecore_Sheap *)heap)
   
   struct _ecore_heap {
      void **data;
      int size;
      int space;
      
      char order, sorted;
      
      /* Callback for comparing node values, default is direct comparison */
      Ecore_Compare_Cb compare;

      /* Callback for freeing node data, default is NULL */
      Ecore_Free_Cb free_func;
   };
   
   EAPI Ecore_Sheap *ecore_sheap_new(Ecore_Compare_Cb compare, int size);
   EAPI void ecore_sheap_destroy(Ecore_Sheap *heap);
   EAPI int ecore_sheap_init(Ecore_Sheap *heap, Ecore_Compare_Cb compare, int size);
   EAPI int ecore_sheap_set_free_cb(Ecore_Sheap *heap, Ecore_Free_Cb free_func);
   EAPI int ecore_sheap_insert(Ecore_Sheap *heap, void *data);
   EAPI void *ecore_sheap_extract(Ecore_Sheap *heap);
   EAPI void *ecore_sheap_extreme(Ecore_Sheap *heap);
   EAPI int ecore_sheap_change(Ecore_Sheap *heap, void *item, void *newval);
   EAPI int ecore_sheap_set_compare(Ecore_Sheap *heap, Ecore_Compare_Cb compare);
   EAPI void ecore_sheap_set_order(Ecore_Sheap *heap, char order);
   EAPI void ecore_sheap_sort(Ecore_Sheap *heap);
   
   EAPI void *ecore_sheap_item(Ecore_Sheap *heap, int i);
   
   
   typedef struct _ecore_string Ecore_String;
   struct _ecore_string {
      char *string;
      int references;
   };
   
   EAPI int ecore_string_init(void);
   EAPI void ecore_string_shutdown(void);
   EAPI const char *ecore_string_instance(char *string);
   EAPI void ecore_string_release(const char *string);
   
   
   typedef struct _Ecore_Tree_Node Ecore_Tree_Node;
# define ECORE_TREE_NODE(object) ((Ecore_Tree_Node *)object)
   struct _Ecore_Tree_Node {
      
      /* The actual data for each node */
      void *key;
      void *value;
      
      /* Pointers to surrounding nodes */
      Ecore_Tree_Node *parent;
      Ecore_Tree_Node *left_child;
      Ecore_Tree_Node *right_child;
      
      /* Book keeping information for quicker balancing of the tree */
      int max_right;
      int max_left;
   };
   
   typedef struct _Ecore_Tree Ecore_Tree;
# define ECORE_TREE(object) ((Ecore_Tree *)object)
   struct _Ecore_Tree {
      /* Nodes of the tree */
      Ecore_Tree_Node *tree;
      
      /* Callback for comparing node values, default is direct comparison */
      Ecore_Compare_Cb compare_func;
      
      /* Callback for freeing node data, default is NULL */
      Ecore_Free_Cb free_func;
   };
   
   /* Some basic tree functions */
   /* Allocate and initialize a new tree */
   EAPI Ecore_Tree *ecore_tree_new(Ecore_Compare_Cb compare_func);
   /* Initialize a new tree */
   EAPI int ecore_tree_init(Ecore_Tree * tree, Ecore_Compare_Cb compare_func);
   
   /* Free the tree */
   EAPI int ecore_tree_destroy(Ecore_Tree * tree);
   /* Check to see if the tree has any nodes in it */
   EAPI int ecore_tree_is_empty(Ecore_Tree * tree);
   
   /* Retrieve the value associated with key */
   EAPI void *ecore_tree_get(Ecore_Tree * tree, void *key);
   EAPI Ecore_Tree_Node *ecore_tree_get_node(Ecore_Tree * tree, void *key);
   /* Retrieve the value of node with key greater than or equal to key */
   EAPI void *ecore_tree_get_closest_larger(Ecore_Tree * tree, void *key);
   /* Retrieve the value of node with key less than or equal to key */
   EAPI void *ecore_tree_get_closest_smaller(Ecore_Tree * tree, void *key);
   
   /* Set the value associated with key to value */
   EAPI int ecore_tree_set(Ecore_Tree * tree, void *key, void *value);
   /* Remove the key from the tree */
   EAPI int ecore_tree_remove(Ecore_Tree * tree, void *key);
   
   /* Add a node to the tree */
   EAPI int ecore_tree_add_node(Ecore_Tree * tree, Ecore_Tree_Node * node);
   /* Remove a node from the tree */
   EAPI int ecore_tree_remove_node(Ecore_Tree * tree, Ecore_Tree_Node * node);
   
   /* For each node in the tree perform the for_each_func function */
   /* For this one pass in the node */
   EAPI int ecore_tree_for_each_node(Ecore_Tree * tree, Ecore_For_Each for_each_func,
				     void *user_data);
   /* And here pass in the node's value */
   EAPI int ecore_tree_for_each_node_value(Ecore_Tree * tree,
					   Ecore_For_Each for_each_func,
					   void *user_data);
   
   /* Some basic node functions */
   /* Initialize a node */
   EAPI int ecore_tree_node_init(Ecore_Tree_Node * new_node);
   /* Allocate and initialize a new node */
   EAPI Ecore_Tree_Node *ecore_tree_node_new(void);
   /* Free the desired node */
   EAPI int ecore_tree_node_destroy(Ecore_Tree_Node * node, Ecore_Free_Cb free_data);
   
   /* Set the node's key to key */
   EAPI int ecore_tree_node_key_set(Ecore_Tree_Node * node, void *key);
   /* Retrieve the key in node */
   EAPI void *ecore_tree_node_key_get(Ecore_Tree_Node * node);
   
   /* Set the node's value to value */
   EAPI int ecore_tree_node_value_set(Ecore_Tree_Node * node, void *value);
   /* Retrieve the value in node */
   EAPI void *ecore_tree_node_value_get(Ecore_Tree_Node * node);
   
   /* Add a function to free the data stored in nodes */
   EAPI int ecore_tree_set_free_cb(Ecore_Tree * tree, Ecore_Free_Cb free_func);

#ifdef __cplusplus
}
#endif
#endif				/* _ECORE_DATA_H */
