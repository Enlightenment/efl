typedef Eo Elm_Multibuttonentry;

/**
 * @brief Add a new multibuttonentry to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 *
 * @ingroup Multibuttonentry
 */
EAPI Evas_Object               *elm_multibuttonentry_add(Evas_Object *parent);

/**
 * @brief Set a function to format the string that will be used to display the
 * hidden items counter.
 *
 * If @c format_function is @c NULL, the default format will be used, which is
 * $"... + %d".
 *
 * @param[in] obj The object.
 * @param[in] format_function Format_function The actual format function
 * @param[in] data Data User data to passed to @c format_function
 *
 * @since 1.9
 *
 * @ingroup Multibuttonentry
 */
 EAPI void elm_multibuttonentry_format_function_set(Eo *obj, Elm_Multibuttonentry_Format_Cb format_function, const void *data);

#include "elm_multibuttonentry_item.eo.legacy.h"
#include "elm_multibuttonentry.eo.legacy.h"
