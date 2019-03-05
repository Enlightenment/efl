#ifndef _ELM_WEB_EO_LEGACY_H_
#define _ELM_WEB_EO_LEGACY_H_

#ifndef _ELM_WEB_EO_CLASS_TYPE
#define _ELM_WEB_EO_CLASS_TYPE

typedef Eo Elm_Web;

#endif

#ifndef _ELM_WEB_EO_TYPES
#define _ELM_WEB_EO_TYPES


#endif

/**
 * @brief Sets whether to highlight the matched marks.
 *
 * If enabled, marks set with @ref elm_web_text_matches_mark will be
 * highlighted.
 *
 * @param[in] obj The object.
 * @param[in] highlight Whether to highlight the marks or not.
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_text_matches_highlight_set(Elm_Web *obj, Eina_Bool highlight);

/**
 * @brief Get whether highlighting marks is enabled.
 *
 * @param[in] obj The object.
 *
 * @return Whether to highlight the marks or not.
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_text_matches_highlight_get(const Elm_Web *obj);

/**
 * @brief Change useragent of a elm_web object.
 *
 * @param[in] obj The object.
 * @param[in] user_agent String for useragent.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_useragent_set(Elm_Web *obj, const char *user_agent);

/**
 * @brief Return current useragent of elm_web object.
 *
 * @param[in] obj The object.
 *
 * @return String for useragent.
 *
 * @ingroup Elm_Web_Group
 */
EAPI const char *elm_web_useragent_get(const Elm_Web *obj);

/**
 * @brief Sets the URL for the web object.
 *
 * It must be a full URL, with resource included, in the form
 * http://www.enlightenment.org or file:///tmp/something.html
 *
 * @param[in] obj The object.
 * @param[in] url The URL to set.
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_url_set(Elm_Web *obj, const char *url);

/**
 * @brief Get the current URL for the object.
 *
 * The returned string must not be freed and is guaranteed to be stringshared.
 *
 * @param[in] obj The object.
 *
 * @return The URL to set.
 *
 * @ingroup Elm_Web_Group
 */
EAPI const char *elm_web_url_get(const Elm_Web *obj);

