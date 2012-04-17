/**
 * @defgroup Store Elementary Store
 * @ingroup Elementary
 *
 * Store is an abstracting API that is intended to farm off fetching of data
 * to threads running asynchronously from the mainloop that actually fetch
 * data needed for a genlist (or possibly future other widgets) so scrolling
 * never blocks waiting on IO (though normally this should be the users
 * job - if using genlist, to ensure all data genlist needs is in memory at
 * the time it needs it, and if it isn't to queue and defer a fetch and let
 * genlist know later when its ready. Store actually does this and implements
 * the infrastructure of this, leaving the actual fetch and convert up to
 * functions provided by the user).
 *
 * It is possible for store to run inline without a thread, but this is
 * highly inadvisable. you can disable this with:
 *
 * elm_store_fetch_thread_set(store, EINA_FALSE);
 *
 * Store works first by creating a store, setting up functions to list items
 * and fetch items. Currently the only store type supported is the
 * filesystem store, which will list the files inside a directory (not
 * recursively) and then hand each file it finds (the file path) to the
 * list function for evaluation.
 *
 * The list function may look at filename, may open the file or do
 * anything it likes to determine something about the file. Either it
 * filters it out (returns EINA_FALSE) and it is discarded or it
 * returns EINA_TRUE and also provides a "sort id" which is a string
 * store uses to figure out sorting. This string could be the filename, or
 * some data based on its contents. The strings are sorted alphabetically
 * like any normal ASCII strings, with case being important. As this listing
 * function runs in a thread, it can do blocking IO and parsing without
 * hurting the fluidity of the main loop and GUI. The list function also
 * returns information on how to map fields in the source file to elements
 * of the genlist item. For example, how the fetcher reads the private
 * data struct of the user (what memory offset in the struct the data is at)
 * and what type is there (it's a label of some sort, an icon, or with a
 * custom mapping function that figures it out itself and creates the
 * content needed for the genlist item).
 *
 * Store then uses this sort id to build (over time) a sorted list of items
 * that then map 1:1 to genlist items. When these items are visible and
 * need content, Store calls the fetch function per item, which is responsible
 * for fetching the data from the given item and returning data to store
 * so it can map this to some item content. This function also runs in a
 * thread, and thus can do blocking IO work to later return the data. Sorting
 * is optional and can be enabled or disabled too.
 *
 * When items are no longer needed, store will cal the unfetch function to
 * free data in memory about that item that is no longer needed. This function
 * is called in the mainloop and is expected to take minimal or almost no time
 * to simply free up memory resources.
 *
 * @{
 */

typedef struct _Elm_Store                      Elm_Store; /**< A store object */
typedef struct _Elm_Store_Item                 Elm_Store_Item; /**< A handle of a store item passed to store fetch/unfetch functions */
typedef struct _Elm_Store_Item_Info            Elm_Store_Item_Info; /**< Basic information about a store item - always cast into a specific type like Elm_Store_Item_Info_Filesystem */
typedef struct _Elm_Store_Item_Info_Filesystem Elm_Store_Item_Info_Filesystem; /**< Filesystem specific information about a store item */
typedef struct _Elm_Store_Item_Mapping         Elm_Store_Item_Mapping; /**< A basic way of telling Store how to take your return data (string, or something else from your struct) and convert it into something genlist can use */
typedef struct _Elm_Store_Item_Mapping_Empty   Elm_Store_Item_Mapping_Empty; /**< An empty piece of mapping information. Useful for String labels as they get used directly */
typedef struct _Elm_Store_Item_Mapping_Icon    Elm_Store_Item_Mapping_Icon; /***< The data being mapped at the given address is an icon, so use these properties for finding it */
typedef struct _Elm_Store_Item_Mapping_Photo   Elm_Store_Item_Mapping_Photo; /**< The data is a photo, so use these parameters to find it */
typedef struct _Elm_Store_Item_Mapping_Custom  Elm_Store_Item_Mapping_Custom; /**> The item needs a custom mapping which means calling a function and returning a string from it, as opposed to a static lookup. It should not be allocated, and should live in a buffer in memory that survives the return of this function if its a label, or an allocated icon object if its an icon needed etc. */

