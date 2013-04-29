/**
 * @enum Elm_Toolbar_Shrink_Mode
 * @typedef Elm_Toolbar_Shrink_Mode
 *
 * Set toolbar's items display behavior, it can be scrollable,
 * show a menu with exceeding items, or simply hide them.
 *
 * @note Default value is #ELM_TOOLBAR_SHRINK_MENU. It reads value
 * from elm config.
 *
 * Values <b> don't </b> work as bitmask, only one can be chosen.
 *
 * @see elm_toolbar_shrink_mode_set()
 * @see elm_toolbar_shrink_mode_get()
 *
 * @ingroup Toolbar
 */
typedef enum
{
   ELM_TOOLBAR_SHRINK_NONE, /**< Set toolbar minimum size to fit all the items. */
   ELM_TOOLBAR_SHRINK_HIDE, /**< Hide exceeding items. */
   ELM_TOOLBAR_SHRINK_SCROLL, /**< Allow accessing exceeding items through a scroller. */
   ELM_TOOLBAR_SHRINK_MENU, /**< Inserts a button to pop up a menu with exceeding items. */
   ELM_TOOLBAR_SHRINK_EXPAND, /**< Expand all items according the size of the toolbar. */
   ELM_TOOLBAR_SHRINK_LAST /**< Indicates error if returned by elm_toolbar_shrink_mode_get() */
} Elm_Toolbar_Shrink_Mode;

/**
 * Defines where to position the item in the toolbar.
 *
 * @ingroup Toolbar
 */
typedef enum
{
   ELM_TOOLBAR_ITEM_SCROLLTO_NONE = 0,   /**< no scrollto */
   ELM_TOOLBAR_ITEM_SCROLLTO_IN = (1 << 0),   /**< to the nearest viewport */
   ELM_TOOLBAR_ITEM_SCROLLTO_FIRST = (1 << 1),   /**< to the first of viewport */
   ELM_TOOLBAR_ITEM_SCROLLTO_MIDDLE = (1 << 2),   /**< to the middle of viewport */
   ELM_TOOLBAR_ITEM_SCROLLTO_LAST = (1 << 3)   /**< to the last of viewport */
} Elm_Toolbar_Item_Scrollto_Type;

typedef struct _Elm_Toolbar_Item_State Elm_Toolbar_Item_State;    /**< State of a Elm_Toolbar_Item. Can be created with elm_toolbar_item_state_add() and removed with elm_toolbar_item_state_del(). */

/**
 * Get the item after @p item in toolbar.
 *
 * @param it The toolbar item.
 * @return The item after @p item, or @c NULL if none or on failure.
 *
 * @note If it is the last item, @c NULL will be returned.
 *
 * @see elm_toolbar_item_append()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_item_next_get(const Elm_Object_Item *it);

/**
 * Get the item before @p item in toolbar.
 *
 * @param it The toolbar item.
 * @return The item before @p item, or @c NULL if none or on failure.
 *
 * @note If it is the first item, @c NULL will be returned.
 *
 * @see elm_toolbar_item_prepend()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_item_prev_get(const Elm_Object_Item *it);

/**
 * Set the priority of a toolbar item.
 *
 * @param it The toolbar item.
 * @param priority The item priority. The default is zero.
 *
 * This is used only when the toolbar shrink mode is set
 * to #ELM_TOOLBAR_SHRINK_MENU or #ELM_TOOLBAR_SHRINK_HIDE.
 * When space is less than required, items with low priority
 * will be removed from the toolbar and added to a dynamically-created menu,
 * while items with higher priority will remain on the toolbar,
 * with the same order they were added.
 *
 * @see elm_toolbar_item_priority_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_item_priority_set(Elm_Object_Item *it, int priority);

/**
 * Get the priority of a toolbar item.
 *
 * @param it The toolbar item.
 * @return The @p item priority, or @c 0 on failure.
 *
 * @see elm_toolbar_item_priority_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI int                          elm_toolbar_item_priority_get(const Elm_Object_Item *it);

/**
 * Get whether the @p item is selected or not.
 *
 * @param it The toolbar item.
 * @return @c EINA_TRUE means item is selected. @c EINA_FALSE indicates
 * it's not. If @p obj is @c NULL, @c EINA_FALSE is returned.
 *
 * @see elm_toolbar_selected_item_set() for details.
 * @see elm_toolbar_item_selected_get()
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool                    elm_toolbar_item_selected_get(const Elm_Object_Item *it);

/**
 * Set the selected state of an item.
 *
 * @param it The toolbar item
 * @param selected The selected state
 *
 * This sets the selected state of the given item @p it.
 * @c EINA_TRUE for selected, @c EINA_FALSE for not selected.
 *
 * If a new item is selected the previously selected will be unselected.
 * Previously selected item can be get with function
 * elm_toolbar_selected_item_get().
 *
 * Selected items will be highlighted.
 *
 * @see elm_toolbar_item_selected_get()
 * @see elm_toolbar_selected_item_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

/**
 * Set the icon associated with @p item.
 *
 * @param it The toolbar item.
 * @param icon A string with icon name or the absolute path of an image file.
 *
 * Toolbar will load icon image from fdo or current theme.
 * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
 * If an absolute path is provided it will load it direct from a file.
 *
 * @note This function does not accept relative icon path.
 *
 * @see elm_toolbar_icon_order_lookup_set()
 * @see elm_toolbar_icon_order_lookup_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_item_icon_set(Elm_Object_Item *it, const char *icon);

/**
 * Get the string used to set the icon of @p item.
 *
 * @param it The toolbar item.
 * @return The string associated with the icon object.
 *
 * @see elm_toolbar_item_icon_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI const char                  *elm_toolbar_item_icon_get(const Elm_Object_Item *it);

/**
 * Get the real Evas(Edje) object created to implement the view of a given
 * toolbar @p item.
 *
 * @param it The toolbar item.
 * @return The base Edje object associated with @p it.
 *
 * @ingroup Toolbar
 */
