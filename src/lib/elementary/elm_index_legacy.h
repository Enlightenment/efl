/**
 * Add a new index widget to the given parent Elementary
 * (container) object
 *
 * @param parent The parent object
 * @return a new index widget handle or @c NULL, on errors
 *
 * This function inserts a new index widget on the canvas.
 *
 * @ingroup Elm_Index
 */
EAPI Evas_Object          *elm_index_add(Evas_Object *parent);

/**
 * @brief Enable or disable horizontal mode on the index object
 *
 * @note Vertical mode is set by default.
 *
 * On horizontal mode items are displayed on index from left to right, instead
 * of from top to bottom. Also, the index will scroll horizontally.
 *
 * @param[in] horizontal @c true to enable horizontal or @c false to disable
 * it, i.e., to enable vertical mode. it's an area one Fingers "finger" wide on
 * the bottom side of the index widget's container.
 *
 * @ingroup Elm_Index
 */
EAPI void elm_index_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * @brief Get enable or disable status of horizontal mode on the index object.
 *
 * @note Vertical mode is set by default.
 *
 * Returns the current status of horizontal mode on index object.
 * On horizontal mode items are displayed on index from left to right, instead
 * of from top to bottom. Also, the index will scroll horizontally.
 *
 * @return Current status of horizontal mode on index object.
 * @c true if horizontal mode is enabled or @c false if disabled.
 *
 * @ingroup Elm_Index
 */
EAPI Eina_Bool elm_index_horizontal_get(const Evas_Object *obj);

#include "elm_index_item_eo.legacy.h"
#include "elm_index_eo.legacy.h"
