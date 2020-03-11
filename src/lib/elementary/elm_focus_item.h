/**
 * Get the focused object item
 *
 * This returns the focused object item.
 *
 * @param obj The container object
 * @return The focused item, or @c NULL if none
 *
 * The focused item can be unfocused with function
 * elm_object_item_focus_set().
 *
 * @see elm_object_item_focus_set()
 * @see elm_object_item_focus_get()
 *
 * @ingroup Elm_Focus
 * @since 1.10
 */
EAPI Elm_Object_Item             *elm_object_focused_item_get(const Evas_Object *obj);

/**
 * Get next object item which was set with specific focus direction.
 *
 * Get next object item which was set by elm_object_focus_next_item_set
 * with specific focus direction.
 *
 * @param obj The Elementary object
 * @param dir Focus direction
 * @return Focus next object item or @c NULL, if there is no focus next
 * object item.
 *
 * @see elm_object_focus_next_item_set(), elm_object_focus_next()
 *
 * @since 1.16
 *
 * @ingroup Elm_Focus
 */
EAPI Elm_Object_Item *    elm_object_focus_next_item_get(const Evas_Object *obj, Elm_Focus_Direction dir);

/**
 * Set next object item with specific focus direction.
 *
 * When focus next object item is set with specific focus direction,
 * this object item will be the first candidate when finding
 * next focusable object or item.
 * If the focus next object item is set, it is preference to focus next object.
 * Focus next object item can be registered with six directions that are
 * previous, next, up, down, right, and left.
 *
 * @param obj The Elementary object
 * @param next_item Focus next object item
 * @param dir Focus direction
 *
 * @see elm_object_focus_next_item_get(), elm_object_focus_next()
 *
 * @since 1.16
 *
 * @ingroup Elm_Focus
 */
EAPI void                 elm_object_focus_next_item_set(Evas_Object *obj, Elm_Object_Item *next_item, Elm_Focus_Direction dir);


