/**
 * Add a new gengrid widget to the given parent Elementary
 * (container) object
 *
 * @param parent The parent object
 * @return a new gengrid widget handle or @c NULL, on errors
 *
 * This function inserts a new gengrid widget on the canvas.
 *
 * @see elm_gengrid_item_size_set()
 * @see elm_gengrid_group_item_size_set()
 * @see elm_gengrid_horizontal_set()
 * @see elm_gengrid_item_append()
 * @see elm_object_item_del()
 * @see elm_gengrid_clear()
 *
 * @ingroup Gengrid
 */
EAPI Evas_Object                  *elm_gengrid_add(Evas_Object *parent);

/**
 * Remove all items from a given gengrid widget
 *
 * @param obj The gengrid object.
 *
 * This removes (and deletes) all items in @p obj, leaving it
 * empty.
 *
 * @see elm_object_item_del(), to remove just one item.
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_clear(Evas_Object *obj);

/**
 * Enable or disable multi-selection in a given gengrid widget
 *
 * @param obj The gengrid object.
 * @param multi @c EINA_TRUE, to enable multi-selection,
 * @c EINA_FALSE to disable it.
 *
 * Multi-selection is the ability to have @b more than one
 * item selected, on a given gengrid, simultaneously. When it is
 * enabled, a sequence of clicks on different items will make them
 * all selected, progressively. A click on an already selected item
 * will unselect it. If interacting via the keyboard,
 * multi-selection is enabled while holding the "Shift" key.
 *
 * @note By default, multi-selection is @b disabled on gengrids
 *
 * @see elm_gengrid_multi_select_get()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_multi_select_set(Evas_Object *obj, Eina_Bool multi);

/**
 * Get whether multi-selection is enabled or disabled for a given
 * gengrid widget
 *
 * @param obj The gengrid object.
 * @return @c EINA_TRUE, if multi-selection is enabled, @c
 * EINA_FALSE otherwise
 *
 * @see elm_gengrid_multi_select_set() for more details
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool                     elm_gengrid_multi_select_get(const Evas_Object *obj);

/**
 * Set the gengrid multi select mode.
 *
 * @param obj The gengrid object
 * @param mode The multi select mode
 *
 * - ELM_OBJECT_MULTI_SELECT_MODE_DEFAULT : select/unselect items whenever each
 *   item is clicked.
 * - ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL : Only one item will be selected
 *   although multi-selection is enabled, if clicked without pressing control
 *   key. This mode is only available with multi-selection.
 *
 * @see elm_gengrid_multi_select_set()
 * @see elm_gengrid_multi_select_mode_get()
 *
 * @ingroup Gengrid
 * @since 1.8
 */
EAPI void elm_gengrid_multi_select_mode_set(Evas_Object *obj, Elm_Object_Multi_Select_Mode mode);

/**
 * Get the gengrid multi select mode.
 *
 * @param obj The gengrid object
 * @return The multi select mode
 * (If getting mode is failed, it returns ELM_OBJECT_MULTI_SELECT_MODE_MAX)
 *
 * @see elm_gengrid_multi_select_set()
 * @see elm_gengrid_multi_select_mode_set()
 *
 * @ingroup Gengrid
 * @since 1.8
 */
EAPI Elm_Object_Multi_Select_Mode elm_gengrid_multi_select_mode_get(const Evas_Object *obj);