EAPI Evas_Object                 *elm_toolbar_item_object_get(const Elm_Object_Item *it);

/**
 * Get the icon object of @p item.
 *
 * @param it The toolbar item.
 * @return The icon object
 *
 * @see elm_toolbar_item_icon_set(), elm_toolbar_item_icon_file_set(),
 * or elm_toolbar_item_icon_memfile_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI Evas_Object                 *elm_toolbar_item_icon_object_get(Elm_Object_Item *it);

/**
 * Set the icon associated with @p item to an image in a binary buffer.
 *
 * @param it The toolbar item.
 * @param img The binary data that will be used as an image
 * @param size The size of binary data @p img
 * @param format Optional format of @p img to pass to the image loader
 * @param key Optional key of @p img to pass to the image loader (eg. if @p img is an edje file)
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @note The icon image set by this function can be changed by
 * elm_toolbar_item_icon_set().
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool                    elm_toolbar_item_icon_memfile_set(Elm_Object_Item *it, const void *img, size_t size, const char *format, const char *key);

/**
 * Set the icon associated with @p item to an image in a binary buffer.
 *
 * @param it The toolbar item.
 * @param file The file that contains the image
 * @param key Optional key of @p img to pass to the image loader (eg. if @p img is an edje file)
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @note The icon image set by this function can be changed by
 * elm_toolbar_item_icon_set().
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool                    elm_toolbar_item_icon_file_set(Elm_Object_Item *it, const char *file, const char *key);

/**
 * Set or unset item as a separator.
 *
 * @param it The toolbar item.
 * @param separator @c EINA_TRUE to set item @p item as separator or
 * @c EINA_FALSE to unset, i.e., item will be used as a regular item.
 *
 * Items aren't set as separator by default.
 *
 * If set as separator it will display separator theme, so won't display
 * icons or label.
 *
 * @see elm_toolbar_item_separator_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_item_separator_set(Elm_Object_Item *it, Eina_Bool separator);

/**
 * Get a value whether item is a separator or not.
 *
 * @param it The toolbar item.
 * @return @c EINA_TRUE means item @p it is a separator. @c EINA_FALSE
 * indicates it's not. If @p it is @c NULL, @c EINA_FALSE is returned.
 *
 * @see elm_toolbar_item_separator_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool                    elm_toolbar_item_separator_get(const Elm_Object_Item *it);

/**
 * Set whether the toolbar item opens a menu.
 *
 * @param it The toolbar item.
 * @param menu If @c EINA_TRUE, @p item will opens a menu when selected.
 *
 * A toolbar item can be set to be a menu, using this function.
 *
 * Once it is set to be a menu, it can be manipulated through the
 * menu-like function elm_toolbar_menu_parent_set() and the other
 * elm_menu functions, using the Evas_Object @c menu returned by
 * elm_toolbar_item_menu_get().
 *
 * So, items to be displayed in this item's menu should be added with
 * elm_menu_item_add().
 *
 * The following code exemplifies the most basic usage:
 * @code
 * tb = elm_toolbar_add(win)
 * item = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
 * elm_toolbar_item_menu_set(item, EINA_TRUE);
 * elm_toolbar_menu_parent_set(tb, win);
 * menu = elm_toolbar_item_menu_get(item);
 * elm_menu_item_add(menu, NULL, "edit-cut", "Cut", NULL, NULL);
 * menu_item = elm_menu_item_add(menu, NULL, "edit-copy", "Copy", NULL,
 * NULL);
 * @endcode
 *
 * @see elm_toolbar_item_menu_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_item_menu_set(Elm_Object_Item *it, Eina_Bool menu);

/**
 * Get toolbar item's menu.
 *
 * @param it The toolbar item.
 * @return Item's menu object or @c NULL on failure.
 *
 * If @p item wasn't set as menu item with elm_toolbar_item_menu_set(),
 * this function will set it.
 *
 * @see elm_toolbar_item_menu_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI Evas_Object                 *elm_toolbar_item_menu_get(const Elm_Object_Item *it);

/**
 * Add a new state to @p item.
 *
 * @param it The toolbar item.
 * @param icon A string with icon name or the absolute path of an image file.
 * @param label The label of the new state.
 * @param func The function to call when the item is clicked when this
 * state is selected.
 * @param data The data to associate with the state.
 * @return The toolbar item state, or @c NULL upon failure.
 *
 * Toolbar will load icon image from fdo or current theme.
 * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
 * If an absolute path is provided it will load it direct from a file.
 *
 * States created with this function can be removed with
 * elm_toolbar_item_state_del().
 *
 * @see elm_toolbar_item_state_del()
 * @see elm_toolbar_item_state_sel()
 * @see elm_toolbar_item_state_get()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Item_State      *elm_toolbar_item_state_add(Elm_Object_Item *it, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * Delete a previously added state to @p item.
 *
 * @param it The toolbar item.
 * @param state The state to be deleted.
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure.
 *
 * @see elm_toolbar_item_state_add()
 */
