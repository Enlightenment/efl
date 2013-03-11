/**
 * @defgroup Toolbar Toolbar
 * @ingroup Elementary
 *
 * @image html toolbar_inheritance_tree.png
 * @image latex toolbar_inheritance_tree.eps
 *
 * @image html img/widget/toolbar/preview-00.png
 * @image latex img/widget/toolbar/preview-00.eps width=\textwidth
 *
 * @image html img/toolbar.png
 * @image latex img/toolbar.eps width=\textwidth
 *
 * A toolbar is a widget that displays a list of items inside
 * a box. It can be scrollable, show a menu with items that don't fit
 * to toolbar size or even crop them.
 *
 * Only one item can be selected at a time.
 *
 * Items can have multiple states, or show menus when selected by the user.
 *
 * This widget implements the @b @ref elm-scrollable-interface
 * interface, so that all (non-deprecated) functions for the base
 * @ref Scroller widget also work for toolbars (@since 1.8)
 *
 * Smart callbacks one can listen to:
 * - @c "clicked" - when the user clicks on a toolbar item and becomes
 *                  selected.
 * - @c "longpressed" - when the toolbar is pressed for a certain
 *                      amount of time.
 * - @c "language,changed" - when the program language changes.
 *
 * Available styles for it:
 * - @c "default"
 * - @c "transparent" - no background or shadow, just show the content
 *
 * Default text parts of the toolbar items that you can use for are:
 * @li "default" - label of the toolbar item
 *
 * Supported elm_object_item common APIs.
 * @li @ref elm_object_item_disabled_set
 * @li @ref elm_object_item_disabled_get
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 *
 * List of examples:
 * @li @ref toolbar_example_01
 * @li @ref toolbar_example_02
 * @li @ref toolbar_example_03
 */

/**
 * @addtogroup Toolbar
 * @{
 */

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
 * Add a new toolbar widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new toolbar widget handle or @c NULL, on errors.
 *
 * This function inserts a new toolbar widget on the canvas.
 *
 * @ingroup Toolbar
 */
EAPI Evas_Object                 *elm_toolbar_add(Evas_Object *parent);