/**
 * Set the direction in which a given gengrid widget will expand while
 * placing its items.
 *
 * @param obj The gengrid object.
 * @param horizontal @c EINA_TRUE to make the gengrid expand horizontally,
 * @c EINA_FALSE to expand vertically.
 *
 * When in "horizontal mode" (@c EINA_TRUE), items will be placed in @b columns,
 * from top to bottom and, when the space for a column is filled, another one is
 * started on the right, thus expanding the grid horizontally. When in
 * "vertical mode" (@c EINA_FALSE), though, items will be placed in @b rows,
 * from left to right and, when the space for a row is filled, another one is
 * started below, thus expanding the grid vertically.
 *
 * @note By default, gengrid is in vertical mode, @c EINA_FALSE.
 *
 * @see elm_gengrid_horizontal_get()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * Get for what direction a given gengrid widget will expand while
 * placing its items.
 *
 * @param obj The gengrid object.
 * @return @c EINA_TRUE, if @p obj is set to expand horizontally,
 * @c EINA_FALSE if it's set to expand vertically.
 *
 * @see elm_gengrid_horizontal_set() for more details
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool                     elm_gengrid_horizontal_get(const Evas_Object *obj);

/**
 * Enable or disable bouncing effect for a given gengrid widget
 *
 * @param obj The gengrid object
 * @param h_bounce @c EINA_TRUE, to enable @b horizontal bouncing,
 * @c EINA_FALSE to disable it
 * @param v_bounce @c EINA_TRUE, to enable @b vertical bouncing,
 * @c EINA_FALSE to disable it
 *
 * The bouncing effect occurs whenever one reaches the gengrid's
 * edge's while panning it -- it will scroll past its limits a
 * little bit and return to the edge again, in a animated for,
 * automatically.
 *
 * @note By default, gengrids have bouncing enabled on both axis
 *
 * @deprecated Use elm_scroller_bounce_set() instead.
 *
 * @see elm_scroller_bounce_set()
 *
 * @ingroup Gengrid
 */
EINA_DEPRECATED EAPI void          elm_gengrid_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

/**
 * Get whether bouncing effects are enabled or disabled, for a
 * given gengrid widget, on each axis
 *
 * @param obj The gengrid object
 * @param h_bounce Pointer to a variable where to store the
 * horizontal bouncing flag.
 * @param v_bounce Pointer to a variable where to store the
 * vertical bouncing flag.
 *
 * @deprecated Use elm_scroller_bounce_get() instead.
 *
 * @see elm_scroller_bounce_get()
 *
 * @ingroup Gengrid
 */
EINA_DEPRECATED EAPI void          elm_gengrid_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);