/**
 * @brief Sets the background color to be used by the web object.
 *
 * This is the color that will be used by default when the loaded page does not
 * set it's own. Color values are pre-multiplied.
 *
 * @param[in] obj The object.
 * @param[in] r Red component.
 * @param[in] g Green component.
 * @param[in] b Blue component.
 * @param[in] a Alpha component.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_bg_color_set(Elm_Web *obj, int r, int g, int b, int a);

/**
 * @brief Get the background color to be used by the web object.
 *
 * This is the color that will be used by default when the loaded page does not
 * set it's own. Color values are pre-multiplied.
 *
 * @param[in] obj The object.
 * @param[out] r Red component.
 * @param[out] g Green component.
 * @param[out] b Blue component.
 * @param[out] a Alpha component.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_bg_color_get(const Elm_Web *obj, int *r, int *g, int *b, int *a);

/**
 * @brief Sets the default dialogs to use an Inwin instead of a normal window.
 *
 * If set, then the default implementation for the JavaScript dialogs and file
 * selector will be opened in an Inwin. Otherwise they will use a normal
 * separated window.
 *
 * @param[in] obj The object.
 * @param[in] value @c true if dialog will be opened as Inwin, @c false
 * otherwise
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_inwin_mode_set(Elm_Web *obj, Eina_Bool value);

/**
 * @brief Get whether Inwin mode is set for the current object.
 *
 * @param[in] obj The object.
 *
 * @return @c true if dialog will be opened as Inwin, @c false otherwise
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_inwin_mode_get(const Elm_Web *obj);

/**
 * @brief Sets whether to use tab propagation.
 *
 * If tab propagation is enabled, whenever the user presses the Tab key,
 * Elementary will handle it and switch focus to the next widget. The default
 * value is disabled, where WebKit will handle the Tab key to cycle focus
 * though its internal objects, jumping to the next widget only when that cycle
 * ends.
 *
 * @param[in] obj The object.
 * @param[in] propagate Whether to propagate Tab keys to Elementary or not.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_tab_propagate_set(Elm_Web *obj, Eina_Bool propagate);

/**
 * @brief Get the status of the tab propagation.
 *
 * @param[in] obj The object.
 *
 * @return Whether to propagate Tab keys to Elementary or not.
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_tab_propagate_get(const Elm_Web *obj);

/**
 * @brief Enables or disables the browsing history.
 *
 * @param[in] obj The object.
 * @param[in] enable Whether to enable or disable the browsing history.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_history_enabled_set(Elm_Web *obj, Eina_Bool enable);

/**
 * @brief Get whether browsing history is enabled for the given object.
 *
 * @param[in] obj The object.
 *
 * @return Whether to enable or disable the browsing history.
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_history_enabled_get(const Elm_Web *obj);

/**
 * @brief Sets the function to call when a console message is emitted from JS.
 *
 * This hook will be called when a console message is emitted from JavaScript.
 * There is no default implementation for this feature.
 *
 * @param[in] obj The object.
 * @param[in] func The callback function to be used.
 * @param[in] data User data.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_console_message_hook_set(Elm_Web *obj, Elm_Web_Console_Message func, void *data);

/**
 * @brief Sets the function to call when a new window is requested.
 *
 * This hook will be called when a request to create a new window is issued
 * from the web page loaded. There is no default implementation for this
 * feature, so leaving this unset or passing @c null in @c func will prevent
 * new windows from opening.
 *
 * @param[in] obj The object.
 * @param[in] func The hook function to be called when a window is requested.
 * @param[in] data User data.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_window_create_hook_set(Elm_Web *obj, Elm_Web_Window_Open func, void *data);

/**
 * @brief Sets the function to call when an file selector dialog.
 *
 * This hook will be called when a JavaScript file selector dialog is
 * requested. If no function is set or @c null is passed in @c func, the
 * default implementation will take place.
 *
 * @param[in] obj The object.
 * @param[in] func The callback function to be used.
 * @param[in] data User data.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_dialog_file_selector_hook_set(Elm_Web *obj, Elm_Web_Dialog_File_Selector func, void *data);

/**
 * @brief Sets the function to call when an confirm dialog.
 *
 * This hook will be called when a JavaScript confirm dialog is requested. If
 * no function is set or @c null is passed in @c func, the default
 * implementation will take place.
 *
 * @param[in] obj The object.
 * @param[in] func The callback function to be used.
 * @param[in] data User data.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_dialog_confirm_hook_set(Elm_Web *obj, Elm_Web_Dialog_Confirm func, void *data);

/**
 * @brief Tells the web object which index in the currently open popup was
 * selected.
 *
 * When the user handles the popup creation from the "popup,created" signal, it
 * needs to tell the web object which item was selected by calling this
 * function with the index corresponding to the item.
 *
 * @param[in] obj The object.
 * @param[in] idx The index selected.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_popup_selected_set(Elm_Web *obj, int idx);

/**
 * @brief Sets the function to call when an prompt dialog.
 *
 * This hook will be called when a JavaScript prompt dialog is requested. If no
 * function is set or @c null is passed in @c func, the default implementation
 * will take place.
 *
 * @param[in] obj The object.
 * @param[in] func The callback function to be used.
 * @param[in] data User data.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_dialog_prompt_hook_set(Elm_Web *obj, Elm_Web_Dialog_Prompt func, void *data);

/**
 * @brief Sets the function to call when an alert dialog.
 *
 * This hook will be called when a JavaScript alert dialog is requested. If no
 * function is set or @c null is passed in @c func, the default implementation
 * will take place.
 *
 * @param[in] obj The object.
 * @param[in] func Callback function to be used.
 * @param[in] data User data.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_dialog_alert_hook_set(Elm_Web *obj, Elm_Web_Dialog_Alert func, void *data);

/**
 * @brief Queries whether it's possible to go forward in history.
 *
 * @param[in] obj The object.
 *
 * @return @c true if it is possible to go forward, @c false otherwise
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_forward_possible_get(const Elm_Web *obj);

/**
 * @brief Get internal ewk_view object from web object.
 *
 * Elementary may not provide some low level features of EWebKit, instead of
 * cluttering the API with proxy methods we opted to return the internal
 * reference. Be careful using it as it may interfere with elm_web behavior.
 *
 * @param[in] obj The object.
 *
 * @return The internal ewk_view object or @c null if it does not exist.
 * (Failure to create or Elementary compiled without ewebkit)
 *
 * @ingroup Elm_Web_Group
 */