EAPI Eina_Bool                    elm_toolbar_item_state_del(Elm_Object_Item *it, Elm_Toolbar_Item_State *state);

/**
 * Set @p state as the current state of @p it.
 *
 * @param it The toolbar item.
 * @param state The state to use.
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure.
 *
 * If @p state is @c NULL, it won't select any state and the default item's
 * icon and label will be used. It's the same behaviour than
 * elm_toolbar_item_state_unset().
 *
 * @see elm_toolbar_item_state_unset()
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool                    elm_toolbar_item_state_set(Elm_Object_Item *it, Elm_Toolbar_Item_State *state);

/**
 * Unset the state of @p it.
 *
 * @param it The toolbar item.
 *
 * The default icon and label from this item will be displayed.
 *
 * @see elm_toolbar_item_state_set() for more details.
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_item_state_unset(Elm_Object_Item *it);

/**
 * Get the current state of @p it.
 *
 * @param it The toolbar item.
 * @return The selected state or @c NULL if none is selected or on failure.
 *
 * @see elm_toolbar_item_state_set() for details.
 * @see elm_toolbar_item_state_unset()
 * @see elm_toolbar_item_state_add()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Item_State      *elm_toolbar_item_state_get(const Elm_Object_Item *it);

/**
 * Get the state after selected state in toolbar's @p item.
 *
 * @param it The toolbar item to change state.
 * @return The state after current state, or @c NULL on failure.
 *
 * If last state is selected, this function will return first state.
 *
 * @see elm_toolbar_item_state_set()
 * @see elm_toolbar_item_state_add()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Item_State      *elm_toolbar_item_state_next(Elm_Object_Item *it);

/**
 * Get the state before selected state in toolbar's @p item.
 *
 * @param it The toolbar item to change state.
 * @return The state before current state, or @c NULL on failure.
 *
 * If first state is selected, this function will return last state.
 *
 * @see elm_toolbar_item_state_set()
 * @see elm_toolbar_item_state_add()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Item_State      *elm_toolbar_item_state_prev(Elm_Object_Item *it);

/**
 * Show a specific item, when the toolbar can be scrolled.
 *
 * @param it the toolbar item
 *
 * see elm_toolbar_item_bring_in()
 *
 * @since 1.8
 * @ingroup Toolbar
 */
EAPI void                          elm_toolbar_item_show(Elm_Object_Item *it, Elm_Toolbar_Item_Scrollto_Type type);

/**
 * Show a specific item with scroll animation, when the toolbar can be scrolled.
 *
 * @param it the toolbar item
 *
 * see elm_toolbar_item_show()
 *
 * @since 1.8
 * @ingroup Toolbar
 */
EAPI void                          elm_toolbar_item_bring_in(Elm_Object_Item *it, Elm_Toolbar_Item_Scrollto_Type type);

