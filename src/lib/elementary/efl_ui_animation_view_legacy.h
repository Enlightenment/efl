typedef Eo Elm_Animation_View;

/**
 * Add a new animation view widget to the parent's canvas
 *
 * @param parent The parent object
 * @return The new animation view object or @c NULL if it failed to create.
 *
 * @ingroup Elm_Animation_View
 *
 * @since 1.22
 */
EAPI Elm_Animation_View     *elm_animation_view_add(Evas_Object *parent);

/**
 *
 * Set the source file from where an vector object must fetch the real
 * vector data (it may be one of json, svg, eet files).
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can specify the key to be used as the index of the vector in
 * this file.
 *
 * @param[in] file The vector file path.
 * @param[in] key The vector key in @p file (if its an Eet one), or @c
NULL, otherwise.
 *
 * @return @c EINA_TRUE if it's succeed to read file, @c EINA_FALSE otherwise.
 *
 * @ingroup Elm_Animation_View
 *
 * @since 1.22
*/
EAPI Eina_Bool        elm_animation_view_file_set(Elm_Animation_View *obj, const char *file, const char *key);

#include "efl_ui_animation_view.eo.legacy.h"
