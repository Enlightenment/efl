/**
 * Add a new flip selector widget to the given parent Elementary
 * (container) widget
 *
 * @param parent The parent object
 * @return a new flip selector widget handle or @c NULL, on errors
 *
 * This function inserts a new flip selector widget on the canvas.
 *
 * @ingroup Elm_Flipselector
 */
EAPI Evas_Object                *elm_flipselector_add(Evas_Object *parent);

/**
 * @brief Set the interval on time updates for a user mouse button hold on a
 * flip selector widget.
 *
 * This interval value is decreased while the user holds the mouse pointer
 * either flipping up or flipping down a given flip selector.
 *
 * This helps the user to get to a given item distant from the current one
 * easier/faster, as it will start to flip quicker and quicker on mouse button
 * holds.
 *
 * The calculation for the next flip interval value, starting from the one set
 * with this call, is the previous interval divided by 1.05, so it decreases a
 * little bit.
 *
 * The default starting interval value for automatic flips is 0.85 seconds.
 *
 * See also @ref elm_obj_flipselector_first_interval_get.
 *
 * @param[in] interval The (first) interval value in seconds.
 *
 * @ingroup Elm_Flipselector
 */
EAPI void elm_flipselector_first_interval_set(Evas_Object *obj, double interval);

/**
 * @brief Get the interval on time updates for an user mouse button hold on a
 * flip selector widget.
 *
 * See also @ref elm_obj_flipselector_first_interval_set for more details.
 *
 * @return The (first) interval value in seconds.
 *
 * @ingroup Elm_Flipselector
 */
EAPI double elm_flipselector_first_interval_get(const Evas_Object *obj);

#include "elm_flipselector_item.eo.legacy.h"
#include "elm_flipselector.eo.legacy.h"
