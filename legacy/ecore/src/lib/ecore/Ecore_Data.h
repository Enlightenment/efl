#ifndef _ECORE_DATA_H
# define _ECORE_DATA_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_BUILD */
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
#endif /* ! _WIN32 */

/* we need this for size_t */
#include <stddef.h>

#include <Eina.h>

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
   
# define ECORE_SORT_MIN 0
# define ECORE_SORT_MAX 1

   typedef void (*Ecore_For_Each) (void *value, void *user_data);
# define ECORE_FOR_EACH(function) ((Ecore_For_Each)function)
   
   typedef void (*Ecore_Free_Cb) (void *data);
# define ECORE_FREE_CB(func) ((Ecore_Free_Cb)func)
   
   typedef unsigned int (*Ecore_Hash_Cb) (const void *key);
# define ECORE_HASH_CB(function) ((Ecore_Hash_Cb)function)
   
   typedef int (*Ecore_Compare_Cb) (const void *data1, const void *data2);
# define ECORE_COMPARE_CB(function) ((Ecore_Compare_Cb)function)
   
   typedef struct _ecore_list Ecore_List;
# define ECORE_LIST(list) ((Ecore_List *)list)
   
   typedef struct _ecore_list_node Ecore_List_Node;
# define ECORE_LIST_NODE(node) ((Ecore_List_Node *)node)

   typedef struct _ecore_strbuf Ecore_Strbuf;
# define ECORE_STRBUF(buf) ((Ecore_Strbuf *)buf)
   
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
   
   EAPI int ecore_direct_compare(const void *key1, const void *key2);
   EAPI int ecore_str_compare(const void *key1, const void *key2);
   
   EAPI unsigned int ecore_direct_hash(const void *key);
   EAPI unsigned int ecore_str_hash(const void *key);
   
   /* Creating and initializing new list structures */
   EAPI Ecore_List *ecore_list_new(void);
   EAPI int ecore_list_init(Ecore_List *list);
   
   /* Adding items to the list */
   EAPI int ecore_list_append(Ecore_List * list, void *_data);
   EAPI int ecore_list_prepend(Ecore_List * list, void *_data);
   EAPI int ecore_list_insert(Ecore_List * list, void *_data);
   EAPI int ecore_list_append_list(Ecore_List * list, Ecore_List * append);
   EAPI int ecore_list_prepend_list(Ecore_List * list, Ecore_List * prepend);
   
   /* Removing items from the list */
   EAPI int ecore_list_remove_destroy(Ecore_List *list);
   EAPI void *ecore_list_remove(Ecore_List * list);
   EAPI void *ecore_list_first_remove(Ecore_List * list);
   EAPI void *ecore_list_last_remove(Ecore_List * list);
   
   /* Retrieve the current position in the list */
   EAPI void *ecore_list_current(Ecore_List * list);
   EAPI void *ecore_list_first(Ecore_List * list);
   EAPI void *ecore_list_last(Ecore_List * list);
   EAPI int ecore_list_index(Ecore_List * list);
   EAPI int ecore_list_count(Ecore_List * list);
   
   /* Traversing the list */
   EAPI int ecore_list_for_each(Ecore_List *list, Ecore_For_Each function,
				void *user_data);
   EAPI void *ecore_list_first_goto(Ecore_List * list);
   EAPI void *ecore_list_last_goto(Ecore_List * list);
   EAPI void *ecore_list_index_goto(Ecore_List * list, int index);
   EAPI void *ecore_list_goto(Ecore_List * list, const void *_data);
   
   /* Traversing the list and returning data */
   EAPI void *ecore_list_next(Ecore_List * list);
   EAPI void *ecore_list_find(Ecore_List *list, Ecore_Compare_Cb function,
        const void *user_data);

   /* Sorting the list */
   EAPI int ecore_list_sort(Ecore_List *list, Ecore_Compare_Cb compare,
                                  char order);
   EAPI int ecore_list_mergesort(Ecore_List *list, Ecore_Compare_Cb compare,
                                  char order);
   EAPI int ecore_list_heapsort(Ecore_List *list, Ecore_Compare_Cb compare,
                                  char order);
   EAPI void ecore_list_merge(Ecore_List *list, Ecore_List *l2, 
                                  Ecore_Compare_Cb, char order);
   
   /* Check to see if there is any data in the list */
   EAPI int ecore_list_empty_is(Ecore_List * list);
   
   /* Remove every node in the list without freeing the list itself */
   EAPI int ecore_list_clear(Ecore_List * list);
   /* Free the list and it's contents */
   EAPI void ecore_list_destroy(Ecore_List *list);
   
   /* Creating and initializing list nodes */
   EAPI Ecore_List_Node *ecore_list_node_new(void);
   EAPI int ecore_list_node_init(Ecore_List_Node *newNode);
   
   /* Destroying nodes */
   EAPI int ecore_list_node_destroy(Ecore_List_Node * _e_node, Ecore_Free_Cb free_func);
   
   EAPI int ecore_list_free_cb_set(Ecore_List * list, Ecore_Free_Cb free_func);
   
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
   EAPI int ecore_dlist_append_list(Ecore_DList * _e_dlist, Ecore_DList * append);
   EAPI int ecore_dlist_prepend_list(Ecore_DList * _e_dlist, Ecore_DList * prepend);
   
   /* Info about list's state */
