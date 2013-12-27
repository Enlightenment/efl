/**
 * Get internal ewk_view object from web object.
 *
 * Elementary may not provide some low level features of EWebKit,
 * instead of cluttering the API with proxy methods we opted to
 * return the internal reference. Be careful using it as it may
 * interfere with elm_web behavior.
 *
 * @param obj The web object.
 * @return The internal ewk_view object or @c NULL if it does not
 *         exist. (Failure to create or Elementary compiled without
 *         ewebkit)
 *
 * @see elm_web_add()
 *
 * @ingroup Web
 */
EAPI Evas_Object      *elm_web_webkit_view_get(const Evas_Object *obj);

/**
 * Sets the function to call when a new window is requested
 *
 * This hook will be called when a request to create a new window is
 * issued from the web page loaded.
 * There is no default implementation for this feature, so leaving this
 * unset or passing @c NULL in @p func will prevent new windows from
 * opening.
 *
 * @param obj The web object where to set the hook function
 * @param func The hook function to be called when a window is requested
 * @param data User data
 *
 * @ingroup Web
 */
EAPI void              elm_web_window_create_hook_set(Evas_Object *obj, Elm_Web_Window_Open func, void *data);

/**
 * Sets the function to call when an alert dialog
 *
 * This hook will be called when a JavaScript alert dialog is requested.
 * If no function is set or @c NULL is passed in @p func, the default
 * implementation will take place.
 *
 * @param obj The web object where to set the hook function
 * @param func The callback function to be used
 * @param data User data
 *
 * @see elm_web_inwin_mode_set()
 *
 * @ingroup Web
 */
EAPI void              elm_web_dialog_alert_hook_set(Evas_Object *obj, Elm_Web_Dialog_Alert func, void *data);

/**
 * Sets the function to call when an confirm dialog
 *
 * This hook will be called when a JavaScript confirm dialog is requested.
 * If no function is set or @c NULL is passed in @p func, the default
 * implementation will take place.
 *
 * @param obj The web object where to set the hook function
 * @param func The callback function to be used
 * @param data User data
 *
 * @see elm_web_inwin_mode_set()
 *
 * @ingroup Web
 */
EAPI void              elm_web_dialog_confirm_hook_set(Evas_Object *obj, Elm_Web_Dialog_Confirm func, void *data);

/**
 * Sets the function to call when an prompt dialog
 *
 * This hook will be called when a JavaScript prompt dialog is requested.
 * If no function is set or @c NULL is passed in @p func, the default
 * implementation will take place.
 *
 * @param obj The web object where to set the hook function
 * @param func The callback function to be used
 * @param data User data
 *
 * @see elm_web_inwin_mode_set()
 *
 * @ingroup Web
 */
EAPI void              elm_web_dialog_prompt_hook_set(Evas_Object *obj, Elm_Web_Dialog_Prompt func, void *data);

/**
 * Sets the function to call when an file selector dialog
 *
 * This hook will be called when a JavaScript file selector dialog is
 * requested.
 * If no function is set or @c NULL is passed in @p func, the default
 * implementation will take place.
 *
 * @param obj The web object where to set the hook function
 * @param func The callback function to be used
 * @param data User data
 *
 * @see elm_web_inwin_mode_set()
 *
 * @ingroup Web
 */
EAPI void              elm_web_dialog_file_selector_hook_set(Evas_Object *obj, Elm_Web_Dialog_File_Selector func, void *data);

/**
 * Sets the function to call when a console message is emitted from JS
 *
 * This hook will be called when a console message is emitted from
 * JavaScript. There is no default implementation for this feature.
 *
 * @param obj The web object where to set the hook function
 * @param func The callback function to be used
 * @param data User data
 *
 * @ingroup Web
 */
EAPI void              elm_web_console_message_hook_set(Evas_Object *obj, Elm_Web_Console_Message func, void *data);

/**
 * Change useragent of a elm_web object
 *
 * @param obj The object
 * @param user_agent String for useragent
 *
 * @ingroup Web
 */
EAPI void elm_web_useragent_set(Evas_Object *obj, const char *user_agent);

/**
 * Return current useragent of elm_web object
 *
 * @param obj The object
 * @return Useragent string
 *
 * @ingroup Web
 */
EAPI const char* elm_web_useragent_get(const Evas_Object *obj);