/**
 * Append a new item in a given gengrid widget.
 *
 * @param obj The gengrid object.
 * @param gic The item class for the item.
 * @param data The item data.
 * @param func Convenience function called when the item is
 * selected.
 * @param func_data Data to be passed to @p func.
 * @return A handle to the item added or @c NULL, on errors.
 *
 * This adds an item to the beginning of the gengrid.
 *
 * @see elm_gengrid_item_prepend()
 * @see elm_gengrid_item_insert_before()
 * @see elm_gengrid_item_insert_after()
 * @see elm_object_item_del()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Item             *elm_gengrid_item_append(Evas_Object *obj, const Elm_Gengrid_Item_Class *gic, const void *data, Evas_Smart_Cb func, const void *func_data);

/**
 * Prepend a new item in a given gengrid widget.
 *
 * @param obj The gengrid object.
 * @param gic The item class for the item.
 * @param data The item data.
 * @param func Convenience function called when the item is
 * selected.
 * @param func_data Data to be passed to @p func.
 * @return A handle to the item added or @c NULL, on errors.
 *
 * This adds an item to the end of the gengrid.
 *
 * @see elm_gengrid_item_append()
 * @see elm_gengrid_item_insert_before()
 * @see elm_gengrid_item_insert_after()
 * @see elm_object_item_del()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Item             *elm_gengrid_item_prepend(Evas_Object *obj, const Elm_Gengrid_Item_Class *gic, const void *data, Evas_Smart_Cb func, const void *func_data);

/**
 * Insert an item before another in a gengrid widget
 *
 * @param obj The gengrid object.
 * @param gic The item class for the item.
 * @param data The item data.
 * @param relative The item to place this new one before.
 * @param func Convenience function called when the item is
 * selected.
 * @param func_data Data to be passed to @p func.
 * @return A handle to the item added or @c NULL, on errors.
 *
 * This inserts an item before another in the gengrid.
 *
 * @see elm_gengrid_item_append()
 * @see elm_gengrid_item_prepend()
 * @see elm_gengrid_item_insert_after()
 * @see elm_object_item_del()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Item             *elm_gengrid_item_insert_before(Evas_Object *obj, const Elm_Gengrid_Item_Class *gic, const void *data, Elm_Object_Item *relative, Evas_Smart_Cb func, const void *func_data);

/**
 * Insert an item after another in a gengrid widget
 *
 * @param obj The gengrid object.
 * @param gic The item class for the item.
 * @param data The item data.
 * @param relative The item to place this new one after.
 * @param func Convenience function called when the item is
 * selected.
 * @param func_data Data to be passed to @p func.
 * @return A handle to the item added or @c NULL, on errors.
 *
 * This inserts an item after another in the gengrid.
 *
 * @see elm_gengrid_item_append()
 * @see elm_gengrid_item_prepend()
 * @see elm_gengrid_item_insert_after()
 * @see elm_object_item_del()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Item             *elm_gengrid_item_insert_after(Evas_Object *obj, const Elm_Gengrid_Item_Class *gic, const void *data, Elm_Object_Item *relative, Evas_Smart_Cb func, const void *func_data);

/**
 * Insert an item in a gengrid widget using a user-defined sort function.
 *
 * @param obj The gengrid object.
 * @param gic The item class for the item.
 * @param data The item data.
 * @param comp User defined comparison function that defines the sort order
 *             based on gengrid item and its data.
 * @param func Convenience function called when the item is selected.
 * @param func_data Data to be passed to @p func.
 * @return A handle to the item added or @c NULL, on errors.
 *
 * This inserts an item in the gengrid based on user defined comparison
 * function. The two arguments passed to the function @p func are gengrid
 * item handles to compare.
 *
 * @see elm_gengrid_item_append()
 * @see elm_gengrid_item_prepend()
 * @see elm_gengrid_item_insert_after()
 * @see elm_object_item_data_get()
 * @see elm_object_item_del()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Item             *elm_gengrid_item_sorted_insert(Evas_Object *obj, const Elm_Gengrid_Item_Class *gic, const void *data, Eina_Compare_Cb comp, Evas_Smart_Cb func, const void *func_data);

/**
 * Get the selected item in a given gengrid widget
 *
 * @param obj The gengrid object.
 * @return The selected item's handle or @c NULL, if none is
 * selected at the moment (and on errors)
 *
 * This returns the selected item in @p obj. If multi selection is
 * enabled on @p obj (@see elm_gengrid_multi_select_set()), only
 * the first item in the list is selected, which might not be very
 * useful. For that case, see elm_gengrid_selected_items_get().
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Item             *elm_gengrid_selected_item_get(const Evas_Object *obj);

/**
 * Get <b>a list</b> of selected items in a given gengrid
 *
 * @param obj The gengrid object.
 * @return The list of selected items or @c NULL, if none is
 * selected at the moment (and on errors)
 *
 * This returns a list of the selected items, in the order that
 * they appear in the grid. This list is only valid as long as no
 * more items are selected or unselected (or unselected implicitly
 * by deletion). The list contains Gengrid item pointers as
 * data, naturally.
 *
 * @see elm_gengrid_selected_item_get()
 *
 * @ingroup Gengrid
 */
EAPI const Eina_List              *elm_gengrid_selected_items_get(const Evas_Object *obj);

/**
 * Get a list of realized items in gengrid
 *
 * @param obj The gengrid object
 * @return The list of realized items, nor NULL if none are realized.
 *
 * This returns a list of the realized items in the gengrid. The list
 * contains gengrid item pointers. The list must be freed by the
 * caller when done with eina_list_free(). The item pointers in the
 * list are only valid so long as those items are not deleted or the
 * gengrid is not deleted.
 *
 * @see elm_gengrid_realized_items_update()
 *
 * @ingroup Gengrid
 */