# define ecore_dlist_first(list) ecore_list_first(list)
# define ecore_dlist_last(list) ecore_list_last(list)
   EAPI void *ecore_dlist_current(Ecore_DList *list);
   EAPI int ecore_dlist_index(Ecore_DList *list);
# define ecore_dlist_count(list) ecore_list_count(list)
   
   /* Removing items from the list */
   EAPI void *ecore_dlist_remove(Ecore_DList * _e_dlist);
   EAPI void *ecore_dlist_first_remove(Ecore_DList * _e_dlist);
   EAPI int ecore_dlist_remove_destroy(Ecore_DList *list);
   EAPI void *ecore_dlist_last_remove(Ecore_DList * _e_dlist);
   
   /* Traversing the list */
# define ecore_dlist_for_each(list, function, user_data) \
   ecore_list_for_each(list, function, user_data)
   EAPI void *ecore_dlist_first_goto(Ecore_DList * _e_dlist);
   EAPI void *ecore_dlist_last_goto(Ecore_DList * _e_dlist);
   EAPI void *ecore_dlist_index_goto(Ecore_DList * _e_dlist, int index);
   EAPI void *ecore_dlist_goto(Ecore_DList * _e_dlist, void *_data);
   
   /* Traversing the list and returning data */
   EAPI void *ecore_dlist_next(Ecore_DList * list);
   EAPI void *ecore_dlist_previous(Ecore_DList * list);
   
   /* Sorting the list */
   EAPI int ecore_dlist_sort(Ecore_DList *list, Ecore_Compare_Cb compare,
                                  char order);
   EAPI int ecore_dlist_mergesort(Ecore_DList *list, Ecore_Compare_Cb compare,
                                  char order);
