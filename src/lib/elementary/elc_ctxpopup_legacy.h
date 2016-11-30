/**
 * @brief Add a new Ctxpopup object to the parent.
 *
 * @param parent Parent object
 * @return New object or @c NULL, if it cannot be created
 *
 * @ingroup Elm_Ctxpopup
 */
EAPI Evas_Object                 *elm_ctxpopup_add(Evas_Object *parent);

/**
 * @brief Change the ctxpopup's orientation to horizontal or vertical.
 *
 * @param obj Elm Ctxpopup object
 * @param horizontal @c EINA_TRUE for horizontal mode, @c EINA_FALSE for vertical.
 *
 * @ingroup Elm_Ctxpopup
 */
EAPI void                         elm_ctxpopup_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * @brief Get the value of current ctxpopup object's orientation.
 *
 * @param obj Elm Ctxpopup object
 * return @c EINA_TRUE for horizontal mode, @c EINA_FALSE for vertical.
 * See also @ref elm_ctxpopup_horizontal_set
 *
 * @ingroup Elm_Ctxpopup
 */
EAPI Eina_Bool                    elm_ctxpopup_horizontal_get(const Evas_Object *obj);

/**
 * @brief Get the internal list of items in a given ctxpopup widget.
 *
 * This list is not to be modified in any way and must not be freed. Use the
 * list members with functions like @ref elm_object_item_text_set, @ref
 * elm_object_item_text_get, @ref elm_object_item_del.
 *
 * @warning This list is only valid until @c obj object's internal items list
 * is changed. It should be fetched again with another call to this function
 * when changes happen.
 *
 * @return The list of items or @c null on errors.
 *
 * @since 1.11
 *
 * @ingroup Elm_Ctxpopup
 */
EAPI const Eina_List *elm_ctxpopup_items_get(const Evas_Object *obj);

/**
 * @brief Get the first item in the given ctxpopup widget's list of items.
 *
 * See also  @ref elm_obj_ctxpopup_item_append,
 * @ref elm_obj_ctxpopup_last_item_get.
 *
 * @return The first item or @c null, if it has no items (and on errors).
 *
 * @since 1.11
 *
 * @ingroup Elm_Ctxpopup
 */
EAPI Elm_Widget_Item *elm_ctxpopup_first_item_get(const Evas_Object *obj);

/**
 * @brief Get the last item in the given ctxpopup widget's list of items.
 *
 * See also @ref elm_obj_ctxpopup_item_prepend,
 * @ref elm_obj_ctxpopup_first_item_get.
 *
 * @return The last item or @c null, if it has no items (and on errors).
 *
 * @since 1.1
 *
 * @ingroup Elm_Ctxpopup
 */
EAPI Elm_Widget_Item *elm_ctxpopup_last_item_get(const Evas_Object *obj);

/**
 * @brief Get the item before $ it in a ctxpopup widget's internal list of
 * items.
 *
 * See also @ref elm_ctxpopup_item_next_get.
 *
 * @return The item before the object in its parent's list. If there is no
 * previous item for $ it or there's an error, @c null is returned.
 *
 * @since 1.11
 *
 * @ingroup Elm_Ctxpopup_Item
 */
EAPI Elm_Widget_Item *elm_ctxpopup_item_prev_get(const Evas_Object *obj);

/**
 * @brief Get the item after $ it in a ctxpopup widget's internal list of
 * items.
 *
 * See also @ref elm_ctxpopup_item_prev_get.
 *
 * @return The item after the object in its parent's list. If there is no
 * previous item for $ it or there's an error, @c null is returned.
 *
 * @since 1.11
 *
 * @ingroup Elm_Ctxpopup_Item
 */
EAPI Elm_Widget_Item *elm_ctxpopup_item_next_get(const Evas_Object *obj);

#include "elm_ctxpopup_item.eo.legacy.h"
#include "elm_ctxpopup.eo.legacy.h"