typedef Eina_Bool                            (*Elm_Store_Item_List_Cb)(void *data, Elm_Store_Item_Info *info); /**< Function to call for listing an item */
typedef void                                 (*Elm_Store_Item_Fetch_Cb)(void *data, Elm_Store_Item *sti); /**< Function to call to fetch item data */
typedef void                                 (*Elm_Store_Item_Unfetch_Cb)(void *data, Elm_Store_Item *sti); /**< Function to cal lto un-fetch (free) an item */
typedef void                                *(*Elm_Store_Item_Mapping_Cb)(void *data, Elm_Store_Item *sti, const char *part); /**< Custom mapping function to call */

typedef enum
{
   ELM_STORE_ITEM_MAPPING_NONE = 0,
   ELM_STORE_ITEM_MAPPING_LABEL, /**< const char * -> label */
   ELM_STORE_ITEM_MAPPING_STATE, /**< Eina_Bool -> state */
   ELM_STORE_ITEM_MAPPING_ICON, /**< char * -> icon path */
   ELM_STORE_ITEM_MAPPING_PHOTO, /**< char * -> photo path */
   ELM_STORE_ITEM_MAPPING_CUSTOM, /**< item->custom(it->data, it, part) -> void * (-> any) */
   ELM_STORE_ITEM_MAPPING_LAST
} Elm_Store_Item_Mapping_Type;

struct _Elm_Store_Item_Mapping_Icon
{
   int                   w, h; /**< The desired icon size in addition to the file path returned from the mapping */
   Elm_Icon_Lookup_Order lookup_order; /**< The order in which to find the icon */
   Eina_Bool             standard_name : 1; /**< Use a standard name to find it (EINA_TRUE) or not */
   Eina_Bool             no_scale : 1; /**< EINA_TRUE is you don't want the icon scaled */
   Eina_Bool             smooth : 1; /**< EINA_TRUE if icon is to be smooth scaled */
   Eina_Bool             scale_up : 1; /**< EINA_TRUE if scaling up is allowed */
   Eina_Bool             scale_down : 1; /**< EINA_TRUE if scaling down is allowed */
};

struct _Elm_Store_Item_Mapping_Empty
{
   Eina_Bool dummy; /**< dummy entry - set to anything you like */
};

struct _Elm_Store_Item_Mapping_Photo
{
   int size; /**< Photo size to use (see elm_photo_add()) with the given photo path */
};

struct _Elm_Store_Item_Mapping_Custom
{
   Elm_Store_Item_Mapping_Cb func; /**< The function called to do the custom mapping and return it */
};

struct _Elm_Store_Item_Mapping
{
   Elm_Store_Item_Mapping_Type type; /**< what kind of mapping is this */
   const char                 *part; /**< what part name in the genlist item is this filling in */
   int                         offset; /**< offset in memory (in bytes) relative to base of structure for item data where the data for the mapping lives */
   union
   {
      Elm_Store_Item_Mapping_Empty  empty;
      Elm_Store_Item_Mapping_Icon   icon;
      Elm_Store_Item_Mapping_Photo  photo;
      Elm_Store_Item_Mapping_Custom custom;
      // add more types here
   } details; /**< Allowed to be one of these possible mapping types */
};

struct _Elm_Store_Item_Info
{
   Elm_Genlist_Item_Class       *item_class; /**< The genlist item class that should be used for the item that has been listed */
   const Elm_Store_Item_Mapping *mapping; /**< What kind of mappings do we use for the fields of this item to fill in the genlist item. Terminate array pointed to here with ELM_STORE_ITEM_MAPPING_END */
   void                         *data; /**< Pointer to pass to struct data in memory if its already there, of not, NULL */
   char                         *sort_id; /**< Sort ID string (strduped()) to know how to wort items, or NULL, if you don't care */
};

struct _Elm_Store_Item_Info_Filesystem
{
   Elm_Store_Item_Info base; /**< Base information about an item */
   char               *path; /**< Extra information specific to the filesystem store */
};

#define ELM_STORE_ITEM_MAPPING_END { ELM_STORE_ITEM_MAPPING_NONE, NULL, 0, { .empty = { EINA_TRUE } } } /**< Use this to end a list of mappings */
#define ELM_STORE_ITEM_MAPPING_OFFSET(st, it) offsetof(st, it) /**< Use this to get the offset in bytes in memory for where the data for the mapping lives relative to the item data (a private struct pointed to owned by the user */