# define ecore_dlist_heapsort(list, compare, order) \
   ecore_list_heapsort(list, compare, order)
   EAPI void ecore_dlist_merge(Ecore_DList *list, Ecore_DList *l2, 
                                  Ecore_Compare_Cb, char order);
   
   /* Check to see if there is any data in the list */
   EAPI int ecore_dlist_empty_is(Ecore_DList * _e_dlist);
   
   /* Remove every node in the list without free'ing it */
   EAPI int ecore_dlist_clear(Ecore_DList * _e_dlist);
   
   /* Creating and initializing list nodes */
   EAPI int ecore_dlist_node_init(Ecore_DList_Node * node);
   EAPI Ecore_DList_Node *ecore_dlist_node_new(void);
   
   /* Destroying nodes */
   EAPI int ecore_dlist_node_destroy(Ecore_DList_Node * node, Ecore_Free_Cb free_func);
   
   EAPI int ecore_dlist_free_cb_set(Ecore_DList * dlist, Ecore_Free_Cb free_func);
   
   
   
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
      Ecore_Hash_Cb hash_func;	/* The callback function to determine hash */
      
      Ecore_Free_Cb free_key;	/* The callback function to free key */
      Ecore_Free_Cb free_value;	/* The callback function to free value */
   };
   
   /* Create and initialize a hash */
   EAPI Ecore_Hash *ecore_hash_new(Ecore_Hash_Cb hash_func, Ecore_Compare_Cb compare);
   EAPI int ecore_hash_init(Ecore_Hash *hash, Ecore_Hash_Cb hash_func, Ecore_Compare_Cb compare);
   
   /* Functions related to freeing the data in the hash table */
   EAPI int ecore_hash_free_key_cb_set(Ecore_Hash *hash, Ecore_Free_Cb function);
   EAPI int ecore_hash_free_value_cb_set(Ecore_Hash *hash, Ecore_Free_Cb function);
   EAPI void ecore_hash_destroy(Ecore_Hash *hash);

   EAPI int ecore_hash_count(Ecore_Hash *hash);
   EAPI int ecore_hash_for_each_node(Ecore_Hash *hash, Ecore_For_Each for_each_func,
				     void *user_data);
   EAPI Ecore_List *ecore_hash_keys(Ecore_Hash *hash);
   
   /* Retrieve and store data into the hash */
   EAPI void *ecore_hash_get(Ecore_Hash *hash, const void *key);
   EAPI int ecore_hash_set(Ecore_Hash *hash, void *key, void *value);
   EAPI int ecore_hash_hash_set(Ecore_Hash *hash, Ecore_Hash *set);
   EAPI void *ecore_hash_remove(Ecore_Hash *hash, const void *key);
   EAPI void *ecore_hash_find(Ecore_Hash *hash, Ecore_Compare_Cb compare, const void *value);
   EAPI void ecore_hash_dump_graph(Ecore_Hash *hash);
   EAPI void ecore_hash_dump_stats(Ecore_Hash *hash);


   typedef struct _ecore_path_group Ecore_Path_Group;