EAPI Efl_Canvas_Object *elm_web_webkit_view_get(const Elm_Web *obj);

/**
 * @brief Queries whether it's possible to go back in history.
 *
 * @param[in] obj The object.
 *
 * @return @c true if it is possible to go back, @c false otherwise
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_back_possible_get(const Elm_Web *obj);

/**
 * @brief Get the overall loading progress of the page.
 *
 * Returns the estimated loading progress of the page, with a value between 0.0
 * and 1.0. This is an estimated progress accounting for all the frames
 * included in the page.
 *
 * @param[in] obj The object.
 *
 * @return A value between 0.0 and 1.0 indicating the progress, or -1.0 on
 * failure.
 *
 * @ingroup Elm_Web_Group
 */
EAPI double elm_web_load_progress_get(const Elm_Web *obj);

/**
 * @brief Get a copy of the currently selected text.
 *
 * The string returned must be freed by the user when it's done with it.
 *
 * @param[in] obj The object.
 *
 * @return A newly allocated string, or @c null if nothing is selected or an
 * error occurred.
 *
 * @ingroup Elm_Web_Group
 */
EAPI char *elm_web_selection_get(const Elm_Web *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the current title.
 *
 * The returned string must not be freed and is guaranteed to be stringshared.
 *
 * @param[in] obj The object.
 *
 * @return A stringshared internal string with the current title, or @c null on
 * failure.
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Stringshare *elm_web_title_get(const Elm_Web *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Jumps the given number of steps in the browsing history.
 *
 * The @c steps value can be a negative integer to back in history, or a
 * positive to move forward.
 *
 * @param[in] obj The object.
 * @param[in] steps The number of steps to jump.
 *
 * @return @c true on success, @c false on error or if not enough history
 * exists to jump the given number of steps
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_navigate(Elm_Web *obj, int steps);

/**
 * @brief Goes back one step in the browsing history.
 *
 * This is equivalent to calling elm_web_object_navigate(obj, -1);
 *
 * @param[in] obj The object.
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_back(Elm_Web *obj);

/**
 * @brief Loads the specified @c html string as the content of the web object.
 *
 * External objects such as stylesheets or images referenced in the HTML
 * document are located relative to @c base_url.
 *
 * If an @c unreachable_url is passed it is used as the url for the loaded
 * content. This is typically used to display error pages for a failed load.
 *
 * @param[in] obj The object.
 * @param[in] html HTML data to load.
 * @param[in] base_url Base URL used for relative paths to external objects
 * (optional).
 * @param[in] unreachable_url URL that could not be reached (optional).
 *
 * @return @c true on success, @c false otherwise
 *
 * @since 1.9
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_html_string_load(Elm_Web *obj, const char *html, const char *base_url, const char *unreachable_url);

/**
 * @brief Searches the given string in a document.
 *
 * @param[in] obj The object.
 * @param[in] string String to search.
 * @param[in] case_sensitive If search should be case sensitive or not.
 * @param[in] forward If search is from cursor and on or backwards.
 * @param[in] wrap If search should wrap at the end.
 *
 * @return @c true if the given string was found, @c false if not or failure.
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_text_search(const Elm_Web *obj, const char *string, Eina_Bool case_sensitive, Eina_Bool forward, Eina_Bool wrap);

/**
 * @brief Dismisses an open dropdown popup
 *
 * When the popup from a dropdown widget is to be dismissed, either after
 * selecting an option or to cancel it, this function must be called, which
 * will later emit an "popup,willdelete" signal to notify the user that any
 * memory and objects related to this popup can be freed.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the menu was successfully destroyed, or @c false if there
 * was no menu to destroy.
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_popup_destroy(Elm_Web *obj);

/**
 * @brief Shows the given region in the web object.
 *
 * @param[in] obj The object.
 * @param[in] x The x coordinate of the region to show.
 * @param[in] y The y coordinate of the region to show.
 * @param[in] w The width of the region to show.
 * @param[in] h The height of the region to show.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_region_show(Elm_Web *obj, int x, int y, int w, int h);

/**
 * @brief Goes forward one step in the browsing history.
 *
 * This is equivalent to calling elm_web_object_navigate(obj, 1);
 *
 * See also @ref elm_web_history_enabled_set,
 * @ref elm_web_forward_possible_get, @ref elm_web_back, @ref elm_web_navigate.
 *
 * @param[in] obj The object.
 *
 * @return @c true on success, @c false otherwise.
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_forward(Elm_Web *obj);

/**
 * @brief Marks matches of the given string in a document.
 *
 * @param[in] obj The object.
 * @param[in] string String to match.
 * @param[in] case_sensitive If match should be case sensitive or not.
 * @param[in] highlight If matches should be highlighted.
 * @param[in] limit Maximum amount of matches, or zero to unlimited.
 *
 * @return Number of matched @c string.
 *
 * @ingroup Elm_Web_Group
 */
EAPI unsigned int elm_web_text_matches_mark(Elm_Web *obj, const char *string, Eina_Bool case_sensitive, Eina_Bool highlight, unsigned int limit);

/**
 * @brief Brings in the region to the visible area.
 *
 * Like @ref elm_web_region_show, but it animates the scrolling of the object
 * to show the area.
 *
 * @param[in] obj The object.
 * @param[in] x The x coordinate of the region to show.
 * @param[in] y The y coordinate of the region to show.
 * @param[in] w The width of the region to show.
 * @param[in] h The height of the region to show.
 *
 * @ingroup Elm_Web_Group
 */
EAPI void elm_web_region_bring_in(Elm_Web *obj, int x, int y, int w, int h);

/**
 * @brief Stops loading the current page.
 *
 * Cancels the loading of the current page in the web object. This will cause a
 * "load,error" signal to be emitted, with the is_cancellation flag set to
 * @c true.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the cancel was successful, @c false otherwise.
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_stop(Elm_Web *obj);

/**
 * @brief Queries whether it's possible to jump the given number of steps.
 *
 * The @c steps value can be a negative integer to back in history, or positive
 * to move forward.
 *
 * @param[in] obj The object.
 * @param[in] steps The number of steps to check for.
 *
 * @return @c true if enough history exists to perform the given jump, @c false
 * otherwise.
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_navigate_possible_get(Elm_Web *obj, int steps);

/**
 * @brief Requests a reload of the current document, avoiding any existing
 * caches.
 *
 * @param[in] obj The object.
 *
 * @return @c true on success, @c false otherwise.
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_reload_full(Elm_Web *obj);

/**
 * @brief Clears all marked matches in the document.
 *
 * @param[in] obj The object.
 *
 * @return @c true on success, @c false otherwise.
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_text_matches_unmark_all(Elm_Web *obj);

/**
 * @brief Requests a reload of the current document in the object.
 *
 * @param[in] obj The object.
 *
 * @return @c true on success, @c false otherwise.
 *
 * @ingroup Elm_Web_Group
 */
EAPI Eina_Bool elm_web_reload(Elm_Web *obj);

#endif