/**
 * Set the icon size, in pixels, to be used by toolbar items.
 *
 * @param obj The toolbar object
 * @param icon_size The icon size in pixels
 *
 * @note Default value is @c 32. It reads value from elm config.
 *
 * @see elm_toolbar_icon_size_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_icon_size_set(Evas_Object *obj, int icon_size);

/**
 * Get the icon size, in pixels, to be used by toolbar items.
 *
 * @param obj The toolbar object.
 * @return The icon size in pixels.
 *
 * @see elm_toolbar_icon_size_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI int                          elm_toolbar_icon_size_get(const Evas_Object *obj);

/**
 * Sets icon lookup order, for toolbar items' icons.
 *
 * @param obj The toolbar object.
 * @param order The icon lookup order.
 *
 * Icons added before calling this function will not be affected.
 * The default lookup order is #ELM_ICON_LOOKUP_THEME_FDO.
 *
 * @see elm_toolbar_icon_order_lookup_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_icon_order_lookup_set(Evas_Object *obj, Elm_Icon_Lookup_Order order);

/**
 * Gets the icon lookup order.
 *
 * @param obj The toolbar object.
 * @return The icon lookup order.
 *
 * @see elm_toolbar_icon_order_lookup_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI Elm_Icon_Lookup_Order        elm_toolbar_icon_order_lookup_get(const Evas_Object *obj);

/**
 * Append item to the toolbar.
 *
 * @param obj The toolbar object.
 * @param icon A string with icon name or the absolute path of an image file.
 * @param label The label of the item.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and appended to the toolbar, i.e., will
 * be set as @b last item.
 *
 * Items created with this method can be deleted with
 * elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * Toolbar will load icon image from fdo or current theme.
 * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
 * If an absolute path is provided it will load it direct from a file.
 *
 * @note This function does not accept relative icon path.
 *
 * @see elm_toolbar_item_icon_set()
 * @see elm_object_item_del()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_item_append(Evas_Object *obj, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * Prepend item to the toolbar.
 *
 * @param obj The toolbar object.
 * @param icon A string with icon name or the absolute path of an image file.
 * @param label The label of the item.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and prepended to the toolbar, i.e., will
 * be set as @b first item.
 *
 * Items created with this method can be deleted with
 * elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * Toolbar will load icon image from fdo or current theme.
 * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
 * If an absolute path is provided it will load it direct from a file.
 *
 * @note This function does not accept relative icon path.
 *
 * @see elm_toolbar_item_icon_set()
 * @see elm_object_item_del()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_item_prepend(Evas_Object *obj, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * Insert a new item into the toolbar object before item @p before.
 *
 * @param obj The toolbar object.
 * @param before The toolbar item to insert before.
 * @param icon A string with icon name or the absolute path of an image file.
 * @param label The label of the item.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and added to the toolbar. Its position in
 * this toolbar will be just before item @p before.
 *
 * Items created with this method can be deleted with
 * elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * Toolbar will load icon image from fdo or current theme.
 * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
 * If an absolute path is provided it will load it direct from a file.
 *
 * @note This function does not accept relative icon path.
 *
 * @see elm_toolbar_item_icon_set()
 * @see elm_object_item_del()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_item_insert_before(Evas_Object *obj, Elm_Object_Item *before, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * Insert a new item into the toolbar object after item @p after.
 *
 * @param obj The toolbar object.
 * @param after The toolbar item to insert after.
 * @param icon A string with icon name or the absolute path of an image file.
 * @param label The label of the item.
 * @param func The function to call when the item is clicked.
 * @param data The data to associate with the item for related callbacks.
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and added to the toolbar. Its position in
 * this toolbar will be just after item @p after.
 *
 * Items created with this method can be deleted with
 * elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user clicks over an unselected item.
 * If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * Toolbar will load icon image from fdo or current theme.
 * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
 * If an absolute path is provided it will load it direct from a file.
 *
 * @note This function does not accept relative icon path.
 *
 * @see elm_toolbar_item_icon_set()
 * @see elm_object_item_del()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_item_insert_after(Evas_Object *obj, Elm_Object_Item *after, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

/**
 * Get the first item in the given toolbar widget's list of
 * items.
 *
 * @param obj The toolbar object
 * @return The first item or @c NULL, if it has no items (and on
 * errors)
 *
 * @see elm_toolbar_item_append()
 * @see elm_toolbar_last_item_get()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_first_item_get(const Evas_Object *obj);

/**
 * Get the last item in the given toolbar widget's list of
 * items.
 *
 * @param obj The toolbar object
 * @return The last item or @c NULL, if it has no items (and on
 * errors)
 *
 * @see elm_toolbar_item_prepend()
 * @see elm_toolbar_first_item_get()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_last_item_get(const Evas_Object *obj);

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
 * Returns a pointer to a toolbar item by its label.
 *
 * @param obj The toolbar object.
 * @param label The label of the item to find.
 *
 * @return The pointer to the toolbar item matching @p label or @c NULL
 * on failure.
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_item_find_by_label(const Evas_Object *obj, const char *label);

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
 * Get the selected item.
 *
 * @param obj The toolbar object.
 * @return The selected toolbar item.
 *
 * The selected item can be unselected with function
 * elm_toolbar_item_selected_set().
 *
 * The selected item always will be highlighted on toolbar.
 *
 * @see elm_toolbar_selected_items_get()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_selected_item_get(const Evas_Object *obj);

/**
 * Get the more item.
 *
 * @param obj The toolbar object.
 * @return The toolbar more item.
 *
 * The more item can be changed with function
 * elm_object_item_text_set() and elm_object_item_content_set.
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Item             *elm_toolbar_more_item_get(const Evas_Object *obj);

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
 * Get the object of @p item.
 *
 * @param it The toolbar item.
 * @return The object
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
 * Set the item displaying mode of a given toolbar widget @p obj.
 *
 * @param obj The toolbar object handle
 * @param shrink_mode Toolbar's items display behavior
 *
 * The toolbar won't scroll under #ELM_TOOLBAR_SHRINK_NONE mode, but
 * it will enforce a minimum size, so that all the items will fit
 * inside it. It won't scroll and won't show the items that don't fit
 * under #ELM_TOOLBAR_SHRINK_HIDE mode. Finally, it'll scroll under
 * #ELM_TOOLBAR_SHRINK_SCROLL mode, and it will create a button to
 * aggregate items which didn't fit with the #ELM_TOOLBAR_SHRINK_MENU
 * mode.
 *
 * @warning This function's behavior will clash with those of
 * elm_scroller_policy_set(), so use either one of them, but not both.
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_shrink_mode_set(Evas_Object *obj, Elm_Toolbar_Shrink_Mode shrink_mode);

/**
 * Get the shrink mode of toolbar @p obj.
 *
 * @param obj The toolbar object.
 * @return Toolbar's items display behavior.
 *
 * @see elm_toolbar_shrink_mode_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Shrink_Mode      elm_toolbar_shrink_mode_get(const Evas_Object *obj);

/**
 * Set the item's transverse expansion of a given toolbar widget @p obj.
 *
 * @param obj The toolbar object.
 * @param transverse_expanded The transverse expansion of the item.
 * (EINA_TRUE = on, EINA_FALSE = off, default = EINA_FALSE)
 *
 * This will expand the transverse length of the item according the transverse length of the toolbar.
 * The default is what the transverse length of the item is set according its min value.
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_transverse_expanded_set(Evas_Object *obj, Eina_Bool transverse_expanded);

/**
 * Get the transverse expansion of toolbar @p obj.
 *
 * @param obj The toolbar object.
 * @return The transverse expansion of the item.
 * (EINA_TRUE = on, EINA_FALSE = off, default = EINA_FALSE)
 *
 * @see elm_toolbar_transverse_expand_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool                    elm_toolbar_transverse_expanded_get(const Evas_Object *obj);

/**
 * Enable/disable homogeneous mode.
 *
 * @param obj The toolbar object
 * @param homogeneous Assume the items within the toolbar are of the
 * same size (EINA_TRUE = on, EINA_FALSE = off). Default is @c EINA_FALSE.
 *
 * This will enable the homogeneous mode where items are of the same size.
 * @see elm_toolbar_homogeneous_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_homogeneous_set(Evas_Object *obj, Eina_Bool homogeneous);

/**
 * Get whether the homogeneous mode is enabled.
 *
 * @param obj The toolbar object.
 * @return Assume the items within the toolbar are of the same height
 * and width (EINA_TRUE = on, EINA_FALSE = off).
 *
 * @see elm_toolbar_homogeneous_set()
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool                    elm_toolbar_homogeneous_get(const Evas_Object *obj);

/**
 * Set the parent object of the toolbar items' menus.
 *
 * @param obj The toolbar object.
 * @param parent The parent of the menu objects.
 *
 * Each item can be set as item menu, with elm_toolbar_item_menu_set().
 *
 * For more details about setting the parent for toolbar menus, see
 * elm_menu_parent_set().
 *
 * @see elm_menu_parent_set() for details.
 * @see elm_toolbar_item_menu_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_menu_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * Get the parent object of the toolbar items' menus.
 *
 * @param obj The toolbar object.
 * @return The parent of the menu objects.
 *
 * @see elm_toolbar_menu_parent_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI Evas_Object                 *elm_toolbar_menu_parent_get(const Evas_Object *obj);

/**
 * Set the alignment of the items.
 *
 * @param obj The toolbar object.
 * @param align The new alignment, a float between <tt> 0.0 </tt>
 * and <tt> 1.0 </tt>.
 *
 * Alignment of toolbar items, from <tt> 0.0 </tt> to indicates to align
 * left, to <tt> 1.0 </tt>, to align to right. <tt> 0.5 </tt> centralize
 * items.
 *
 * Centered items by default.
 *
 * @see elm_toolbar_align_get()
 *
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_align_set(Evas_Object *obj, double align);

/**
 * Get the alignment of the items.
 *
 * @param obj The toolbar object.
 * @return toolbar items alignment, a float between <tt> 0.0 </tt> and
 * <tt> 1.0 </tt>.
 *
 * @see elm_toolbar_align_set() for details.
 *
 * @ingroup Toolbar
 */