/**
 * Get the status of the tab propagation
 *
 * @param obj The web object to query
 * @return @c EINA_TRUE if tab propagation is enabled, @c EINA_FALSE otherwise
 *
 * @see elm_web_tab_propagate_set()
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_tab_propagate_get(const Evas_Object *obj);

/**
 * Sets whether to use tab propagation
 *
 * If tab propagation is enabled, whenever the user presses the Tab key,
 * Elementary will handle it and switch focus to the next widget.
 * The default value is disabled, where WebKit will handle the Tab key to
 * cycle focus though its internal objects, jumping to the next widget
 * only when that cycle ends.
 *
 * @param obj The web object
 * @param propagate Whether to propagate Tab keys to Elementary or not
 *
 * @ingroup Web
 */
EAPI void              elm_web_tab_propagate_set(Evas_Object *obj, Eina_Bool propagate);

/**
 * Sets the URL for the web object
 *
 * It must be a full URL, with resource included, in the form
 * http://www.enlightenment.org or file:///tmp/something.html
 *
 * @param obj The web object
 * @param url The URL to set
 * @return @c EINA_TRUE if the URL could be set, @c EINA_FALSE if an error occurred.
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_url_set(Evas_Object *obj, const char *url);

/**
 * Loads the specified @a html string as the content of the web object
 *
 * @param obj The web object
 * @param html HTML data to load
 * @param base_url Base URL used for relative paths to external objects (optional)
 * @param unreachable_url URL that could not be reached (optional)
 *
 * @return @c EINA_TRUE if it the HTML was successfully loaded, @c EINA_FALSE otherwise
 *
 * External objects such as stylesheets or images referenced in the HTML
 * document are located relative to @a base_url.
 *
 * If an @a unreachable_url is passed it is used as the url for the loaded
 * content. This is typically used to display error pages for a failed
 * load.
 *
 * @since 1.9
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_html_string_load(Evas_Object *obj, const char *html, const char *base_url, const char *unreachable_url);

/**
 * Get the current URL for the object
 *
 * The returned string must not be freed and is guaranteed to be
 * stringshared.
 *
 * @param obj The web object
 * @return A stringshared internal string with the current URL, or @c NULL on
 * failure
 *
 * @ingroup Web
 */
EAPI const char       *elm_web_url_get(const Evas_Object *obj);

/**
 * Sets the URI for the web object
 *
 * It must be a full URI, with resource included, in the form
 * http://www.enlightenment.org or file:///tmp/something.html
 *
 * @param obj The web object
 * @param uri The URI to set
 * @return @c EINA_TRUE if the URI could be set, @c EINA_FALSE if an error occurred.
 *
 * @deprecated Use elm_web_url_set() instead
 *
 * @see elm_web_url_set()
 *
 * @ingroup Web
 */
EINA_DEPRECATED EAPI Eina_Bool         elm_web_uri_set(Evas_Object *obj, const char *uri);

/**
 * Get the current URI for the object
 *
 * The returned string must not be freed and is guaranteed to be
 * stringshared.
 *
 * @param obj The web object
 * @return A stringshared internal string with the current URI, or @c NULL on
 * failure
 *
 * @deprecated Use elm_web_url_get() instead
 *
 * @see elm_web_url_get()
 *
 * @ingroup Web
 */
EINA_DEPRECATED EAPI const char       *elm_web_uri_get(const Evas_Object *obj);

/**
 * Get the current title
 *
 * The returned string must not be freed and is guaranteed to be
 * stringshared.
 *
 * @param obj The web object
 * @return A stringshared internal string with the current title, or @c NULL on
 * failure
 *
 * @ingroup Web
 */
EAPI const char       *elm_web_title_get(const Evas_Object *obj);

/**
 * Sets the background color to be used by the web object
 *
 * This is the color that will be used by default when the loaded page
 * does not set it's own. Color values are pre-multiplied.
 *
 * @param obj The web object
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 * @param a Alpha component
 *
 * @ingroup Web
 */
EAPI void              elm_web_bg_color_set(Evas_Object *obj, int r, int g, int b, int a);

/**
 * Get the background color to be used by the web object
 *
 * This is the color that will be used by default when the loaded page
 * does not set it's own. Color values are pre-multiplied.
 *
 * @param obj The web object
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 * @param a Alpha component
 *
 * @ingroup Web
 */
EAPI void              elm_web_bg_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a);

/**
 * Get a copy of the currently selected text
 *
 * The string returned must be freed by the user when it's done with it.
 *
 * @param obj The web object
 * @return A newly allocated string, or @c NULL if nothing is selected or an
 * error occurred
 *
 * @ingroup Web
 */
EAPI const char       *elm_web_selection_get(const Evas_Object *obj);

