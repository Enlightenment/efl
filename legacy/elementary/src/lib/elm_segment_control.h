/**
 * @defgroup SegmentControl SegmentControl
 * @ingroup Elementary
 *
 * @image html segment_control_inheritance_tree.png
 * @image latex segment_control_inheritance_tree.eps
 *
 * @image html img/widget/segment_control/preview-00.png
 * @image latex img/widget/segment_control/preview-00.eps width=\textwidth
 *
 * @image html img/segment_control.png
 * @image latex img/segment_control.eps width=\textwidth
 *
 * Segment control widget is a horizontal control made of multiple segment
 * items, each segment item functioning similar to discrete two state button.
 * A segment control groups the items together and provides compact
 * single button with multiple equal size segments.
 *
 * Segment item size is determined by base widget
 * size and the number of items added.
 * Only one segment item can be at selected state. A segment item can display
 * combination of Text and any Evas_Object like Images or other widget.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for segment control objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "changed" - When the user clicks on a segment item which is not
 *   previously selected and get selected. The event_info parameter is the
 *   segment item pointer.
 *
 * Available styles for it:
 * - @c "default"
 *
 * Default content parts of the segment control items that you can use for are:
 * @li "icon" - An icon in a segment control item
 *
 * Default text parts of the segment control items that you can use for are:
 * @li "default" - Title label in a segment control item
 *
 * Supported elm_object common APIs.
 * @li elm_object_disabled_set
 * @li elm_object_disabled_get
 *
 * Supported elm_object_item common APIs.
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 * @li @ref elm_object_item_part_content_set
 * @li @ref elm_object_item_part_content_get
 *
 * Here is an example on its usage:
 * @li @ref segment_control_example
 *
 */

/**
 * @addtogroup SegmentControl
 * @{
 */

/**
 * Add a new segment control widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new segment control widget handle or @c NULL, on errors.
 *
 * This function inserts a new segment control widget on the canvas.
 *
 * @ingroup SegmentControl
 */
EAPI Evas_Object      *elm_segment_control_add(Evas_Object *parent);

/**
 * Append a new item to the segment control object.
 *
 * @param obj The segment control object.
 * @param icon The icon object to use for the left side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 * @param label The label of the item.
 *        Note that, NULL is different from empty string "".
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and appended to the segment control, i.e., will
 * be set as @b last item.
 *
 * If it should be inserted at another position,
 * elm_segment_control_item_insert_at() should be used instead.
 *
 * Items created with this function can be deleted with function
 * elm_object_item_del() or elm_object_item_del_at().
 *
 * @note @p label set to @c NULL is different from empty string "".
 * If an item
 * only has icon, it will be displayed bigger and centered. If it has
 * icon and label, even that an empty string, icon will be smaller and
 * positioned at left.
 *
 * Simple example:
 * @code
 * sc = elm_segment_control_add(win);
 * ic = elm_icon_add(win);
 * elm_image_file_set(ic, "path/to/image", NULL);
 * elm_icon_resizable_set(ic, EINA_TRUE, EINA_TRUE);
 * elm_segment_control_item_add(sc, ic, "label");
 * evas_object_show(sc);
 * @endcode
 *
 * @see elm_segment_control_item_insert_at()
 * @see elm_object_item_del()
 *
 * @ingroup SegmentControl
 */
EAPI Elm_Object_Item *elm_segment_control_item_add(Evas_Object *obj, Evas_Object *icon, const char *label);

/**
 * Insert a new item to the segment control object at specified position.
 *
 * @param obj The segment control object.
 * @param icon The icon object to use for the left side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 * @param label The label of the item.
 * @param index Item position. Value should be between 0 and items count.
 * @return The created item or @c NULL upon failure.

 * Index values must be between @c 0, when item will be prepended to
 * segment control, and items count, that can be get with
 * elm_segment_control_item_count_get(), case when item will be appended
 * to segment control, just like elm_segment_control_item_add().
 *
 * Items created with this function can be deleted with function
 * elm_object_item_del() or elm_segment_control_item_del_at().
 *
 * @note @p label set to @c NULL is different from empty string "".
 * If an item
 * only has icon, it will be displayed bigger and centered. If it has
 * icon and label, even that an empty string, icon will be smaller and
 * positioned at left.
 *
 * @see elm_segment_control_item_add()
 * @see elm_segment_control_item_count_get()
 * @see elm_object_item_del()
 *
 * @ingroup SegmentControl
 */
