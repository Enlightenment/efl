   /**
    * @defgroup Toolbar Toolbar
    * @ingroup Elementary
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
    * Smart callbacks one can listen to:
    * - "clicked" - when the user clicks on a toolbar item and becomes selected.
    * - "language,changed" - when the program language changes
    *
    * Available styles for it:
    * - @c "default"
    * - @c "transparent" - no background or shadow, just show the content
    *
    * Default text parts of the toolbar items that you can use for are:
    * @li "default" - label of the toolbar item
    *  
    * Supported elm_object_item common APIs.
    * @li elm_object_item_disabled_set
    * @li elm_object_item_text_set
    * @li elm_object_item_part_text_set
    * @li elm_object_item_text_get
    * @li elm_object_item_part_text_get
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
    * @enum _Elm_Toolbar_Shrink_Mode
    * @typedef Elm_Toolbar_Shrink_Mode
    *
    * Set toolbar's items display behavior, it can be scrollabel,
    * show a menu with exceeding items, or simply hide them.
    *
    * @note Default value is #ELM_TOOLBAR_SHRINK_MENU. It reads value
    * from elm config.
    *
    * Values <b> don't </b> work as bitmask, only one can be choosen.
    *
    * @see elm_toolbar_mode_shrink_set()
    * @see elm_toolbar_mode_shrink_get()
    *
    * @ingroup Toolbar
    */
   typedef enum _Elm_Toolbar_Shrink_Mode
     {
        ELM_TOOLBAR_SHRINK_NONE,   /**< Set toolbar minimun size to fit all the items. */
        ELM_TOOLBAR_SHRINK_HIDE,   /**< Hide exceeding items. */
        ELM_TOOLBAR_SHRINK_SCROLL, /**< Allow accessing exceeding items through a scroller. */
        ELM_TOOLBAR_SHRINK_MENU,   /**< Inserts a button to pop up a menu with exceeding items. */
        ELM_TOOLBAR_SHRINK_LAST    /**< Indicates error if returned by elm_toolbar_shrink_mode_get() */
     } Elm_Toolbar_Shrink_Mode;

   typedef struct _Elm_Toolbar_Item_State Elm_Toolbar_Item_State; /**< State of a Elm_Toolbar_Item. Can be created with elm_toolbar_item_state_add() and removed with elm_toolbar_item_state_del(). */

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
   EAPI Evas_Object            *elm_toolbar_add(Evas_Object *parent) EINA_ARG_NONNULL(1);

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
   EAPI void                    elm_toolbar_icon_size_set(Evas_Object *obj, int icon_size) EINA_ARG_NONNULL(1);

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
   EAPI int                     elm_toolbar_icon_size_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
   EAPI void                    elm_toolbar_icon_order_lookup_set(Evas_Object *obj, Elm_Icon_Lookup_Order order) EINA_ARG_NONNULL(1);

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
   EAPI Elm_Icon_Lookup_Order   elm_toolbar_icon_order_lookup_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * Set whether the toolbar should always have an item selected.
    *
    * @param obj The toolbar object.
    * @param wrap @c EINA_TRUE to enable always-select mode or @c EINA_FALSE to
    * disable it.
    *
    * This will cause the toolbar to always have an item selected, and clicking
    * the selected item will not cause a selected event to be emitted. Enabling this mode
    * will immediately select the first toolbar item.
    *
    * Always-selected is disabled by default.
    *
    * @see elm_toolbar_always_select_mode_get().
    *
    * @ingroup Toolbar
    */
   EAPI void                    elm_toolbar_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select) EINA_ARG_NONNULL(1);

   /**
    * Get whether the toolbar should always have an item selected.
    *
    * @param obj The toolbar object.
    * @return @c EINA_TRUE means an item will always be selected, @c EINA_FALSE indicates
    * that it is possible to have no items selected. If @p obj is @c NULL, @c EINA_FALSE is returned.
    *
    * @see elm_toolbar_always_select_mode_set() for details.
    *
    * @ingroup Toolbar
    */
   EAPI Eina_Bool               elm_toolbar_always_select_mode_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * Set whether the toolbar items' should be selected by the user or not.
    *
    * @param obj The toolbar object.
    * @param wrap @c EINA_TRUE to disable selection or @c EINA_FALSE to
    * enable it.
    *
    * This will turn off the ability to select items entirely and they will
    * neither appear selected nor emit selected signals. The clicked
    * callback function will still be called.
    *
    * Selection is enabled by default.
    *
    * @see elm_toolbar_no_select_mode_get().
    *
    * @ingroup Toolbar
    */
   EAPI void                    elm_toolbar_no_select_mode_set(Evas_Object *obj, Eina_Bool no_select) EINA_ARG_NONNULL(1);

   /**
    * Set whether the toolbar items' should be selected by the user or not.
    *
    * @param obj The toolbar object.
    * @return @c EINA_TRUE means items can be selected. @c EINA_FALSE indicates
    * they can't. If @p obj is @c NULL, @c EINA_FALSE is returned.
    *
    * @see elm_toolbar_no_select_mode_set() for details.
    *
    * @ingroup Toolbar
    */
   EAPI Eina_Bool               elm_toolbar_no_select_mode_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
    * elm_toolbar_item_del().
    *
    * Associated @p data can be properly freed when item is deleted if a
    * callback function is set with elm_toolbar_item_del_cb_set().
    *
    * If a function is passed as argument, it will be called everytime this item
    * is selected, i.e., the user clicks over an unselected item.
    * If such function isn't needed, just passing
    * @c NULL as @p func is enough. The same should be done for @p data.
    *
    * Toolbar will load icon image from fdo or current theme.
    * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
    * If an absolute path is provided it will load it direct from a file.
    *
    * @see elm_toolbar_item_icon_set()
    * @see elm_toolbar_item_del()
    * @see elm_toolbar_item_del_cb_set()
    *
    * @ingroup Toolbar
    */
   EAPI Elm_Object_Item       *elm_toolbar_item_append(Evas_Object *obj, const char *icon, const char *label, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1);

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
    * elm_toolbar_item_del().
    *
    * Associated @p data can be properly freed when item is deleted if a
    * callback function is set with elm_toolbar_item_del_cb_set().
    *
    * If a function is passed as argument, it will be called everytime this item
    * is selected, i.e., the user clicks over an unselected item.
    * If such function isn't needed, just passing
    * @c NULL as @p func is enough. The same should be done for @p data.
    *
    * Toolbar will load icon image from fdo or current theme.
    * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
    * If an absolute path is provided it will load it direct from a file.
    *
    * @see elm_toolbar_item_icon_set()
    * @see elm_toolbar_item_del()
    * @see elm_toolbar_item_del_cb_set()
    *
    * @ingroup Toolbar
    */
   EAPI Elm_Object_Item       *elm_toolbar_item_prepend(Evas_Object *obj, const char *icon, const char *label, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1);

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
    * elm_toolbar_item_del().
    *
    * Associated @p data can be properly freed when item is deleted if a
    * callback function is set with elm_toolbar_item_del_cb_set().
    *
    * If a function is passed as argument, it will be called everytime this item
    * is selected, i.e., the user clicks over an unselected item.
    * If such function isn't needed, just passing
    * @c NULL as @p func is enough. The same should be done for @p data.
    *
    * Toolbar will load icon image from fdo or current theme.
    * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
    * If an absolute path is provided it will load it direct from a file.
    *
    * @see elm_toolbar_item_icon_set()
    * @see elm_toolbar_item_del()
    * @see elm_toolbar_item_del_cb_set()
    *
    * @ingroup Toolbar
    */
   EAPI Elm_Object_Item       *elm_toolbar_item_insert_before(Evas_Object *obj, Elm_Object_Item *before, const char *icon, const char *label, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1);

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
    * elm_toolbar_item_del().
    *
    * Associated @p data can be properly freed when item is deleted if a
    * callback function is set with elm_toolbar_item_del_cb_set().
    *
    * If a function is passed as argument, it will be called everytime this item
    * is selected, i.e., the user clicks over an unselected item.
    * If such function isn't needed, just passing
    * @c NULL as @p func is enough. The same should be done for @p data.
    *
    * Toolbar will load icon image from fdo or current theme.
    * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
    * If an absolute path is provided it will load it direct from a file.
    *
    * @see elm_toolbar_item_icon_set()
    * @see elm_toolbar_item_del()
    * @see elm_toolbar_item_del_cb_set()
    *
    * @ingroup Toolbar
    */
   EAPI Elm_Object_Item       *elm_toolbar_item_insert_after(Evas_Object *obj, Elm_Object_Item *after, const char *icon, const char *label, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1);

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
   EAPI Elm_Object_Item       *elm_toolbar_first_item_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
   EAPI Elm_Object_Item       *elm_toolbar_last_item_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
   EAPI Elm_Object_Item       *elm_toolbar_item_next_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Get the item before @p item in toolbar.
    *
    * @param item The toolbar item.
    * @return The item before @p item, or @c NULL if none or on failure.
    *
    * @note If it is the first item, @c NULL will be returned.
    *
    * @see elm_toolbar_item_prepend()
    *
    * @ingroup Toolbar
    */
   EAPI Elm_Object_Item       *elm_toolbar_item_prev_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Get the toolbar object from an item.
    *
    * @param it The item.
    * @return The toolbar object.
    *
    * This returns the toolbar object itself that an item belongs to.
    *
	 * @deprecated use elm_object_item_object_get() instead.
    * @ingroup Toolbar
    */
   EINA_DEPRECATED EAPI Evas_Object            *elm_toolbar_item_toolbar_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Set the priority of a toolbar item.
    *
    * @param it The toolbar item.
    * @param priority The item priority. The default is zero.
    *
    * This is used only when the toolbar shrink mode is set to
    * #ELM_TOOLBAR_SHRINK_MENU or #ELM_TOOLBAR_SHRINK_HIDE.
    * When space is less than required, items with low priority
    * will be removed from the toolbar and added to a dynamically-created menu,
    * while items with higher priority will remain on the toolbar,
    * with the same order they were added.
    *
    * @see elm_toolbar_item_priority_get()
    *
    * @ingroup Toolbar
    */
   EAPI void                    elm_toolbar_item_priority_set(Elm_Object_Item *it, int priority) EINA_ARG_NONNULL(1);

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
   EAPI int                     elm_toolbar_item_priority_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Get the label of item.
    *
    * @param it The item of toolbar.
    * @return The label of item.
    *
    * The return value is a pointer to the label associated to @p item when
    * it was created, with function elm_toolbar_item_append() or similar,
    * or later,
    * with function elm_toolbar_item_label_set. If no label
    * was passed as argument, it will return @c NULL.
    *
    * @see elm_toolbar_item_label_set() for more details.
    * @see elm_toolbar_item_append()
    *
	 * @deprecated use elm_object_item_text_get() instead.
    * @ingroup Toolbar
    */
   EINA_DEPRECATED EAPI const char             *elm_toolbar_item_label_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Set the label of item.
    *
    * @param it The item of toolbar.
    * @param text The label of item.
    *
    * The label to be displayed by the item.
    * Label will be placed at icons bottom (if set).
    *
    * If a label was passed as argument on item creation, with function
    * elm_toolbar_item_append() or similar, it will be already
    * displayed by the item.
    *
    * @see elm_toolbar_item_label_get()
    * @see elm_toolbar_item_append()
    *
	 * @deprecated use elm_object_item_text_set() instead
    * @ingroup Toolbar
    */
   EINA_DEPRECATED EAPI void                    elm_toolbar_item_label_set(Elm_Object_Item *it, const char *label) EINA_ARG_NONNULL(1);

   /**
    * Return the data associated with a given toolbar widget item.
    *
    * @param it The toolbar widget item handle.
    * @return The data associated with @p item.
    *
    * @see elm_toolbar_item_data_set()
    *
    * @deprecated use elm_object_item_data_get() instead.
    * @ingroup Toolbar
    */
   EINA_DEPRECATED EAPI void                   *elm_toolbar_item_data_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Set the data associated with a given toolbar widget item.
    *
    * @param it The toolbar widget item handle
    * @param data The new data pointer to set to @p item.
    *
    * This sets new item data on @p item.
    *
    * @warning The old data pointer won't be touched by this function, so
    * the user had better to free that old data himself/herself.
    *
    * @deprecated use elm_object_item_data_set() instead.
    * @ingroup Toolbar
    */
   EINA_DEPRECATED EAPI void                    elm_toolbar_item_data_set(Elm_Object_Item *it, const void *data) EINA_ARG_NONNULL(1);

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
   EAPI Elm_Object_Item       *elm_toolbar_item_find_by_label(const Evas_Object *obj, const char *label) EINA_ARG_NONNULL(1);

   /*
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
   EAPI Eina_Bool               elm_toolbar_item_selected_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Set the selected state of an item.
    *
    * @param it The toolbar item
    * @param selected The selected state
    *
    * This sets the selected state of the given item @p it.
    * @c EINA_TRUE for selected, @c EINA_FALSE for not selected.
    *
    * If a new item is selected the previosly selected will be unselected.
    * Previoulsy selected item can be get with function
    * elm_toolbar_selected_item_get().
    *
    * Selected items will be highlighted.
    *
    * @see elm_toolbar_item_selected_get()
    * @see elm_toolbar_selected_item_get()
    *
    * @ingroup Toolbar
    */
   EAPI void                    elm_toolbar_item_selected_set(Elm_Object_Item *it, Eina_Bool selected) EINA_ARG_NONNULL(1);

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
   EAPI Elm_Object_Item       *elm_toolbar_selected_item_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * Set the icon associated with @p item.
    *
    * @param obj The parent of this item.
    * @param it The toolbar item.
    * @param icon A string with icon name or the absolute path of an image file.
    *
    * Toolbar will load icon image from fdo or current theme.
    * This behavior can be set by elm_toolbar_icon_order_lookup_set() function.
    * If an absolute path is provided it will load it direct from a file.
    *
    * @see elm_toolbar_icon_order_lookup_set()
    * @see elm_toolbar_icon_order_lookup_get()
    *
    * @ingroup Toolbar
    */
   EAPI void                    elm_toolbar_item_icon_set(Elm_Object_Item *it, const char *icon) EINA_ARG_NONNULL(1);

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
   EAPI const char             *elm_toolbar_item_icon_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Get the object of @p item.
    *
    * @param it The toolbar item.
    * @return The object
    *
    * @ingroup Toolbar
    */
   EAPI Evas_Object            *elm_toolbar_item_object_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

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
   EAPI Evas_Object            *elm_toolbar_item_icon_object_get(Elm_Object_Item *it) EINA_ARG_NONNULL(1);

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
   EAPI Eina_Bool elm_toolbar_item_icon_memfile_set(Elm_Object_Item *it, const void *img, size_t size, const char *format, const char *key) EINA_ARG_NONNULL(1);

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
   EAPI Eina_Bool elm_toolbar_item_icon_file_set(Elm_Object_Item *it, const char *file, const char *key) EINA_ARG_NONNULL(1);

   /**
    * Delete them item from the toolbar.
    *
    * @param it The item of toolbar to be deleted.
    *
    * @see elm_toolbar_item_append()
    * @see elm_toolbar_item_del_cb_set()
    *
    * @ingroup Toolbar
    */
   EAPI void                    elm_toolbar_item_del(Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Set the function called when a toolbar item is freed.
    *
    * @param it The item to set the callback on.
    * @param func The function called.
    *
    * If there is a @p func, then it will be called prior item's memory release.
    * That will be called with the following arguments:
    * @li item's data;
    * @li item's Evas object;
    * @li item itself;
    *
    * This way, a data associated to a toolbar item could be properly freed.
    *
    * @ingroup Toolbar
    */
   EAPI void                    elm_toolbar_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func) EINA_ARG_NONNULL(1);

   /**
    * Get a value whether toolbar item is disabled or not.
    *
    * @param it The item.
    * @return The disabled state.
    *
    * @see elm_toolbar_item_disabled_set() for more details.
    *
    * @deprecated use elm_object_item_disabled_get() instead.
    * @ingroup Toolbar
    */
   EINA_DEPRECATED EAPI Eina_Bool               elm_toolbar_item_disabled_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Sets the disabled/enabled state of a toolbar item.
    *
    * @param it The item.
    * @param disabled The disabled state.
    *
    * A disabled item cannot be selected or unselected. It will also
    * change its appearance (generally greyed out). This sets the
    * disabled state (@c EINA_TRUE for disabled, @c EINA_FALSE for
    * enabled).
    *
    * @deprecated use elm_object_item_disabled_set() instead.
    * @ingroup Toolbar
    */
   EINA_DEPRECATED EAPI void                    elm_toolbar_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled) EINA_ARG_NONNULL(1);

   /**
    * Set or unset item as a separator.
    *
    * @param it The toolbar item.
    * @param setting @c EINA_TRUE to set item @p item as separator or
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
   EAPI void                    elm_toolbar_item_separator_set(Elm_Object_Item *it, Eina_Bool separator) EINA_ARG_NONNULL(1);

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
   EAPI Eina_Bool               elm_toolbar_item_separator_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Set the shrink state of toolbar @p obj.
    *
    * @param obj The toolbar object.
    * @param shrink_mode Toolbar's items display behavior.
    *
    * The toolbar won't scroll if #ELM_TOOLBAR_SHRINK_NONE,
    * but will enforce a minimun size so all the items will fit, won't scroll
    * and won't show the items that don't fit if #ELM_TOOLBAR_SHRINK_HIDE,
    * will scroll if #ELM_TOOLBAR_SHRINK_SCROLL, and will create a button to
    * pop up excess elements with #ELM_TOOLBAR_SHRINK_MENU.
    *
    * @ingroup Toolbar
    */
   EAPI void                    elm_toolbar_mode_shrink_set(Evas_Object *obj, Elm_Toolbar_Shrink_Mode shrink_mode) EINA_ARG_NONNULL(1);

   /**
    * Get the shrink mode of toolbar @p obj.
    *
    * @param obj The toolbar object.
    * @return Toolbar's items display behavior.
    *
    * @see elm_toolbar_mode_shrink_set() for details.
    *
    * @ingroup Toolbar
    */
   EAPI Elm_Toolbar_Shrink_Mode elm_toolbar_mode_shrink_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
   EAPI void                    elm_toolbar_homogeneous_set(Evas_Object *obj, Eina_Bool homogeneous) EINA_ARG_NONNULL(1);

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
   EAPI Eina_Bool               elm_toolbar_homogeneous_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
   EAPI void                    elm_toolbar_menu_parent_set(Evas_Object *obj, Evas_Object *parent) EINA_ARG_NONNULL(1);

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
   EAPI Evas_Object            *elm_toolbar_menu_parent_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
   EAPI void                    elm_toolbar_align_set(Evas_Object *obj, double align) EINA_ARG_NONNULL(1);

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
   EAPI double                  elm_toolbar_align_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
   EAPI void                    elm_toolbar_item_menu_set(Elm_Object_Item *it, Eina_Bool menu) EINA_ARG_NONNULL(1);

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
   EAPI Evas_Object            *elm_toolbar_item_menu_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

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
   EAPI Elm_Toolbar_Item_State *elm_toolbar_item_state_add(Elm_Object_Item *it, const char *icon, const char *label, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1);

   /**
    * Delete a previoulsy added state to @p item.
    *
    * @param it The toolbar item.
    * @param state The state to be deleted.
    * @return @c EINA_TRUE on success or @c EINA_FALSE on failure.
    *
    * @see elm_toolbar_item_state_add()
    */
   EAPI Eina_Bool               elm_toolbar_item_state_del(Elm_Object_Item *it, Elm_Toolbar_Item_State *state) EINA_ARG_NONNULL(1);

   /**
    * Set @p state as the current state of @p it.
    *
    * @param it The toolbar item.
    * @param state The state to use.
    * @return @c EINA_TRUE on success or @c EINA_FALSE on failure.
    *
    * If @p state is @c NULL, it won't select any state and the default item's
    * icon and label will be used. It's the same behaviour than
    * elm_toolbar_item_state_unser().
    *
    * @see elm_toolbar_item_state_unset()
    *
    * @ingroup Toolbar
    */
   EAPI Eina_Bool               elm_toolbar_item_state_set(Elm_Object_Item *it, Elm_Toolbar_Item_State *state) EINA_ARG_NONNULL(1);

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
   EAPI void                    elm_toolbar_item_state_unset(Elm_Object_Item *it) EINA_ARG_NONNULL(1);

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
   EAPI Elm_Toolbar_Item_State *elm_toolbar_item_state_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

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
   EAPI Elm_Toolbar_Item_State *elm_toolbar_item_state_next(Elm_Object_Item *it) EINA_ARG_NONNULL(1);

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
   EAPI Elm_Toolbar_Item_State *elm_toolbar_item_state_prev(Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Set the text to be shown in a given toolbar item's tooltips.
    *
    * @param it toolbar item.
    * @param text The text to set in the content.
    *
    * Setup the text as tooltip to object. The item can have only one tooltip,
    * so any previous tooltip data - set with this function or
    * elm_toolbar_item_tooltip_content_cb_set() - is removed.
    *
    * @see elm_object_tooltip_text_set() for more details.
    *
    * @ingroup Toolbar
    */
   EAPI void             elm_toolbar_item_tooltip_text_set(Elm_Object_Item *it, const char *text) EINA_ARG_NONNULL(1);

   /**
    * Set the content to be shown in the tooltip item.
    *
    * Setup the tooltip to item. The item can have only one tooltip,
    * so any previous tooltip data is removed. @p func(with @p data) will
    * be called every time that need show the tooltip and it should
    * return a valid Evas_Object. This object is then managed fully by
    * tooltip system and is deleted when the tooltip is gone.
    *
    * @param it the toolbar item being attached a tooltip.
    * @param func the function used to create the tooltip contents.
    * @param data what to provide to @a func as callback data/context.
    * @param del_cb called when data is not needed anymore, either when
    *        another callback replaces @a func, the tooltip is unset with
    *        elm_toolbar_item_tooltip_unset() or the owner @a item
    *        dies. This callback receives as the first parameter the
    *        given @a data, and @c event_info is the item.
    *
    * @see elm_object_tooltip_content_cb_set() for more details.
    *
    * @ingroup Toolbar
    */
   EAPI void             elm_toolbar_item_tooltip_content_cb_set(Elm_Object_Item *it, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb) EINA_ARG_NONNULL(1);

   /**
    * Unset tooltip from item.
    *
    * @param it toolbar item to remove previously set tooltip.
    *
    * Remove tooltip from item. The callback provided as del_cb to
    * elm_toolbar_item_tooltip_content_cb_set() will be called to notify
    * it is not used anymore.
    *
    * @see elm_object_tooltip_unset() for more details.
    * @see elm_toolbar_item_tooltip_content_cb_set()
    *
    * @ingroup Toolbar
    */
   EAPI void             elm_toolbar_item_tooltip_unset(Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Sets a different style for this item tooltip.
    *
    * @note before you set a style you should define a tooltip with
    *       elm_toolbar_item_tooltip_content_cb_set() or
    *       elm_toolbar_item_tooltip_text_set()
    *
    * @param it toolbar item with tooltip already set.
    * @param style the theme style to use (default, transparent, ...)
    *
    * @see elm_object_tooltip_style_set() for more details.
    *
    * @ingroup Toolbar
    */
   EAPI void             elm_toolbar_item_tooltip_style_set(Elm_Object_Item *it, const char *style) EINA_ARG_NONNULL(1);

   /**
    * Get the style for this item tooltip.
    *
    * @param it toolbar item with tooltip already set.
    * @return style the theme style in use, defaults to "default". If the
    *         object does not have a tooltip set, then NULL is returned.
    *
    * @see elm_object_tooltip_style_get() for more details.
    * @see elm_toolbar_item_tooltip_style_set()
    *
    * @ingroup Toolbar
    */
   EAPI const char      *elm_toolbar_item_tooltip_style_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Set the type of mouse pointer/cursor decoration to be shown,
    * when the mouse pointer is over the given toolbar widget item
    *
    * @param it toolbar item to customize cursor on
    * @param cursor the cursor type's name
    *
    * This function works analogously as elm_object_cursor_set(), but
    * here the cursor's changing area is restricted to the item's
    * area, and not the whole widget's. Note that that item cursors
    * have precedence over widget cursors, so that a mouse over an
    * item with custom cursor set will always show @b that cursor.
    *
    * If this function is called twice for an object, a previously set
    * cursor will be unset on the second call.
    *
    * @see elm_object_cursor_set()
    * @see elm_toolbar_item_cursor_get()
    * @see elm_toolbar_item_cursor_unset()
    *
    * @ingroup Toolbar
    */
   EAPI void             elm_toolbar_item_cursor_set(Elm_Object_Item *it, const char *cursor) EINA_ARG_NONNULL(1);

   /*
    * Get the type of mouse pointer/cursor decoration set to be shown,
    * when the mouse pointer is over the given toolbar widget item
    *
    * @param it toolbar item with custom cursor set
    * @return the cursor type's name or @c NULL, if no custom cursors
    * were set to @p item (and on errors)
    *
    * @see elm_object_cursor_get()
    * @see elm_toolbar_item_cursor_set()
    * @see elm_toolbar_item_cursor_unset()
    *
    * @ingroup Toolbar
    */
   EAPI const char      *elm_toolbar_item_cursor_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Unset any custom mouse pointer/cursor decoration set to be
    * shown, when the mouse pointer is over the given toolbar widget
    * item, thus making it show the @b default cursor again.
    *
    * @param it a toolbar item
    *
    * Use this call to undo any custom settings on this item's cursor
    * decoration, bringing it back to defaults (no custom style set).
    *
    * @see elm_object_cursor_unset()
    * @see elm_toolbar_item_cursor_set()
    *
    * @ingroup Toolbar
    */
   EAPI void             elm_toolbar_item_cursor_unset(Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Set a different @b style for a given custom cursor set for a
    * toolbar item.
    *
    * @param it toolbar item with custom cursor set
    * @param style the <b>theme style</b> to use (e.g. @c "default",
    * @c "transparent", etc)
    *
    * This function only makes sense when one is using custom mouse
    * cursor decorations <b>defined in a theme file</b>, which can have,
    * given a cursor name/type, <b>alternate styles</b> on it. It
    * works analogously as elm_object_cursor_style_set(), but here
    * applyed only to toolbar item objects.
    *
    * @warning Before you set a cursor style you should have definen a
    *       custom cursor previously on the item, with
    *       elm_toolbar_item_cursor_set()
    *
    * @see elm_toolbar_item_cursor_engine_only_set()
    * @see elm_toolbar_item_cursor_style_get()
    *
    * @ingroup Toolbar
    */
   EAPI void             elm_toolbar_item_cursor_style_set(Elm_Object_Item *it, const char *style) EINA_ARG_NONNULL(1);

   /**
    * Get the current @b style set for a given toolbar item's custom
    * cursor
    *
    * @param it toolbar item with custom cursor set.
    * @return style the cursor style in use. If the object does not
    *         have a cursor set, then @c NULL is returned.
    *
    * @see elm_toolbar_item_cursor_style_set() for more details
    *
    * @ingroup Toolbar
    */
   EAPI const char      *elm_toolbar_item_cursor_style_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Set if the (custom)cursor for a given toolbar item should be
    * searched in its theme, also, or should only rely on the
    * rendering engine.
    *
    * @param it item with custom (custom) cursor already set on
    * @param engine_only Use @c EINA_TRUE to have cursors looked for
    * only on those provided by the rendering engine, @c EINA_FALSE to
    * have them searched on the widget's theme, as well.
    *
    * @note This call is of use only if you've set a custom cursor
    * for toolbar items, with elm_toolbar_item_cursor_set().
    *
    * @note By default, cursors will only be looked for between those
    * provided by the rendering engine.
    *
    * @ingroup Toolbar
    */
   EAPI void             elm_toolbar_item_cursor_engine_only_set(Elm_Object_Item *it, Eina_Bool engine_only) EINA_ARG_NONNULL(1);

   /**
    * Get if the (custom) cursor for a given toolbar item is being
    * searched in its theme, also, or is only relying on the rendering
    * engine.
    *
    * @param it a toolbar item
    * @return @c EINA_TRUE, if cursors are being looked for only on
    * those provided by the rendering engine, @c EINA_FALSE if they
    * are being searched on the widget's theme, as well.
    *
    * @see elm_toolbar_item_cursor_engine_only_set(), for more details
    *
    * @ingroup Toolbar
    */
   EAPI Eina_Bool        elm_toolbar_item_cursor_engine_only_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Change a toolbar's orientation
    * @param obj The toolbar object
    * @param vertical If @c EINA_TRUE, the toolbar is vertical
    * By default, a toolbar will be horizontal. Use this function to create a vertical toolbar.
    * @ingroup Toolbar
    * @deprecated use elm_toolbar_horizontal_set() instead.
    */
   EINA_DEPRECATED EAPI void             elm_toolbar_orientation_set(Evas_Object *obj, Eina_Bool vertical) EINA_ARG_NONNULL(1);

   /**
    * Change a toolbar's orientation
    * @param obj The toolbar object
    * @param horizontal If @c EINA_TRUE, the toolbar is horizontal
    * By default, a toolbar will be horizontal. Use this function to create a vertical toolbar.
    * @ingroup Toolbar
    */
   EAPI void             elm_toolbar_horizontal_set(Evas_Object *obj, Eina_Bool horizontal) EINA_ARG_NONNULL(1);

   /**
    * Get a toolbar's orientation
    * @param obj The toolbar object
    * @return If @c EINA_TRUE, the toolbar is vertical
    * By default, a toolbar will be horizontal. Use this function to determine whether a toolbar is vertical.
    * @ingroup Toolbar
    * @deprecated use elm_toolbar_horizontal_get() instead.
    */
   EINA_DEPRECATED EAPI Eina_Bool        elm_toolbar_orientation_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * Get a toolbar's orientation
    * @param obj The toolbar object
    * @return If @c EINA_TRUE, the toolbar is horizontal
    * By default, a toolbar will be horizontal. Use this function to determine whether a toolbar is vertical.
    * @ingroup Toolbar
    */
   EAPI Eina_Bool elm_toolbar_horizontal_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * Get the number of items in a toolbar
    * @param obj The toolbar object
    * @return The number of items in @p obj toolbar
    * @ingroup Toolbar
    */
   EAPI unsigned int elm_toolbar_items_count(const Evas_Object *obj) EINA_ARG_NONNULL(1) EINA_PURE;
   /**
    * @}
    */