/**
 * Tells the web object which index in the currently open popup was selected
 *
 * When the user handles the popup creation from the "popup,created" signal,
 * it needs to tell the web object which item was selected by calling this
 * function with the index corresponding to the item.
 *
 * @param obj The web object
 * @param index The index selected
 *
 * @see elm_web_popup_destroy()
 *
 * @ingroup Web
 */
EAPI void              elm_web_popup_selected_set(Evas_Object *obj, int index);

/**
 * Dismisses an open dropdown popup
 *
 * When the popup from a dropdown widget is to be dismissed, either after
 * selecting an option or to cancel it, this function must be called, which
 * will later emit an "popup,willdelete" signal to notify the user that
 * any memory and objects related to this popup can be freed.
 *
 * @param obj The web object
 * @return @c EINA_TRUE if the menu was successfully destroyed, or @c EINA_FALSE
 * if there was no menu to destroy
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_popup_destroy(Evas_Object *obj);

/**
 * Searches the given string in a document.
 *
 * @param obj The web object where to search the text
 * @param string String to search
 * @param case_sensitive If search should be case sensitive or not
 * @param forward If search is from cursor and on or backwards
 * @param wrap If search should wrap at the end
 *
 * @return @c EINA_TRUE if the given string was found, @c EINA_FALSE if not
 * or failure
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_text_search(const Evas_Object *obj, const char *string, Eina_Bool case_sensitive, Eina_Bool forward, Eina_Bool wrap);

/**
 * Marks matches of the given string in a document.
 *
 * @param obj The web object where to search text
 * @param string String to match
 * @param case_sensitive If match should be case sensitive or not
 * @param highlight If matches should be highlighted
 * @param limit Maximum amount of matches, or zero to unlimited
 *
 * @return number of matched @a string
 *
 * @ingroup Web
 */
EAPI unsigned int      elm_web_text_matches_mark(Evas_Object *obj, const char *string, Eina_Bool case_sensitive, Eina_Bool highlight, unsigned int limit);

/**
 * Clears all marked matches in the document
 *
 * @param obj The web object
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_text_matches_unmark_all(Evas_Object *obj);

/**
 * Sets whether to highlight the matched marks
 *
 * If enabled, marks set with elm_web_text_matches_mark() will be
 * highlighted.
 *
 * @param obj The web object
 * @param highlight Whether to highlight the marks or not
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_text_matches_highlight_set(Evas_Object *obj, Eina_Bool highlight);

/**
 * Get whether highlighting marks is enabled
 *
 * @param obj The web object
 *
 * @return @c EINA_TRUE is marks are set to be highlighted, @c EINA_FALSE
 * otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_text_matches_highlight_get(const Evas_Object *obj);

/**
 * Get the overall loading progress of the page
 *
 * Returns the estimated loading progress of the page, with a value between
 * 0.0 and 1.0. This is an estimated progress accounting for all the frames
 * included in the page.
 *
 * @param obj The web object
 *
 * @return A value between 0.0 and 1.0 indicating the progress, or -1.0 on
 * failure
 *
 * @ingroup Web
 */
EAPI double            elm_web_load_progress_get(const Evas_Object *obj);