EAPI Elm_Object_Item *elm_segment_control_item_insert_at(Evas_Object *obj, Evas_Object *icon, const char *label, int index);

/**
 * Remove a segment control item at given index from its parent,
 * deleting it.
 *
 * @param obj The segment control object.
 * @param index The position of the segment control item to be deleted.
 *
 * Items can be added with elm_segment_control_item_add() or
 * elm_segment_control_item_insert_at().
 *
 * @ingroup SegmentControl
 */
EAPI void              elm_segment_control_item_del_at(Evas_Object *obj, int index);

/**
 * Get the Segment items count from segment control.
 *
 * @param obj The segment control object.
 * @return Segment items count.
 *
 * It will just return the number of items added to segment control @p obj.
 *
 * @ingroup SegmentControl
 */
EAPI int               elm_segment_control_item_count_get(const Evas_Object *obj);

/**
 * Get the item placed at specified index.
 *
 * @param obj The segment control object.
 * @param index The index of the segment item.
 * @return The segment control item or @c NULL on failure.
 *
 * Index is the position of an item in segment control widget. Its
 * range is from @c 0 to <tt> count - 1 </tt>.
 * Count is the number of items, that can be get with
 * elm_segment_control_item_count_get().
 *
 * @ingroup SegmentControl
 */
EAPI Elm_Object_Item *elm_segment_control_item_get(const Evas_Object *obj, int index);

/**
 * Get the label of item.
 *
 * @param obj The segment control object.
 * @param index The index of the segment item.
 * @return The label of the item at @p index.
 *
 * The return value is a pointer to the label associated to the item when
 * it was created, with function elm_segment_control_item_add(), or later
 * with function elm_object_item_text_set. If no label
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_object_item_text_set() for more details.
 * @see elm_segment_control_item_add()
 *
 * @ingroup SegmentControl
 */
EAPI const char       *elm_segment_control_item_label_get(const Evas_Object *obj, int index);

/**
 * Get the icon associated to the item.
 *
 * @param obj The segment control object.
 * @param index The index of the segment item.
 * @return The left side icon associated to the item at @p index.
 *
 * The return value is a pointer to the icon associated to the item when
 * it was created, with function elm_segment_control_item_add(), or later
 * with function elm_object_item_part_content_set(). If no icon
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_segment_control_item_add()
 * @see elm_object_item_part_content_set()
 *
 * @ingroup SegmentControl
 */
EAPI Evas_Object      *elm_segment_control_item_icon_get(const Evas_Object *obj, int index);

/**
 * Get the index of an item.
 *
 * @param it The segment control item.
 * @return The position of item in segment control widget.
 *
 * Index is the position of an item in segment control widget. Its
 * range is from @c 0 to <tt> count - 1 </tt>.
 * Count is the number of items, that can be get with
 * elm_segment_control_item_count_get().
 *
 * @ingroup SegmentControl
 */
EAPI int               elm_segment_control_item_index_get(const Elm_Object_Item *it);

/**
 * Get the base object of the item.
 *
 * @param it The segment control item.
 * @return The base object associated with @p it.
 *
 * Base object is the @c Evas_Object that represents that item.
 *
 * @ingroup SegmentControl
 */
EAPI Evas_Object      *elm_segment_control_item_object_get(const Elm_Object_Item *it);

/**
 * Get the selected item.
 *
 * @param obj The segment control object.
 * @return The selected item or @c NULL if none of segment items is
 * selected.
 *
 * The selected item can be unselected with function
 * elm_segment_control_item_selected_set().
 *
 * The selected item always will be highlighted on segment control.
 *
 * @ingroup SegmentControl
 */
EAPI Elm_Object_Item *elm_segment_control_item_selected_get(const Evas_Object *obj);

/**
 * Set the selected state of an item.
 *
 * @param it The segment control item
 * @param select The selected state
 *
 * This sets the selected state of the given item @p it.
 * @c EINA_TRUE for selected, @c EINA_FALSE for not selected.
 *
 * If a new item is selected the previously selected will be unselected.
 * Selected item can be got with function
 * elm_segment_control_item_selected_get().
 *
 * The selected item always will be highlighted on segment control.
 *
 * @see elm_segment_control_item_selected_get()
 *
 * @ingroup SegmentControl
 */
EAPI void              elm_segment_control_item_selected_set(Elm_Object_Item *it, Eina_Bool select);

/**
 * @}
 */
