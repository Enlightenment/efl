/**
 * Add a new background to the parent
 *
 * @param parent The parent object
 * @return The new object or @c NULL if it cannot be created
 *
 * @ingroup Bg
 */
EAPI Evas_Object                 *elm_bg_add(Evas_Object *parent);

/**
 * Set the color on a given background widget
 *
 * @param obj The background object handle
 * @param r The red color component's value
 * @param g The green color component's value
 * @param b The blue color component's value
 *
 * This sets the color used for the background rectangle, in RGB
 * format. Each color component's range is from 0 to 255.
 *
 * @note You probably only want to use this function if you haven't
 * previously called elm_bg_file_set(), so that you just want a solid
 * color background.
 *
 * @note You can reset the color by setting @p r, @p g, @p b as -1, -1, -1.
 *
 * @see elm_bg_color_get()
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_color_set(Evas_Object *obj, int r, int g, int b);

/**
 * Get the color set on a given background widget
 *
 * @param obj The background object handle
 * @param r Where to store the red color component's value
 * @param g Where to store the green color component's value
 * @param b Where to store the blue color component's value
 *
 * @note Use @c NULL pointers on the file components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see elm_bg_color_get() for more details
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_color_get(const Evas_Object *obj, int *r, int *g, int *b);

#include "elm_bg.eo.legacy.h"