EAPI double                       elm_toolbar_align_get(const Evas_Object *obj);

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
 * Change a toolbar's orientation
 * @param obj The toolbar object
 * @param horizontal If @c EINA_TRUE, the toolbar is horizontal
 * By default, a toolbar will be horizontal. Use this function to create a vertical toolbar.
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * Get a toolbar's orientation
 * @param obj The toolbar object
 * @return If @c EINA_TRUE, the toolbar is horizontal
 * By default, a toolbar will be horizontal. Use this function to determine whether a toolbar is vertical.
 * @ingroup Toolbar
 */
EAPI Eina_Bool                    elm_toolbar_horizontal_get(const Evas_Object *obj);

/**
 * Get the number of items in a toolbar
 * @param obj The toolbar object
 * @return The number of items in @p obj toolbar
 * @ingroup Toolbar
 */
EAPI unsigned int                 elm_toolbar_items_count(const Evas_Object *obj);

/**
 * Set the standard priority of visible items in a toolbar
 * @param obj The toolbar object
 * @param priority The standard_priority of visible items
 *
 * If the priority of the item is up to standard priority, it is shown in basic panel.
 * The other items are located in more menu or panel. The more menu or panel can be shown when the more item is clicked.
 *
 * @see elm_toolbar_standard_priority_get()
 *
 * @since 1.7
 * @ingroup Toolbar
 */