EAPI Eina_List                    *elm_gengrid_realized_items_get(const Evas_Object *obj);

/**
 * Update the contents of all realized items.
 *
 * @param obj The gengrid object.
 *
 * This updates all realized items by calling all the item class functions again
 * to get the contents, texts and states. Use this when the original
 * item data has changed and the changes are desired to be reflected.
 *
 * To update just one item, use elm_gengrid_item_update().
 *
 * @see elm_gengrid_realized_items_get()
 * @see elm_gengrid_item_update()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_realized_items_update(Evas_Object *obj);

/**
 * Get the first item in a given gengrid widget
 *
 * @param obj The gengrid object
 * @return The first item's handle or @c NULL, if there are no
 * items in @p obj (and on errors)
 *
 * This returns the first item in the @p obj's internal list of
 * items.
 *
 * @see elm_gengrid_last_item_get()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Item             *elm_gengrid_first_item_get(const Evas_Object *obj);

/**
 * Get the last item in a given gengrid widget
 *
 * @param obj The gengrid object
 * @return The last item's handle or @c NULL, if there are no
 * items in @p obj (and on errors)
 *
 * This returns the last item in the @p obj's internal list of
 * items.
 *
 * @see elm_gengrid_first_item_get()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Item             *elm_gengrid_last_item_get(const Evas_Object *obj);

/**
 * Set the scrollbar policy
 *
 * @param obj The gengrid object
 * @param policy_h Horizontal scrollbar policy.
 * @param policy_v Vertical scrollbar policy.
 *
 * This sets the scrollbar visibility policy for the given gengrid
 * scroller. #ELM_SCROLLER_POLICY_AUTO means the scrollbar is made
 * visible if it is needed, and otherwise kept
 * hidden. #ELM_SCROLLER_POLICY_ON turns it on all the time, and
 * #ELM_SCROLLER_POLICY_OFF always keeps it off.  This applies
 * respectively for the horizontal and vertical scrollbars.  Default
 * is #ELM_SCROLLER_POLICY_AUTO.
 *
 * @deprecated Use elm_scroller_policy_set() instead.
 *
 * @see elm_scroller_policy_set()
 *
 * @see elm_gengrid_scroller_policy_get()
 *
 * @ingroup Gengrid
 */
EINA_DEPRECATED EAPI void          elm_gengrid_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v);

/**
 * Get the scrollbar policy
 *
 * @param obj The gengrid object
 * @param policy_h Pointer to store the horizontal scrollbar policy.
 * @param policy_v Pointer to store the vertical scrollbar policy.
 *
 * @deprecated Use elm_scroller_policy_get() instead.
 *
 * @see elm_scroller_policy_get()
 *
 * @see elm_gengrid_scroller_policy_set()
 *
 * @ingroup Gengrid
 */
EINA_DEPRECATED EAPI void          elm_gengrid_scroller_policy_get(const Evas_Object *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v);

/**
 * Return how many items are currently in a list
 *
 * @param obj The list
 * @return The total number of list items in the list
 *
 * This behavior is O(1) and includes items which may or may not be realized.
 *
 * @ingroup Gengrid
 */
EAPI unsigned int elm_gengrid_items_count(const Evas_Object *obj);