/**
 * Stops loading the current page
 *
 * Cancels the loading of the current page in the web object. This will
 * cause a "load,error" signal to be emitted, with the is_cancellation
 * flag set to @c EINA_TRUE.
 *
 * @param obj The web object
 *
 * @return @c EINA_TRUE if the cancel was successful, @c EINA_FALSE otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_stop(Evas_Object *obj);

/**
 * Requests a reload of the current document in the object
 *
 * @param obj The web object
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_reload(Evas_Object *obj);

/**
 * Requests a reload of the current document, avoiding any existing caches
 *
 * @param obj The web object
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_reload_full(Evas_Object *obj);

/**
 * Goes back one step in the browsing history
 *
 * This is equivalent to calling elm_web_object_navigate(obj, -1);
 *
 * @param obj The web object
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @see elm_web_history_enabled_set()
 * @see elm_web_back_possible()
 * @see elm_web_forward()
 * @see elm_web_navigate()
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_back(Evas_Object *obj);

/**
 * Goes forward one step in the browsing history
 *
 * This is equivalent to calling elm_web_object_navigate(obj, 1);
 *
 * @param obj The web object
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @see elm_web_history_enabled_set()
 * @see elm_web_forward_possible_get()
 * @see elm_web_back()
 * @see elm_web_navigate()
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_forward(Evas_Object *obj);

/**
 * Jumps the given number of steps in the browsing history
 *
 * The @p steps value can be a negative integer to back in history, or a
 * positive to move forward.
 *
 * @param obj The web object
 * @param steps The number of steps to jump
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error or if not enough
 * history exists to jump the given number of steps
 *
 * @see elm_web_history_enabled_set()
 * @see elm_web_back()
 * @see elm_web_forward()
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_navigate(Evas_Object *obj, int steps);

/**
 * Queries whether it's possible to go back in history
 *
 * @param obj The web object
 *
 * @return @c EINA_TRUE if it's possible to back in history, @c EINA_FALSE
 * otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_back_possible_get(Evas_Object *obj);

/**
 * Queries whether it's possible to go forward in history
 *
 * @param obj The web object
 *
 * @return @c EINA_TRUE if it's possible to forward in history, @c EINA_FALSE
 * otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_forward_possible_get(Evas_Object *obj);

/**
 * Queries whether it's possible to jump the given number of steps
 *
 * The @p steps value can be a negative integer to back in history, or a
 * positive to move forward.
 *
 * @param obj The web object
 * @param steps The number of steps to check for
 *
 * @return @c EINA_TRUE if enough history exists to perform the given jump,
 * @c EINA_FALSE otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_navigate_possible_get(Evas_Object *obj, int steps);

/**
 * Get whether browsing history is enabled for the given object
 *
 * @param obj The web object
 *
 * @return @c EINA_TRUE if history is enabled, @c EINA_FALSE otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_history_enabled_get(const Evas_Object *obj);

/**
 * Enables or disables the browsing history
 *
 * @param obj The web object
 * @param enabled Whether to enable or disable the browsing history
 *
 * @ingroup Web
 */
EAPI void              elm_web_history_enabled_set(Evas_Object *obj, Eina_Bool enabled);

/**
 * Sets the zoom level of the web object
 *
 * Zoom level matches the Webkit API, so 1.0 means normal zoom, with higher
 * values meaning zoom in and lower meaning zoom out. This function will
 * only affect the zoom level if the mode set with elm_web_zoom_mode_set()
 * is ::ELM_WEB_ZOOM_MODE_MANUAL.
 *
 * @param obj The web object
 * @param zoom The zoom level to set
 *
 * @ingroup Web
 */
EAPI void              elm_web_zoom_set(Evas_Object *obj, double zoom);

/**
 * Get the current zoom level set on the web object
 *
 * Note that this is the zoom level set on the web object and not that
 * of the underlying Webkit one. In the ::ELM_WEB_ZOOM_MODE_MANUAL mode,
 * the two zoom levels should match, but for the other two modes the
 * Webkit zoom is calculated internally to match the chosen mode without
 * changing the zoom level set for the web object.
 *
 * @param obj The web object
 *
 * @return The zoom level set on the object
 *
 * @ingroup Web
 */
EAPI double            elm_web_zoom_get(const Evas_Object *obj);

/**
 * Shows the given region in the web object
 *
 * @param obj The web object
 * @param x The x coordinate of the region to show
 * @param y The y coordinate of the region to show
 * @param w The width of the region to show
 * @param h The height of the region to show
 *
 * @ingroup Web
 */
EAPI void              elm_web_region_show(Evas_Object *obj, int x, int y, int w, int h);

/**
 * Brings in the region to the visible area
 *
 * Like elm_web_region_show(), but it animates the scrolling of the object
 * to show the area
 *
 * @param obj The web object
 * @param x The x coordinate of the region to show
 * @param y The y coordinate of the region to show
 * @param w The width of the region to show
 * @param h The height of the region to show
 *
 * @ingroup Web
 */
EAPI void              elm_web_region_bring_in(Evas_Object *obj, int x, int y, int w, int h);

/**
 * Sets the default dialogs to use an Inwin instead of a normal window
 *
 * If set, then the default implementation for the JavaScript dialogs and
 * file selector will be opened in an Inwin. Otherwise they will use a
 * normal separated window.
 *
 * @param obj The web object
 * @param value @c EINA_TRUE to use Inwin, @c EINA_FALSE to use a normal window
 *
 * @ingroup Web
 */
EAPI void              elm_web_inwin_mode_set(Evas_Object *obj, Eina_Bool value);

/**
 * Get whether Inwin mode is set for the current object
 *
 * @param obj The web object
 *
 * @return @c EINA_TRUE if Inwin mode is set, @c EINA_FALSE otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_inwin_mode_get(const Evas_Object *obj);
