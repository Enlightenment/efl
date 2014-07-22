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

/**
 *
 * Set the file (image or edje collection) to give life for the
 * background
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * This sets the image file used in the background object. If the
 * image comes from an Edje group, it will be stretched to completely
 * fill the background object. If it comes from a traditional image file, it
 * will by default be centered in this widget's are (thus retaining
 * its aspect), what could lead to some parts being not visible. You
 * may change the mode of exhibition for a real image file with
 * elm_bg_option_set().
 *
 * @note Once the image of @p obj is set, a previously set one will be
 * deleted, even if @p file is @c NULL.
 *
 * @note This will only affect the contents of one of the background's
 * swallow spots, namely @c "elm.swallow.background". If you want to
 * achieve the @c Layout's file setting behavior, you'll have to call
 * that method on this object.
 *
 * @ingroup Bg
 *
 * @param[in] file The file path
 * @param[in] group Optional key (group in Edje) within the file
 */
EAPI Eina_Bool elm_bg_file_set(Eo *obj, const char *file, const char *group);

/**
 *
 * Get the file (image or edje collection) set on a given background
 * widget
 *
 * @note Use @c NULL pointers on the file components you're not
 * interested in: they'll be ignored by the function.
 *
 * @ingroup Bg
 *
 * @param[out] file The file path
 * @param[out] group Optional key (group in Edje) within the file
 */
EAPI void elm_bg_file_get(const Eo *obj, const char **file, const char **group);

#include "elm_bg.eo.legacy.h"