/**
 * Set the size for the items of a given gengrid widget
 *
 * @param obj The gengrid object.
 * @param w The items' width.
 * @param h The items' height.
 *
 * A gengrid, after creation, has still no information on the size
 * to give to each of its cells. So, you most probably will end up
 * with squares one @ref Fingers "finger" wide, the default
 * size. Use this function to force a custom size for you items,
 * making them as big as you wish.
 *
 * @see elm_gengrid_item_size_get()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

/**
 * Get the size set for the items of a given gengrid widget
 *
 * @param obj The gengrid object.
 * @param w Pointer to a variable where to store the items' width.
 * @param h Pointer to a variable where to store the items' height.
 *
 * @note Use @c NULL pointers on the size values you're not
 * interested in: they'll be ignored by the function.
 *
 * @see elm_gengrid_item_size_get() for more details
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_item_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

/**
 * Set the size for the group items of a given gengrid widget
 *
 * @param obj The gengrid object.
 * @param w The group items' width.
 * @param h The group items' height.
 *
 * A gengrid, after creation, has still no information on the size
 * to give to each of its cells. So, you most probably will end up
 * with squares one @ref Fingers "finger" wide, the default
 * size. Use this function to force a custom size for you group items,
 * making them as big as you wish.
 *
 * @see elm_gengrid_group_item_size_get()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_group_item_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

/**
 * Get the size set for the group items of a given gengrid widget
 *
 * @param obj The gengrid object.
 * @param w Pointer to a variable where to store the group items' width.
 * @param h Pointer to a variable where to store the group items' height.
 *
 * @note Use @c NULL pointers on the size values you're not
 * interested in: they'll be ignored by the function.
 *
 * @see elm_gengrid_group_item_size_get() for more details
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_group_item_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

/**
 * Set the items grid's alignment within a given gengrid widget
 *
 * @param obj The gengrid object.
 * @param align_x Alignment in the horizontal axis (0 <= align_x <= 1).
 * @param align_y Alignment in the vertical axis (0 <= align_y <= 1).
 *
 * This sets the alignment of the whole grid of items of a gengrid
 * within its given viewport. By default, those values are both
 * 0.5, meaning that the gengrid will have its items grid placed
 * exactly in the middle of its viewport.
 *
 * @note If given alignment values are out of the cited ranges,
 * they'll be changed to the nearest boundary values on the valid
 * ranges.
 *
 * @see elm_gengrid_align_get()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_align_set(Evas_Object *obj, double align_x, double align_y);

/**
 * Get the items grid's alignment values within a given gengrid
 * widget
 *
 * @param obj The gengrid object.
 * @param align_x Pointer to a variable where to store the
 * horizontal alignment.
 * @param align_y Pointer to a variable where to store the vertical
 * alignment.
 *
 * @note Use @c NULL pointers on the alignment values you're not
 * interested in: they'll be ignored by the function.
 *
 * @see elm_gengrid_align_set() for more details
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_align_get(const Evas_Object *obj, double *align_x, double *align_y);

/**
 * Set whether a given gengrid widget is or not able have items
 * @b reordered
 *
 * @param obj The gengrid object
 * @param reorder_mode Use @c EINA_TRUE to turn reordering on,
 * @c EINA_FALSE to turn it off
 *
 * If a gengrid is set to allow reordering, a click held for more
 * than 0.5 over a given item will highlight it specially,
 * signaling the gengrid has entered the reordering state. From
 * that time on, the user will be able to, while still holding the
 * mouse button down, move the item freely in the gengrid's
 * viewport, replacing to said item to the locations it goes to.
 * The replacements will be animated and, whenever the user
 * releases the mouse button, the item being replaced gets a new
 * definitive place in the grid.
 *
 * @see elm_gengrid_reorder_mode_get()
 *
 * @ingroup Gengrid
 */
EAPI void                          elm_gengrid_reorder_mode_set(Evas_Object *obj, Eina_Bool reorder_mode);

