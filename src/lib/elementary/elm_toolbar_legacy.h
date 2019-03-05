/**
 * Add a new toolbar widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new toolbar widget handle or @c NULL, on errors.
 *
 * This function inserts a new toolbar widget on the canvas.
 *
 * @ingroup Elm_Toolbar
 */
EAPI Evas_Object                 *elm_toolbar_add(Evas_Object *parent);

/**
 * Set reorder mode
 *
 * @param obj The toolbar object
 * @param reorder_mode The reorder mode
 * (@c EINA_TRUE = on, @c EINA_FALSE = off)
 *
 * @ingroup Elm_Toolbar
 */
EAPI void                          elm_toolbar_reorder_mode_set(Evas_Object *obj, Eina_Bool reorder_mode);

/**
 * Get the reorder mode
 *
 * @param obj The toolbar object
 * @return The reorder mode
 * (@c EINA_TRUE = on, @c EINA_FALSE = off)
 *
 * @ingroup Elm_Toolbar
 */
EAPI Eina_Bool                     elm_toolbar_reorder_mode_get(const Evas_Object *obj);

/**
 * Set the item's transverse expansion of a given toolbar widget @p obj.
 *
 * @param obj The toolbar object.
 * @param transverse_expanded The transverse expansion of the item.
 * (@c EINA_TRUE = on, @c EINA_FALSE = off, default = @c EINA_FALSE)
 *
 * This will expand the transverse length of the item according the transverse length of the toolbar.
 * The default is what the transverse length of the item is set according its min value.
 *
 * @ingroup Elm_Toolbar
 */
EAPI void                         elm_toolbar_transverse_expanded_set(Evas_Object *obj, Eina_Bool transverse_expanded);

/**
 * Get the transverse expansion of toolbar @p obj.
 *
 * @param obj The toolbar object.
 * @return The transverse expansion of the item.
 * (@c EINA_TRUE = on, @c EINA_FALSE = off, default = @c EINA_FALSE)
 *
 * @see elm_toolbar_transverse_expand_set() for details.
 *
 * @ingroup Elm_Toolbar
 */
EAPI Eina_Bool                    elm_toolbar_transverse_expanded_get(const Evas_Object *obj);

/**
 * Sets icon lookup order, for toolbar items' icons.
 *
 * Icons added before calling this function will not be affected. The default
 * lookup order is #ELM_ICON_LOOKUP_THEME_FDO.
 *
 * @param[in] order The icon lookup order. (If getting the icon order loopup
 * fails, it returns #ELM_ICON_LOOKUP_THEME_FDO)
 *
 * @ingroup Elm_Toolbar
 */
EAPI void elm_toolbar_icon_order_lookup_set(Evas_Object *obj EINA_UNUSED, Elm_Icon_Lookup_Order order EINA_UNUSED);

/**
 * Gets icon lookup order, for toolbar items' icons.
 *
 * Icons added before calling this function will not be affected. The default
 * lookup order is #ELM_ICON_LOOKUP_THEME_FDO.
 *
 * @return The icon lookup order. (If getting the icon order loopup fails, it
 * returns #ELM_ICON_LOOKUP_THEME_FDO)
 *
 * @ingroup Elm_Toolbar
 */
EAPI Elm_Icon_Lookup_Order elm_toolbar_icon_order_lookup_get(const Evas_Object *obj EINA_UNUSED);

/**
 * @brief Change a toolbar's orientation
 *
 * By default, a toolbar will be horizontal. Use this function to create a
 * vertical toolbar.
 *
 * @param[in] horizontal If @c true, the toolbar is horizontal.
 *
 * @ingroup Elm_Toolbar
 */
EAPI void elm_toolbar_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * @brief Get toolbar's current orientation
 *
 * Use this function to get orientation of toolbar.
 *
 * @return If @c true, the toolbar is horizontal.
 *
 * @ingroup Elm_Toolbar
 */
EAPI Eina_Bool elm_toolbar_horizontal_get(const Evas_Object *obj);

#include "elm_toolbar_item_eo.legacy.h"
#include "elm_toolbar_eo.legacy.h"