EAPI void                         elm_toolbar_standard_priority_set(Evas_Object *obj, int priority);

/**
 * Get the standard_priority of visible items in a toolbar
 * @param obj The toolbar object
 * @return The standard priority of items in @p obj toolbar
 *
 * @see elm_toolbar_standard_priority_set()
 *
 * @since 1.7
 * @ingroup Toolbar
 */
EAPI int                          elm_toolbar_standard_priority_get(const Evas_Object *obj);

/**
 * Set the toolbar select mode.
 *
 * @param obj The toolbar object
 * @param mode The select mode
 *
 * elm_toolbar_select_mode_set() changes item select mode in the toolbar widget.
 * - ELM_OBJECT_SELECT_MODE_DEFAULT : Items will only call their selection func and
 *      callback when first becoming selected. Any further clicks will
 *      do nothing, unless you set always select mode.
 * - ELM_OBJECT_SELECT_MODE_ALWAYS :  This means that, even if selected,
 *      every click will make the selected callbacks be called.
 * - ELM_OBJECT_SELECT_MODE_NONE : This will turn off the ability to select items
 *      entirely and they will neither appear selected nor call selected
 *      callback functions.
 *
 * @see elm_toolbar_select_mode_get()
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_select_mode_set(Evas_Object *obj, Elm_Object_Select_Mode mode);

/**
 * Get the toolbar select mode.
 *
 * @param obj The toolbar object
 * @return The select mode
 * (If getting mode is failed, it returns ELM_OBJECT_SELECT_MODE_MAX)
 *
 * @see elm_toolbar_select_mode_set()
 *
 * @ingroup Toolbar
 */
EAPI Elm_Object_Select_Mode
elm_toolbar_select_mode_get(const Evas_Object *obj);

#define ELM_OBJ_TOOLBAR_CLASS elm_obj_toolbar_class_get()

const Eo_Class *elm_obj_toolbar_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_TOOLBAR_BASE_ID;

enum
{
   ELM_OBJ_TOOLBAR_SUB_ID_ICON_SIZE_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_ICON_SIZE_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_ITEM_APPEND,
   ELM_OBJ_TOOLBAR_SUB_ID_ITEM_PREPEND,
   ELM_OBJ_TOOLBAR_SUB_ID_ITEM_INSERT_BEFORE,
   ELM_OBJ_TOOLBAR_SUB_ID_ITEM_INSERT_AFTER,
   ELM_OBJ_TOOLBAR_SUB_ID_FIRST_ITEM_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_LAST_ITEM_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_ITEM_FIND_BY_LABEL,
   ELM_OBJ_TOOLBAR_SUB_ID_SELECTED_ITEM_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_MORE_ITEM_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_SHRINK_MODE_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_SHRINK_MODE_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_HOMOGENEOUS_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_HOMOGENEOUS_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_MENU_PARENT_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_MENU_PARENT_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_ALIGN_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_ALIGN_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_ICON_ORDER_LOOKUP_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_ICON_ORDER_LOOKUP_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_ITEMS_COUNT,
   ELM_OBJ_TOOLBAR_SUB_ID_STANDARD_PRIORITY_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_STANDARD_PRIORITY_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_SELECT_MODE_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_SELECT_MODE_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_LAST
};

#define ELM_OBJ_TOOLBAR_ID(sub_id) (ELM_OBJ_TOOLBAR_BASE_ID + sub_id)


/**
 * @def elm_obj_toolbar_icon_size_set
 * @since 1.8
 *
 * Set the icon size, in pixels, to be used by toolbar items.
 *
 * @param[in] icon_size
 *
 * @see elm_toolbar_icon_size_set
 */
