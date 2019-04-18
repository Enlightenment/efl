/**
 * Add a new file selector button widget to the given parent
 * Elementary (container) object
 *
 * @param parent The parent object
 * @return a new file selector button widget handle or @c NULL, on
 * errors
 *
 * @ingroup Elm_File_Selector_Button
 */
EAPI Evas_Object                *elm_fileselector_button_add(Evas_Object *parent);

#include "elm_fileselector_button_eo.legacy.h"

/**
 * @brief Set whether a given file selector button widget's internal file
 * selector will raise an Elementary "inner window", instead of a dedicated
 * Elementary window. By default, it depends on the current profile.
 *
 * See @ref elm_win_inwin_add for more information on inner windows. See also
 * @ref elm_fileselector_button_inwin_mode_get.
 *
 * @param[in] value true to make it use an inner window, false to make it use a
 * dedicated window
 *
 * @ingroup Elm_Fileselector_Button
 */
EAPI void elm_fileselector_button_inwin_mode_set(Elm_Fileselector_Button *obj, Eina_Bool value);

/**
 * @brief Get whether a given file selector button widget's internal file
 * selector will raise an Elementary "inner window", instead of a dedicated
 * Elementary window.
 *
 * See also @ref elm_fileselector_button_inwin_mode_set for more details.
 *
 * @return true to make it use an inner window, false to make it use a
 * dedicated window
 *
 * @ingroup Elm_Fileselector_Button
 */
EAPI Eina_Bool elm_fileselector_button_inwin_mode_get(const Elm_Fileselector_Button *obj);

/**
 * @brief Set the size of a given file selector button widget's window, holding
 * the file selector itself.
 *
 * @note it will only take any effect if the file selector button widget is not
 * under "inwin mode". The default size for the window (when applicable) is
 * 400x400 pixels.
 *
 * See also @ref elm_fileselector_button_window_size_get.
 *
 * @param[in] width The window's width
 * @param[in] height The window's height
 *
 * @ingroup Elm_Fileselector_Button
 */
EAPI void elm_fileselector_button_window_size_set(Elm_Fileselector_Button *obj, Evas_Coord width, Evas_Coord height);

/**
 * @brief Get the size of a given file selector button widget's window, holding
 * the file selector itself.
 *
 * @note Use null pointers on the size values you're not interested in: they'll
 * be ignored by the function.
 *
 * See also @ref elm_fileselector_button_window_size_set for more details.
 *
 * @param[out] width The window's width
 * @param[out] height The window's height
 *
 * @ingroup Elm_Fileselector_Button
 */
EAPI void elm_fileselector_button_window_size_get(const Elm_Fileselector_Button *obj, Evas_Coord *width, Evas_Coord *height);

/**
 * @brief Set the title for a given file selector button widget's window
 *
 * This will change the popup window's title, when the file selector pops out
 * after a click on the button. Those windows have the default (unlocalized)
 * value of "Select a file" as titles.
 *
 * @note It will only take effect if the file selector button widget is not
 * under "inwin mode".
 *
 * See also @ref elm_fileselector_button_window_title_get.
 *
 * @param[in] title The title string.
 *
 * @ingroup Elm_Fileselector_Button
 */
EAPI void elm_fileselector_button_window_title_set(Elm_Fileselector_Button *obj, const char *title);

/**
 * @brief Get the title for a given file selector button widget's window
 *
 * See also @ref elm_fileselector_button_window_title_set for more details.
 *
 * @return The title string.
 *
 * @ingroup Elm_Fileselector_Button
 */
EAPI const char *elm_fileselector_button_window_title_get(const Elm_Fileselector_Button *obj);