/**
 * Get whether a given gengrid widget is or not able have items
 * @b reordered
 *
 * @param obj The gengrid object
 * @return @c EINA_TRUE, if reordering is on, @c EINA_FALSE if it's
 * off
 *
 * @see elm_gengrid_reorder_mode_set() for more details
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool                     elm_gengrid_reorder_mode_get(const Evas_Object *obj);


/**
 * Set a given gengrid widget's scrolling page size, relative to
 * its viewport size.
 *
 * @param obj The gengrid object
 * @param h_pagerel The horizontal page (relative) size
 * @param v_pagerel The vertical page (relative) size
 *
 * The gengrid's scroller is capable of binding scrolling by the
 * user to "pages". It means that, while scrolling and, specially
 * after releasing the mouse button, the grid will @b snap to the
 * nearest displaying page's area. When page sizes are set, the
 * grid's continuous content area is split into (equal) page sized
 * pieces.
 *
 * This function sets the size of a page <b>relatively to the
 * viewport dimensions</b> of the gengrid, for each axis. A value
 * @c 1.0 means "the exact viewport's size", in that axis, while @c
 * 0.0 turns paging off in that axis. Likewise, @c 0.5 means "half
 * a viewport". Sane usable values are, than, between @c 0.0 and @c
 * 1.0. Values beyond those will make it behave behave
 * inconsistently. If you only want one axis to snap to pages, use
 * the value @c 0.0 for the other one.
 *
 * There is a function setting page size values in @b absolute
 * values, too -- elm_gengrid_page_size_set(). Naturally, its use
 * is mutually exclusive to this one.
 *
 * @deprecated Use elm_scroller_page_relative_set() instead.
 *
 * @see elm_scroller_page_relative_set()
 *
 * @ingroup Gengrid
 */
EINA_DEPRECATED EAPI void          elm_gengrid_page_relative_set(Evas_Object *obj, double h_pagerel, double v_pagerel);

/**
 * Get a given gengrid widget's scrolling page size, relative to
 * its viewport size.
 *
 * @param obj The gengrid object
 * @param h_pagerel Pointer to a variable where to store the
 * horizontal page (relative) size
 * @param v_pagerel Pointer to a variable where to store the
 * vertical page (relative) size
 *
 * @deprecated Use elm_scroller_page_relative_get() instead.
 *
 * @see elm_scroller_page_relative_get()
 *
 * @ingroup Gengrid
 */
EINA_DEPRECATED EAPI void          elm_gengrid_page_relative_get(const Evas_Object *obj, double *h_pagerel, double *v_pagerel);

/**
 * Set a given gengrid widget's scrolling page size
 *
 * @param obj The gengrid object
 * @param h_pagesize The horizontal page size, in pixels
 * @param v_pagesize The vertical page size, in pixels
 *
 * The gengrid's scroller is capable of binding scrolling by the
 * user to "pages". It means that, while scrolling and, specially
 * after releasing the mouse button, the grid will @b snap to the
 * nearest displaying page's area. When page sizes are set, the
 * grid's continuous content area is split into (equal) page sized
 * pieces.
 *
 * This function sets the size of a page of the gengrid, in pixels,
 * for each axis. Sane usable values are, between @c 0 and the
 * dimensions of @p obj, for each axis. Values beyond those will
 * make it behave behave inconsistently. If you only want one axis
 * to snap to pages, use the value @c 0 for the other one.
 *
 * There is a function setting page size values in @b relative
 * values, too -- elm_gengrid_page_relative_set(). Naturally, its
 * use is mutually exclusive to this one.
 *
 * @deprecated Use elm_scroller_page_size_set() instead.
 *
 * @see elm_scroller_page_size_set()
 *
 * @ingroup Gengrid
 */
EINA_DEPRECATED EAPI void          elm_gengrid_page_size_set(Evas_Object *obj, Evas_Coord h_pagesize, Evas_Coord v_pagesize);

/**
 * @brief Get gengrid current page number.
 *
 * @param obj The gengrid object
 * @param h_pagenumber The horizontal page number
 * @param v_pagenumber The vertical page number
 *
 * The page number starts from 0. 0 is the first page.
 * Current page means the page which meet the top-left of the viewport.
 * If there are two or more pages in the viewport, it returns the number of page
 * which meet the top-left of the viewport.
 *
 * @deprecated Use elm_scroller_current_page_set() instead.
 *
 * @see elm_scroller_current_page_set()
 *
 * @see elm_gengrid_last_page_get()
 * @see elm_gengrid_page_show()
 * @see elm_gengrid_page_bring_in()
 */