#define elm_obj_toolbar_icon_size_set(icon_size) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ICON_SIZE_SET), EO_TYPECHECK(int, icon_size)

/**
 * @def elm_obj_toolbar_icon_size_get
 * @since 1.8
 *
 * Get the icon size, in pixels, to be used by toolbar items.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_icon_size_get
 */
#define elm_obj_toolbar_icon_size_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ICON_SIZE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_toolbar_item_append
 * @since 1.8
 *
 * Append item to the toolbar.
 *
 * @param[in] icon
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_toolbar_item_append
 */
#define elm_obj_toolbar_item_append(icon, label, func, data, ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const char *, icon), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_item_prepend
 * @since 1.8
 *
 * Prepend item to the toolbar.
 *
 * @param[in] icon
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_toolbar_item_prepend
 */
#define elm_obj_toolbar_item_prepend(icon, label, func, data, ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_PREPEND), EO_TYPECHECK(const char *, icon), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_item_insert_before
 * @since 1.8
 *
 * Insert a new item into the toolbar object before item before.
 *
 * @param[in] before
 * @param[in] icon
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_toolbar_item_insert_before
 */
#define elm_obj_toolbar_item_insert_before(before, icon, label, func, data, ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_INSERT_BEFORE), EO_TYPECHECK(Elm_Object_Item *, before), EO_TYPECHECK(const char *, icon), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_item_insert_after
 * @since 1.8
 *
 * Insert a new item into the toolbar object after item after.
 *
 * @param[in] after
 * @param[in] icon
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_toolbar_item_insert_after
 */
#define elm_obj_toolbar_item_insert_after(after, icon, label, func, data, ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_INSERT_AFTER), EO_TYPECHECK(Elm_Object_Item *, after), EO_TYPECHECK(const char *, icon), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_first_item_get
 * @since 1.8
 *
 * Get the first item in the given toolbar widget's list of
 * items.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_first_item_get
 */
#define elm_obj_toolbar_first_item_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_FIRST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_last_item_get
 * @since 1.8
 *
 * Get the last item in the given toolbar widget's list of
 * items.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_last_item_get
 */
#define elm_obj_toolbar_last_item_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_LAST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_item_find_by_label
 * @since 1.8
 *
 * Returns a pointer to a toolbar item by its label.
 *
 * @param[in] label
 * @param[out] ret
 *
 * @see elm_toolbar_item_find_by_label
 */
#define elm_obj_toolbar_item_find_by_label(label, ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_FIND_BY_LABEL), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_selected_item_get
 * @since 1.8
 *
 * Get the selected item.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_selected_item_get
 */
#define elm_obj_toolbar_selected_item_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SELECTED_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_more_item_get
 * @since 1.8
 *
 * Get the more item.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_more_item_get
 */
#define elm_obj_toolbar_more_item_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_MORE_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_shrink_mode_set
 * @since 1.8
 *
 * Set the item displaying mode of a given toolbar widget obj.
 *
 * @param[in] shrink_mode
 *
 * @see elm_toolbar_shrink_mode_set
 */
#define elm_obj_toolbar_shrink_mode_set(shrink_mode) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SHRINK_MODE_SET), EO_TYPECHECK(Elm_Toolbar_Shrink_Mode, shrink_mode)

/**
 * @def elm_obj_toolbar_shrink_mode_get
 * @since 1.8
 *
 * Get the shrink mode of toolbar obj.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_shrink_mode_get
 */
#define elm_obj_toolbar_shrink_mode_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SHRINK_MODE_GET), EO_TYPECHECK(Elm_Toolbar_Shrink_Mode *, ret)

/**
 * @def elm_obj_toolbar_homogeneous_set
 * @since 1.8
 *
 * Enable/disable homogeneous mode.
 *
 * @param[in] homogeneous
 *
 * @see elm_toolbar_homogeneous_set
 */
#define elm_obj_toolbar_homogeneous_set(homogeneous) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_HOMOGENEOUS_SET), EO_TYPECHECK(Eina_Bool, homogeneous)

/**
 * @def elm_obj_toolbar_homogeneous_get
 * @since 1.8
 *
 * Get whether the homogeneous mode is enabled.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_homogeneous_get
 */
#define elm_obj_toolbar_homogeneous_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_HOMOGENEOUS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_toolbar_menu_parent_set
 * @since 1.8
 *
 * Set the parent object of the toolbar items' menus.
 *
 * @param[in] parent
 *
 * @see elm_toolbar_menu_parent_set
 */
