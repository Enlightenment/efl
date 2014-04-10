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

#include "elm_toolbar.eo.legacy.h"