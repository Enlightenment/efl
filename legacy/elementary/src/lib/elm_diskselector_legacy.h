/**
 * Add a new diskselector widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new diskselector widget handle or @c NULL, on errors.
 *
 * This function inserts a new diskselector widget on the canvas.
 *
 * @ingroup Diskselector
 */
EAPI Evas_Object           *elm_diskselector_add(Evas_Object *parent);

/**
 * Get a value whether round mode is enabled or not.
 *
 * @see elm_diskselector_round_enabled_set() for details.
 *
 * @param obj The diskselector object.
 * @return @c EINA_TRUE means round mode is enabled. @c EINA_FALSE indicates
 * it's disabled. If @p obj is @c NULL, @c EINA_FALSE is returned.
 *
 * @ingroup Diskselector
 */
EAPI Eina_Bool              elm_diskselector_round_enabled_get(const Evas_Object *obj);

/**
 * Enable or disable round mode.
 *
 * @param obj The diskselector object.
 * @param enabled @c EINA_TRUE to enable round mode or @c EINA_FALSE to
 * disable it.
 *
 * Disabled by default. If round mode is enabled the items list will
 * work like a circular list, so when the user reaches the last item,
 * the first one will popup.
 *
 * @see elm_diskselector_round_enabled_get()
 *
 * @ingroup Diskselector
 */
EAPI void                   elm_diskselector_round_enabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * Get the side labels max length.
 *
 * @see elm_diskselector_side_text_max_length_set() for details.
 *
 * @param obj The diskselector object.
 * @return The max length defined for side labels, or 0 if not a valid
 * diskselector.
 *
 * @ingroup Diskselector
 */
EAPI int                    elm_diskselector_side_text_max_length_get(const Evas_Object *obj);

/**
 * Set the side labels max length.
 *
 * @param obj The diskselector object.
 * @param len The max length defined for side labels.
 *
 * Length is the number of characters of items' label that will be
 * visible when it's set on side positions. It will just crop
 * the string after defined size. E.g.:
 *
 * An item with label "January" would be displayed on side position as
 * "Jan" if max length is set to 3, or "Janu", if this property
 * is set to 4.
 *
 * When it's selected, the entire label will be displayed, except for
 * width restrictions. In this case label will be cropped and "..."
 * will be concatenated.
 *
 * Default side label max length is 3.
 *
 * This property will be applied over all items, included before or
 * later this function call.
 *
 * @ingroup Diskselector
 */
EAPI void                   elm_diskselector_side_text_max_length_set(Evas_Object *obj, int len);

/**
 * Remove all diskselector's items.
 *
 * @param obj The diskselector object.
 *
 * @see elm_object_item_del()
 * @see elm_diskselector_item_append()
 *
 * @ingroup Diskselector
 */
EAPI void                   elm_diskselector_clear(Evas_Object *obj);

/**
 * Get a list of all the diskselector items.
 *
 * @param obj The diskselector object.
 * @return An @c Eina_List of diskselector items, #Elm_Object_Item,
 * or @c NULL on failure.
 *
 * @see elm_diskselector_item_append()
 * @see elm_object_item_del()
 * @see elm_diskselector_clear()
 *
 * @ingroup Diskselector
 */
EAPI const Eina_List       *elm_diskselector_items_get(const Evas_Object *obj);

/**
 * Appends a new item to the diskselector object.
 *
 * @param obj The diskselector object.
 * @param label The label of the diskselector item.
 * @param icon The icon object to use at left side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 * @param func The function to call when the item is selected.
 * @param data The data to associate with the item for related callbacks.
 *
 * @return The created item or @c NULL upon failure.
 *
 * A new item will be created and appended to the diskselector, i.e., will
 * be set as last item. Also, if there is no selected item, it will
 * be selected. This will always happens for the first appended item.
 *
 * If no icon is set, label will be centered on item position, otherwise
 * the icon will be placed at left of the label, that will be shifted
 * to the right.
 *
 * Items created with this method can be deleted with
 * elm_object_item_del().
 *
 * Associated @p data can be properly freed when item is deleted if a
 * callback function is set with elm_object_item_del_cb_set().
 *
 * If a function is passed as argument, it will be called every time this item
 * is selected, i.e., the user stops the diskselector with this
 * item on center position. If such function isn't needed, just passing
 * @c NULL as @p func is enough. The same should be done for @p data.
 *
 * Simple example (with no function callback or data associated):
 * @code
 * disk = elm_diskselector_add(win);
 * ic = elm_icon_add(win);
 * elm_image_file_set(ic, "path/to/image", NULL);
 * elm_icon_resizable_set(ic, EINA_TRUE, EINA_TRUE);
 * elm_diskselector_item_append(disk, "label", ic, NULL, NULL);
 * @endcode
 *
 * @see elm_object_item_del()
 * @see elm_diskselector_clear()
 * @see elm_icon_add()
 *
 * @ingroup Diskselector
 */
EAPI Elm_Object_Item *elm_diskselector_item_append(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Smart_Cb func, const void *data);

/**
 * Get the selected item.
 *
 * @param obj The diskselector object.
 * @return The selected diskselector item.
 *
 * The selected item can be unselected with function
 * elm_diskselector_item_selected_set(), and the first item of
 * diskselector will be selected.
 *
 * The selected item always will be centered on diskselector, with
 * full label displayed, i.e., max length set to side labels won't
 * apply on the selected item. More details on
 * elm_diskselector_side_text_max_length_set().
 *
 * @ingroup Diskselector
 */
EAPI Elm_Object_Item *elm_diskselector_selected_item_get(const Evas_Object *obj);

/**
 * Get the first item of the diskselector.
 *
 * @param obj The diskselector object.
 * @return The first item, or @c NULL if none.
 *
 * The list of items follows append order. So it will return the first
 * item appended to the widget that wasn't deleted.
 *
 * @see elm_diskselector_item_append()
 * @see elm_diskselector_items_get()
 *
 * @ingroup Diskselector
 */
EAPI Elm_Object_Item *elm_diskselector_first_item_get(const Evas_Object *obj);

/**
 * Get the last item of the diskselector.
 *
 * @param obj The diskselector object.
 * @return The last item, or @c NULL if none.
 *
 * The list of items follows append order. So it will return last first
 * item appended to the widget that wasn't deleted.
 *
 * @see elm_diskselector_item_append()
 * @see elm_diskselector_items_get()
 *
 * @ingroup Diskselector
 */
EAPI Elm_Object_Item *elm_diskselector_last_item_get(const Evas_Object *obj);

/**
 * Set the number of items to be displayed.
 *
 * @param obj The diskselector object.
 * @param num The number of items the diskselector will display.
 *
 * Default value is 3, and also it's the minimum. If @p num is less
 * than 3, it will be set to 3.
 *
 * Also, it can be set on theme, using data item @c display_item_num
 * on group "elm/diskselector/item/X", where X is style set.
 * E.g.:
 *
 * group { name: "elm/diskselector/item/X";
 * data {
 *     item: "display_item_num" "5";
 *     }
 *
 * @ingroup Diskselector
 */
EAPI void                   elm_diskselector_display_item_num_set(Evas_Object *obj, int num);

/**
 * Get the number of items in the diskselector object.
 *
 * @param obj The diskselector object.
 *
 * @ingroup Diskselector
 */
EAPI int                   elm_diskselector_display_item_num_get(const Evas_Object *obj);