/**
 * Create a new store object
 *
 * This creates a new store object to then configure so it works.
 *
 * @return A new store object, or NULL if creation fails
 *
 * @ingroup Store
 */
EAPI Elm_Store              *elm_store_filesystem_new(void);
/**
 * Free the store object and all items it manages
 *
 * This frees the given @p st store and all the items it manages. It will
 * clear the List that it populated, but otherwise leave it alone. It will
 * cancel background threads (and may have to wait for them to complete a
 * pending operation to do this).
 *
 * @param st The store to free
 *
 * @ingroup Store
 */
EAPI void                    elm_store_free(Elm_Store *st);

/**
 * Set the path to the directory to scan for a filesystem store
 *
 * This sets the directory (@p dir) to scan and begins scanning in the
 * the background in threads (or not if threading is disabled with
 * elm_store_fetch_thread_set()). Note that Listing is always done in a thread
 * but fetching may not be if disabled here. This should be the last thing
 * called after fetch, list and unfetch functions are set, as well as target
 * genlist etc. You also should not change the directory once set. If you
 * need a new directory scanned, create a new store.
 *
 * @param st The store to modify
 * @param dir A string giving the path to the directory to scan
 *
 * @ingroup Store
 */
EAPI void                    elm_store_filesystem_directory_set(Elm_Store *st, const char *dir);

/**
 * Get the directory set on a filesystem store
 *
 * This gets the directory set by elm_store_filesystem_directory_set(). This
 * string returned will be valid until elm_store_filesystem_directory_set()
 * changes it or until the store is freed with elm_store_free().
 *
 * @return A string with the path set, or NULL if none set.
 *
 * @ingroup Store
 */
EAPI const char             *elm_store_filesystem_directory_get(const Elm_Store *st);

/**
 * Get the path of a specific store item
 *
 * This returns the full path of a store item. This string is valid only
 * during the list function set by elm_store_list_func_set() or during the
 * fetch function set by elm_store_fetch_func_set() or during the unfetch
 * function set by elm_store_unfetch_func_set().
 *
 * @param sti The store item to get the path from
 * @return A full path in a string or NULL if none available
 *
 * @ingroup Store
 */
EAPI const char             *elm_store_item_filesystem_path_get(const Elm_Store_Item *sti);

/**
 * Set the target genlist to fill in from the store
 *
 * This tells the store the target genlist to use to fill in content from
 * the store. Once a store starts "going" via elm_store_filesystem_directory_set()
 * The target should never be changed again.
 *
 * @param st The store to do the filling.
 * @param obj The genlist object to fill in and control the content of from the store.
 *
 * @ingroup Store
 */
EAPI void                    elm_store_target_genlist_set(Elm_Store *st, Evas_Object *obj);

/**
 * Set the maximum number of items that are not visible to keep cached
 *
 * Store may keep some items around for caching purposes that cannot be seen,
 * so this controls the maximum number. The default is 128, but may change
 * at any point in time in the future.
 *
 * @param st The store to modify
 * @param max The number of items to keep (should be greater than or equal to 0)
 *
 * @ingroup Store
 */
EAPI void                    elm_store_cache_set(Elm_Store *st, int max);

/**
 * Get the maximum number if items to cache
 *
 * This returns the number of items at most to cache.
 *
 * @param st The store to query
 * @return The maximum number of items to cache (>= 0)
 * @see elm_store_cache_set()
 *
 * @ingroup Store
 */
EAPI int                     elm_store_cache_get(const Elm_Store *st);

/**
 * Set the function used to deal with listing of items
 *
 * This function is called per item that is found so it can examine the item
 * and discard it (return EINA_FALSE to discard, or EINA_TRUE to accept), and
 * work out some sorting ID (that may be filename or anything else based on
 * content). This function is always called from a thread.
 *
 * @param st The store to set the function of
 * @param func The function to be called
 * @param data the data pointer to be passed to the @p func function when called
 *
 * @ingroup Store
 */
EAPI void                    elm_store_list_func_set(Elm_Store *st, Elm_Store_Item_List_Cb func, const void *data);