EINA_DEPRECATED EAPI void          elm_gengrid_current_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber);

/**
 * @brief Get gengrid last page number.
 *
 * @param obj The gengrid object
 * @param h_pagenumber The horizontal page number
 * @param v_pagenumber The vertical page number
 *
 * The page number starts from 0. 0 is the first page.
 * This returns the last page number among the pages.
 *
 * @deprecated Use elm_scroller_last_page_set() instead.
 *
 * @see elm_scroller_last_page_set()
 *
 * @see elm_gengrid_current_page_get()
 * @see elm_gengrid_page_show()
 * @see elm_gengrid_page_bring_in()
 */
EINA_DEPRECATED EAPI void          elm_gengrid_last_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber);

/**
 * Show a specific virtual region within the gengrid content object by page number.
 *
 * @param obj The gengrid object
 * @param h_pagenumber The horizontal page number
 * @param v_pagenumber The vertical page number
 *
 * 0, 0 of the indicated page is located at the top-left of the viewport.
 * This will jump to the page directly without animation.
 *
 * Example of usage:
 *
 * @code
 * sc = elm_gengrid_add(win);
 * elm_gengrid_content_set(sc, content);
 * elm_gengrid_page_relative_set(sc, 1, 0);
 * elm_gengrid_current_page_get(sc, &h_page, &v_page);
 * elm_gengrid_page_show(sc, h_page + 1, v_page);
 * @endcode
 *
 * @see elm_gengrid_page_bring_in()
 */
EAPI void                          elm_gengrid_page_show(const Evas_Object *obj, int h_pagenumber, int v_pagenumber);

/**
 * Show a specific virtual region within the gengrid content object by page number.
 *
 * @param obj The gengrid object
 * @param h_pagenumber The horizontal page number
 * @param v_pagenumber The vertical page number
 *
 * 0, 0 of the indicated page is located at the top-left of the viewport.
 * This will slide to the page with animation.
 *
 * Example of usage:
 *
 * @code
 * sc = elm_gengrid_add(win);
 * elm_gengrid_content_set(sc, content);
 * elm_gengrid_page_relative_set(sc, 1, 0);
 * elm_gengrid_last_page_get(sc, &h_page, &v_page);
 * elm_gengrid_page_bring_in(sc, h_page, v_page);
 * @endcode
 *
 * @deprecated Use elm_scroller_page_bring_in() instead.
 *
 * @see elm_scroller_page_bring_in()
 *
 * @see elm_gengrid_page_show()
 */
EINA_DEPRECATED EAPI void          elm_gengrid_page_bring_in(const Evas_Object *obj, int h_pagenumber, int v_pagenumber);

/**
 * Set how the items grid's filled within a given gengrid widget
 *
 * @param obj The gengrid object.
 * @param fill Filled if True
 *
 * This sets the fill state of the whole grid of items of a gengrid
 * within its given viewport. By default, this value is false, meaning
 * that if the first line of items grid's isn't filled, the items are
 * centered with the alignment
 *
 * @see elm_gengrid_filled_get()
 *
 * @ingroup Gengrid
 *
 */
EAPI void                          elm_gengrid_filled_set(Evas_Object *obj, Eina_Bool fill);

/**
 * Get how the items grid's filled within a given gengrid widget
 *
 * @param obj The gengrid object.
 * @return @c EINA_TRUE, if filled is on, @c EINA_FALSE if it's
 * off
 *
 * @note Use @c NULL pointers on the alignment values you're not
 * interested in: they'll be ignored by the function.
 *
 * @see elm_gengrid_align_set() for more details
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool                     elm_gengrid_filled_get(const Evas_Object *obj);

/**
 * Set the gengrid select mode.
 *
 * @param obj The gengrid object
 * @param mode The select mode
 *
 * elm_gengrid_select_mode_set() changes item select mode in the gengrid widget.
 * - ELM_OBJECT_SELECT_MODE_DEFAULT : Items will only call their selection func and
 *      callback when first becoming selected. Any further clicks will
 *      do nothing, unless you set always select mode.
 * - ELM_OBJECT_SELECT_MODE_ALWAYS :  This means that, even if selected,
 *      every click will make the selected callbacks be called.
 * - ELM_OBJECT_SELECT_MODE_NONE : This will turn off the ability to select items
 *      entirely and they will neither appear selected nor call selected
 *      callback functions.
 *
 * @see elm_gengrid_select_mode_get()
 *
 * @ingroup Gengrid
 */
