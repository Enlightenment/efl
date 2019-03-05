/**
 * @ingroup Elm_Prefs
 *
 * @{
 */

/**
 * Add a page widget to an elm_prefs object.
 */
EAPI Eina_Bool elm_prefs_page_widget_common_add(Evas_Object *prefs, Evas_Object *obj);

/**
 *
 * Set file and page to populate a given prefs widget's interface.
 *
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * Elm prefs widgets start blank, with no child widgets. It's meant to
 * have its viewport populated with child elements coming from a
 * declaration file. That file (usually with @b .epb extension), is a
 * binary format (Eet) one, coming from a human-readable textual
 * declaration. This textual form (usually with @b .epc extension) is
 * translated to the binary form by means of the @b prefs_cc compiler.
 *
 * With this function, one thus populates a prefs widget with UI
 * elements.
 *
 * If @a file is @c NULL, "elm_app_data_dir_get()/preferences.epb"
 * will be used, by default. If @a file is a @b relative path, the
 * prefix "elm_app_data_dir_get()/" will be implicitly used with it.
 * If @a page is @c NULL, it is considered "main", as default.
 *
 * @warning If your binary is not properly installed and
 * elm_app_data_dir_get() can't be figured out, a fallback value of
 * "." will be tryed, instead.
 *
 * @see elm_prefs_file_get()
 *
 * @since 1.8
 *
 * @param[in] file The @b .epb (binary) file to get contents from
 * @param[in] page The page, inside @a file, where to get item contents from
 */
EAPI Eina_Bool elm_prefs_file_set(Eo *obj, const char *file, const char *page);

/**
 *
 * Get file and page bound to a given prefs widget.
 *
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * @note Use @c NULL pointers on the components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see elm_prefs_file_set() for more information
 *
 * @since 1.8
 *
 * @param[out] file The @b .epb (binary) file to get contents from
 * @param[out] page The page, inside @a file, where to get item contents from
 */
EAPI Eina_Bool elm_prefs_file_get(const Eo *obj, const char **file, const char **page);

#include "elm_prefs_eo.legacy.h"
/**
 * @}
 */