# define ECORE_PATH_GROUP(group) ((Ecore_Path_Group *)(group))

   struct _ecore_path_group
     {
	Eina_List *paths;
     };
   
   /*
    * Create a new path group
    */
   EAPI Ecore_Path_Group *ecore_path_group_new(void);
   
   /*
    * Destroy a previous path group
    */
   EAPI void ecore_path_group_del(Ecore_Path_Group *group);
   
   /*
    * Add a directory to be searched for files
    */
   EAPI void ecore_path_group_add(Ecore_Path_Group *group, const char *path);
   
   /*
    * Remove a directory to be searched for files
    */
   EAPI void ecore_path_group_remove(Ecore_Path_Group *group, const char *path);
   
   /*
    * Find the absolute path if it exists in the group of paths
    */
   EAPI char * ecore_path_group_find(Ecore_Path_Group *group, const char *name);
   
   /*
    * Get a list of all the available files in a path set
    */
   EAPI Eina_List * ecore_path_group_available_get(Ecore_Path_Group *group);
   
   
   typedef struct _ecore_plugin Ecore_Plugin;
   struct _ecore_plugin
     {
	void *handle;
     };
   
   /*
    * Load the specified plugin
    */
   EAPI Ecore_Plugin *ecore_plugin_load(Ecore_Path_Group *group, const char *plugin, const char *version);
   
   /*
    * Unload the specified plugin
    */
   EAPI void ecore_plugin_unload(Ecore_Plugin * plugin);
   
   /*
    * Lookup the specified symbol for the plugin
    */
   EAPI void *ecore_plugin_symbol_get(Ecore_Plugin * plugin, const char *symbol_name);

   EAPI Eina_List *ecore_plugin_available_get(Ecore_Path_Group *group);


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
   EAPI int ecore_sheap_free_cb_set(Ecore_Sheap *heap, Ecore_Free_Cb free_func);
   EAPI int ecore_sheap_insert(Ecore_Sheap *heap, void *data);
   EAPI void *ecore_sheap_extract(Ecore_Sheap *heap);
   EAPI void *ecore_sheap_extreme(Ecore_Sheap *heap);
   EAPI int ecore_sheap_change(Ecore_Sheap *heap, void *item, void *newval);
   EAPI int ecore_sheap_compare_set(Ecore_Sheap *heap, Ecore_Compare_Cb compare);
   EAPI void ecore_sheap_order_set(Ecore_Sheap *heap, char order);
   EAPI void ecore_sheap_sort(Ecore_Sheap *heap);
   
   EAPI void *ecore_sheap_item(Ecore_Sheap *heap, int i);
   
  #define ecore_string_init eina_stringshare_init
  #define ecore_string_shutdown eina_stringshare_shutdown
  #define ecore_string_instance eina_stringshare_add
  #define ecore_string_release eina_stringshare_del

  #define ecore_string_hash_dump_graph (void)0
  #define ecore_string_hash_dump_stats (void)0
   
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
      Ecore_Free_Cb free_value;
      /* Callback for freeing node key, default is NULL */
      Ecore_Free_Cb free_key;
   };
   
   /* Some basic tree functions */
   /* Allocate and initialize a new tree */
   EAPI Ecore_Tree *ecore_tree_new(Ecore_Compare_Cb compare_func);
   /* Initialize a new tree */
   EAPI int ecore_tree_init(Ecore_Tree * tree, Ecore_Compare_Cb compare_func);
   
   /* Free the tree */
   EAPI int ecore_tree_destroy(Ecore_Tree * tree);
   /* Check to see if the tree has any nodes in it */
   EAPI int ecore_tree_empty_is(Ecore_Tree * tree);
   
   /* Retrieve the value associated with key */
   EAPI void *ecore_tree_get(Ecore_Tree * tree, const void *key);
   EAPI Ecore_Tree_Node *ecore_tree_get_node(Ecore_Tree * tree, const void *key);
   /* Retrieve the value of node with key greater than or equal to key */
   EAPI void *ecore_tree_closest_larger_get(Ecore_Tree * tree, const void *key);
   /* Retrieve the value of node with key less than or equal to key */
   EAPI void *ecore_tree_closest_smaller_get(Ecore_Tree * tree, const void *key);
   
   /* Set the value associated with key to value */
   EAPI int ecore_tree_set(Ecore_Tree * tree, void *key, void *value);
   /* Remove the key from the tree */
   EAPI int ecore_tree_remove(Ecore_Tree * tree, const void *key);
   
   /* Add a node to the tree */
   EAPI int ecore_tree_node_add(Ecore_Tree * tree, Ecore_Tree_Node * node);
   /* Remove a node from the tree */
   EAPI int ecore_tree_node_remove(Ecore_Tree * tree, Ecore_Tree_Node * node);
   
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
   EAPI int ecore_tree_node_destroy(Ecore_Tree_Node * node, 
		   Ecore_Free_Cb free_value, Ecore_Free_Cb free_key);
   
   /* Set the node's key to key */
   EAPI int ecore_tree_node_key_set(Ecore_Tree_Node * node, void *key);
   /* Retrieve the key in node */
   EAPI void *ecore_tree_node_key_get(Ecore_Tree_Node * node);
   
   /* Set the node's value to value */
   EAPI int ecore_tree_node_value_set(Ecore_Tree_Node * node, void *value);
   /* Retrieve the value in node */
   EAPI void *ecore_tree_node_value_get(Ecore_Tree_Node * node);
   
   /* Add a function to free the data stored in nodes */
   EAPI int ecore_tree_free_value_cb_set(Ecore_Tree * tree, Ecore_Free_Cb free_value);
   /* Add a function to free the keys stored in nodes */
   EAPI int ecore_tree_free_key_cb_set(Ecore_Tree * tree, Ecore_Free_Cb free_key);


   EAPI Ecore_Strbuf * ecore_strbuf_new(void);
   EAPI void ecore_strbuf_free(Ecore_Strbuf *buf);
   EAPI void ecore_strbuf_append(Ecore_Strbuf *buf, const char *str);
   EAPI void ecore_strbuf_append_char(Ecore_Strbuf *buf, char c);
   EAPI void ecore_strbuf_insert(Ecore_Strbuf *buf, const char *str, 
                                 size_t pos);
# define ecore_strbuf_prepend(buf, str) ecore_strbuf_insert(buf, str, 0)
   EAPI const char * ecore_strbuf_string_get(Ecore_Strbuf *buf);
   EAPI size_t ecore_strbuf_length_get(Ecore_Strbuf *buf);
   EAPI int ecore_strbuf_replace(Ecore_Strbuf *buf, const char *str, 
                                 const char *with, unsigned int n);
# define ecore_strbuf_replace_first(buf, str, with) \
	ecore_strbuf_replace(buf, str, with, 1)
   EAPI int ecore_strbuf_replace_all(Ecore_Strbuf *buf, const char *str,
                                     const char *with);

#ifdef __cplusplus
}
#endif
#endif				/* _ECORE_DATA_H */
