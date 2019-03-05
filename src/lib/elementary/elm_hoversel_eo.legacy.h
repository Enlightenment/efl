#ifndef _ELM_HOVERSEL_EO_LEGACY_H_
#define _ELM_HOVERSEL_EO_LEGACY_H_

#ifndef _ELM_HOVERSEL_EO_CLASS_TYPE
#define _ELM_HOVERSEL_EO_CLASS_TYPE

typedef Eo Elm_Hoversel;

#endif

#ifndef _ELM_HOVERSEL_EO_TYPES
#define _ELM_HOVERSEL_EO_TYPES


#endif

/**
 * @brief Control if the hoversel should expand horizontally.
 *
 * @note The initial button will display horizontally regradless of this
 * setting.
 *
 * @param[in] obj The object.
 * @param[in] horizontal If @c true, the hover will expand horizontally to the
 * right.
 *
 * @ingroup Elm_Hoversel_Group
 */
EAPI void elm_hoversel_horizontal_set(Elm_Hoversel *obj, Eina_Bool horizontal);

/**
 * @brief Control if the hoversel should expand horizontally.
 *
 * @note The initial button will display horizontally regradless of this
 * setting.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the hover will expand horizontally to the right.
 *
 * @ingroup Elm_Hoversel_Group
 */
EAPI Eina_Bool elm_hoversel_horizontal_get(const Elm_Hoversel *obj);

/**
 * @brief Control the hover parent.
 *
 * Sets the hover parent object, the area that will be darkened when the
 * hoversel is clicked. Should probably be the window that the hoversel is in.
 *
 * @param[in] obj The object.
 * @param[in] parent The parent to use
 *
 * @ingroup Elm_Hoversel_Group
 */
EAPI void elm_hoversel_hover_parent_set(Elm_Hoversel *obj, Efl_Canvas_Object *parent);

/**
 * @brief Control the hover parent.
 *
 * Sets the hover parent object, the area that will be darkened when the
 * hoversel is clicked. Should probably be the window that the hoversel is in.
 *
 * @param[in] obj The object.
 *
 * @return The parent to use
 *
 * @ingroup Elm_Hoversel_Group
 */
EAPI Efl_Canvas_Object *elm_hoversel_hover_parent_get(const Elm_Hoversel *obj);

/**
 * @brief Returns whether the hoversel is expanded.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the hoversel is expanded or @c false otherwise
 *
 * @ingroup Elm_Hoversel_Group
 */
EAPI Eina_Bool elm_hoversel_expanded_get(const Elm_Hoversel *obj);

/**
 * @brief Get the list of items within the given hoversel.
 *
 * @param[in] obj The object.
 *
 * @return List of items
 *
 * @ingroup Elm_Hoversel_Group
 */
EAPI const Eina_List *elm_hoversel_items_get(const Elm_Hoversel *obj);

/**
 * @brief Update icon and text of hoversel same to those of selected item
 * automatically.
 *
 * @param[in] obj The object.
 * @param[in] auto_update @c true if hoversel is updated automatically or
 * @c false otherwise
 *
 * @ingroup Elm_Hoversel_Group
 */
EAPI void elm_hoversel_auto_update_set(Elm_Hoversel *obj, Eina_Bool auto_update);

/**
 * @brief Update icon and text of hoversel same to those of selected item
 * automatically.
 *
 * @param[in] obj The object.
 *
 * @return @c true if hoversel is updated automatically or @c false otherwise
 *
 * @ingroup Elm_Hoversel_Group
 */
EAPI Eina_Bool elm_hoversel_auto_update_get(const Elm_Hoversel *obj);

/** This triggers the hoversel popup from code, the same as if the user had
 * clicked the button.
 *
 * @ingroup Elm_Hoversel_Group
 */
EAPI void elm_hoversel_hover_begin(Elm_Hoversel *obj);

/**
 * @brief This will remove all the children items from the hoversel.
 *
 * Warning Should *not* be called while the hoversel is active; use
 * @ref elm_hoversel_expanded_get to check first.
 * @param[in] obj The object.
 *
 * @ingroup Elm_Hoversel_Group
 */
EAPI void elm_hoversel_clear(Elm_Hoversel *obj);

/** This dismisses the hoversel popup as if the user had clicked outside the
 * hover.
 *
 * @ingroup Elm_Hoversel_Group
 */
EAPI void elm_hoversel_hover_end(Elm_Hoversel *obj);

/**
 * @brief Add an item to the hoversel button
 *
 * This adds an item to the hoversel to show when it is clicked. Note: if you
 * need to use an icon from an edje file then use Elm.Hoversel.Item.icon.set
 * right after this function, and set icon_file to @c null here.
 *
 * @param[in] obj The object.
 * @param[in] label The text label to use for the item (NULL if not desired)
 * @param[in] icon_file An image file path on disk to use for the icon or
 * standard icon name (NULL if not desired)
 * @param[in] icon_type The icon type if relevant
 * @param[in] func Convenience function to call when this item is selected. The
 * last parameter @c event_info of @c func is the selected item pointer.
 * @param[in] data Data to pass to item-related functions
 *
 * @return A handle to the added item.
 *
 * @ingroup Elm_Hoversel_Group
 */
EAPI Elm_Widget_Item *elm_hoversel_item_add(Elm_Hoversel *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);

#endif