EAPI void elm_gengrid_select_mode_set(Evas_Object *obj, Elm_Object_Select_Mode mode);

/**
 * Get the gengrid select mode.
 *
 * @param obj The gengrid object
 * @return The select mode
 * (If getting mode is failed, it returns ELM_OBJECT_SELECT_MODE_MAX)
 *
 * @see elm_gengrid_select_mode_set()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Select_Mode elm_gengrid_select_mode_get(const Evas_Object *obj);

/**
 * Set whether the gengrid items' should be highlighted when item selected.
 *
 * @param obj The gengrid object.
 * @param highlight @c EINA_TRUE to enable highlight or @c EINA_FALSE to
 * disable it.
 *
 * This will turn on/off the highlight effect when items are selected and
 * they will or will not be highlighted. The selected and clicked
 * callback functions will still be called.
 *
 * highlight is enabled by default.
 *
 * @see elm_gengrid_highlight_mode_get().
 *
 * @ingroup Gengrid
 */

EAPI void                          elm_gengrid_highlight_mode_set(Evas_Object *obj, Eina_Bool highlight);

/**
 * Get whether the gengrid items' should be highlighted when item selected.
 *
 * @param obj The gengrid object.
 * @return @c EINA_TRUE means items can be highlighted. @c EINA_FALSE indicates
 * they can't. If @p obj is @c NULL, @c EINA_FALSE is returned.
 *
 * @see elm_gengrid_highlight_mode_set() for details.
 *
 * @ingroup Gengrid
 */

EAPI Eina_Bool                     elm_gengrid_highlight_mode_get(const Evas_Object *obj);

/**
 * Get the nth item, in a given gengrid widget, placed at position @p nth, in
 * its internal items list
 *
 * @param obj The gengrid object
 * @param nth The number of the item to grab (0 being the first)
 *
 * @return The item stored in @p obj at position @p nth or @c NULL, if there's
 * no item with that index (and on errors)
 *
 * @ingroup Genilst
 * @since 1.8
 */
EAPI Elm_Object_Item *elm_gengrid_nth_item_get(const Evas_Object *obj, unsigned int nth);

/**
 * Get the item that is at the x, y canvas coords.
 *
 * @param obj The gengrid object.
 * @param x The input x coordinate
 * @param y The input y coordinate
 * @param xposret The position relative to the item returned here
 * @param yposret The position relative to the item returned here
 * @return The item at the coordinates or NULL if none
 *
 * This returns the item at the given coordinates (which are canvas
 * relative, not object-relative). If an item is at that coordinate,
 * that item handle is returned, and if @p xposret is not NULL, the
 * integer pointed to is set to a value of -1, 0 or 1, depending if
 * the coordinate is on the left portion of that item (-1), on the
 * middle section (0) or on the right part (1).
 * if @p yposret is not NULL, the
 * integer pointed to is set to a value of -1, 0 or 1, depending if
 * the coordinate is on the upper portion of that item (-1), on the
 * middle section (0) or on the lower part (1). If NULL is returned as
 * an item (no item found there), then posret may indicate -1 or 1
 * based if the coordinate is above or below all items respectively in
 * the gengrid.
 *
 * @ingroup Gengrid
 */
EAPI Elm_Object_Item             *elm_gengrid_at_xy_item_get(const Evas_Object *obj, Evas_Coord x, Evas_Coord y, int *xposret, int *yposret);