#define elm_obj_toolbar_menu_parent_set(parent) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_MENU_PARENT_SET), EO_TYPECHECK(Evas_Object *, parent)

/**
 * @def elm_obj_toolbar_menu_parent_get
 * @since 1.8
 *
 * Get the parent object of the toolbar items' menus.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_menu_parent_get
 */
#define elm_obj_toolbar_menu_parent_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_MENU_PARENT_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_toolbar_align_set
 * @since 1.8
 *
 * Set the alignment of the items.
 *
 * @param[in] align
 *
 * @see elm_toolbar_align_set
 */
#define elm_obj_toolbar_align_set(align) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ALIGN_SET), EO_TYPECHECK(double, align)

/**
 * @def elm_obj_toolbar_align_get
 * @since 1.8
 *
 * Get the alignment of the items.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_align_get
 */
#define elm_obj_toolbar_align_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ALIGN_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_toolbar_icon_order_lookup_set
 * @since 1.8
 *
 * Sets icon lookup order, for toolbar items' icons.
 *
 * @param[in] order
 *
 * @see elm_toolbar_icon_order_lookup_set
 */
#define elm_obj_toolbar_icon_order_lookup_set(order) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ICON_ORDER_LOOKUP_SET), EO_TYPECHECK(Elm_Icon_Lookup_Order, order)

/**
 * @def elm_obj_toolbar_icon_order_lookup_get
 * @since 1.8
 *
 * Gets the icon lookup order.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_icon_order_lookup_get
 */
#define elm_obj_toolbar_icon_order_lookup_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ICON_ORDER_LOOKUP_GET), EO_TYPECHECK(Elm_Icon_Lookup_Order *, ret)

/**
 * @def elm_obj_toolbar_horizontal_set
 * @since 1.8
 *
 * Change a toolbar's orientation
 *
 * @param[in] horizontal
 *
 * @see elm_toolbar_horizontal_set
 */
#define elm_obj_toolbar_horizontal_set(horizontal) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_toolbar_horizontal_get
 * @since 1.8
 *
 * Get a toolbar's orientation
 *
 * @param[out] ret
 *
 * @see elm_toolbar_horizontal_get
 */
#define elm_obj_toolbar_horizontal_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_toolbar_items_count
 * @since 1.8
 *
 * Get the number of items in a toolbar
 *
 * @param[out] ret
 *
 * @see elm_toolbar_items_count
 */
#define elm_obj_toolbar_items_count(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEMS_COUNT), EO_TYPECHECK(unsigned int *, ret)

/**
 * @def elm_obj_toolbar_standard_priority_set
 * @since 1.8
 *
 * Set the standard priority of visible items in a toolbar
 *
 * @param[in] priority
 *
 * @see elm_toolbar_standard_priority_set
 */
#define elm_obj_toolbar_standard_priority_set(priority) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_STANDARD_PRIORITY_SET), EO_TYPECHECK(int, priority)

/**
 * @def elm_obj_toolbar_standard_priority_get
 * @since 1.8
 *
 * Get the standard_priority of visible items in a toolbar
 *
 * @param[out] ret
 *
 * @see elm_toolbar_standard_priority_get
 */
#define elm_obj_toolbar_standard_priority_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_STANDARD_PRIORITY_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_toolbar_select_mode_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] mode
 *
 * @see elm_toolbar_select_mode_set
 */
#define elm_obj_toolbar_select_mode_set(mode) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SELECT_MODE_SET), EO_TYPECHECK(Elm_Object_Select_Mode, mode)

/**
 * @def elm_obj_toolbar_select_mode_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_select_mode_get
 */
#define elm_obj_toolbar_select_mode_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SELECT_MODE_GET), EO_TYPECHECK(Elm_Object_Select_Mode *, ret)


/**
 * Set reorder mode
 *
 * @param obj The toolbar object
 * @param reorder_mode The reorder mode
 * (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Toolbar
 */
EAPI void                          elm_toolbar_reorder_mode_set(Evas_Object *obj, Eina_Bool reorder_mode);

/**
 * Get the reorder mode
 *
 * @param obj The toolbar object
 * @return The reorder mode
 * (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool                     elm_toolbar_reorder_mode_get(const Evas_Object *obj);

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

/**
 * @}
 */