/**
 * Set the function used to deal with fetching of items
 *
 * This function is called per item that needs data to be fetched when it
 * becomes visible and such data is needed. This function is normally run
 * from a thread (unless elm_store_fetch_thread_set() disables this). The
 * fetch function is to read data from the source and fill a structure
 * allocated for this item with fields and then rely on the mapping setup
 * to tell Store how to take a field in the structure and apply it to a
 * genlist item.
 *
 * @param st The store to set the function of
 * @param func The function to be called
 * @param data the data pointer to be passed to the @p func function when called
 *
 * @ingroup Store
 */
EAPI void                    elm_store_fetch_func_set(Elm_Store *st, Elm_Store_Item_Fetch_Cb func, const void *data);

/**
 * Set the function used to free the structure allocated for the item
 *
 * This function is called per item when it is not needed in memory anymore
 * and should free the structure allocated in and filled in the function set
 * by elm_store_fetch_func_set().
 *
 * @param st The store to set the function of
 * @param func The function to be called
 * @param data the data pointer to be passed to the @p func function when called
 *
 * @ingroup Store
 */
EAPI void                    elm_store_unfetch_func_set(Elm_Store *st, Elm_Store_Item_Unfetch_Cb func, const void *data);

/**
 * Enable or disable fetching in a thread for Store
 *
 * @param st The store to modify
 * @param use_thread EINA_TRUE to use a thread to fetch, EINA_FALSE don't use a thread.
 *
 * @ingroup Store
 */
EAPI void                    elm_store_fetch_thread_set(Elm_Store *st, Eina_Bool use_thread);

/**
 * Get the thread enabled fetching option for Store
 *
 * @return The state set currently for the store.
 * @see elm_store_fetch_thread_set()
 *
 * @ingroup Store
 */
EAPI Eina_Bool               elm_store_fetch_thread_get(const Elm_Store *st);

/**
 * Set if items are to be sorted or not.
 *
 * By default items are not sorted, but read "in order" as they are found. If
 * you want to sort, your list function set by elm_store_list_func_set() must
 * provide a sort ID to sort by, and then Store will take care of sorting when
 * it inserts items. You should set this up before you begin listing items
 * in the store and then never change it again.
 *
 * @param st The store to modify
 * @param sorted EINA_TRUE if we are to sort, EINA_FALSE if not.
 *
 * @ingroup Store
 */
EAPI void                    elm_store_sorted_set(Elm_Store *st, Eina_Bool sorted);

/**
 * Get the sorting flag
 *
 * Get the sorted flag as set by elm_store_sorted_set().
 *
 * @param st The store to query
 * @return EINA_TRUE if sorted, EINA_FALSE if not.
 *
 * @ingroup Store
 */
EAPI Eina_Bool               elm_store_sorted_get(const Elm_Store *st);

/**
 * Set the item data holding item fields to map to item values in genlist
 *
 * Once you decode an item, allocate a structure for it and fill the structure,
 * you should set the item data with this function (eg in the fetch function).
 * This item pointer is the base offset to use when mapping fields to item
 * values. Once you unfetch, store will handle NULLing the data pointer for you.
 *
 * @param sti The store item to set the data pointer of
 * @param data The data pointer to set.
 *
 * @ingroup Store
 */
EAPI void                    elm_store_item_data_set(Elm_Store_Item *sti, void *data);

/**
 * Get the item data
 *
 * This gets the data pointer set by elm_store_item_data_set().
 *
 * @param sti The store item to query
 * @return The data pointer set on the item
 *
 * @ingroup Store
 */
EAPI void                   *elm_store_item_data_get(Elm_Store_Item *sti);

/**
 * Fetch the store than a store item belongs to
 *
 * This fetches the store object that owns the store item.
 *
 * @param sti The store item to query
 * @return The store the item belongs to
 *
 * @ingroup Store
 */
EAPI const Elm_Store        *elm_store_item_store_get(const Elm_Store_Item *sti);

/**
 * Fetch the genlist item that this store item controls
 *
 * @param sti The store item to query
 * @return The genlist object item handle controlled by this store item
 *
 * @ingroup Store
 */
EAPI const Elm_Object_Item *elm_store_item_genlist_item_get(const Elm_Store_Item *sti);

/**
 * @}
 */
