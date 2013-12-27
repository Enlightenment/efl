#define ELM_OBJ_WEB_CLASS elm_obj_web_class_get()

const Eo_Class *elm_obj_web_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_WEB_BASE_ID;

enum
{
   ELM_OBJ_WEB_SUB_ID_WEBKIT_VIEW_GET,
   ELM_OBJ_WEB_SUB_ID_WINDOW_CREATE_HOOK_SET,
   ELM_OBJ_WEB_SUB_ID_DIALOG_ALERT_HOOK_SET,
   ELM_OBJ_WEB_SUB_ID_DIALOG_CONFIRM_HOOK_SET,
   ELM_OBJ_WEB_SUB_ID_DIALOG_PROMPT_HOOK_SET,
   ELM_OBJ_WEB_SUB_ID_DIALOG_FILE_SELECTOR_HOOK_SET,
   ELM_OBJ_WEB_SUB_ID_CONSOLE_MESSAGE_HOOK_SET,
   ELM_OBJ_WEB_SUB_ID_USERAGENT_SET,
   ELM_OBJ_WEB_SUB_ID_USERAGENT_GET,
   ELM_OBJ_WEB_SUB_ID_TAB_PROPAGATE_GET,
   ELM_OBJ_WEB_SUB_ID_TAB_PROPAGATE_SET,
   ELM_OBJ_WEB_SUB_ID_URL_SET,
   ELM_OBJ_WEB_SUB_ID_URL_GET,
   ELM_OBJ_WEB_SUB_ID_HTML_STRING_LOAD,
   ELM_OBJ_WEB_SUB_ID_TITLE_GET,
   ELM_OBJ_WEB_SUB_ID_BG_COLOR_SET,
   ELM_OBJ_WEB_SUB_ID_BG_COLOR_GET,
   ELM_OBJ_WEB_SUB_ID_SELECTION_GET,
   ELM_OBJ_WEB_SUB_ID_POPUP_SELECTED_SET,
   ELM_OBJ_WEB_SUB_ID_POPUP_DESTROY,
   ELM_OBJ_WEB_SUB_ID_TEXT_SEARCH,
   ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_MARK,
   ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_UNMARK_ALL,
   ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_HIGHLIGHT_SET,
   ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_HIGHLIGHT_GET,
   ELM_OBJ_WEB_SUB_ID_LOAD_PROGRESS_GET,
   ELM_OBJ_WEB_SUB_ID_STOP,
   ELM_OBJ_WEB_SUB_ID_RELOAD,
   ELM_OBJ_WEB_SUB_ID_RELOAD_FULL,
   ELM_OBJ_WEB_SUB_ID_BACK,
   ELM_OBJ_WEB_SUB_ID_FORWARD,
   ELM_OBJ_WEB_SUB_ID_NAVIGATE,
   ELM_OBJ_WEB_SUB_ID_BACK_POSSIBLE_GET,
   ELM_OBJ_WEB_SUB_ID_FORWARD_POSSIBLE_GET,
   ELM_OBJ_WEB_SUB_ID_NAVIGATE_POSSIBLE_GET,
   ELM_OBJ_WEB_SUB_ID_HISTORY_ENABLED_GET,
   ELM_OBJ_WEB_SUB_ID_HISTORY_ENABLED_SET,
   ELM_OBJ_WEB_SUB_ID_ZOOM_SET,
   ELM_OBJ_WEB_SUB_ID_ZOOM_GET,
   ELM_OBJ_WEB_SUB_ID_ZOOM_MODE_SET,
   ELM_OBJ_WEB_SUB_ID_ZOOM_MODE_GET,
   ELM_OBJ_WEB_SUB_ID_REGION_SHOW,
   ELM_OBJ_WEB_SUB_ID_REGION_BRING_IN,
   ELM_OBJ_WEB_SUB_ID_INWIN_MODE_SET,
   ELM_OBJ_WEB_SUB_ID_INWIN_MODE_GET,
   ELM_OBJ_WEB_SUB_ID_LAST
};

#define ELM_OBJ_WEB_ID(sub_id) (ELM_OBJ_WEB_BASE_ID + sub_id)


/**
 * @def elm_obj_web_webkit_view_get
 * @since 1.8
 *
 * Get internal ewk_view object from web object.
 *
 * @param[out] ret
 *
 * @see elm_web_webkit_view_get
 *
 * @ingroup Web
 */
#define elm_obj_web_webkit_view_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_WEBKIT_VIEW_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_web_window_create_hook_set
 * @since 1.8
 *
 * Sets the function to call when a new window is requested
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_web_window_create_hook_set
 *
 * @ingroup Web
 */
#define elm_obj_web_window_create_hook_set(func, data) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_WINDOW_CREATE_HOOK_SET), EO_TYPECHECK(Elm_Web_Window_Open, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_web_dialog_alert_hook_set
 * @since 1.8
 *
 * Sets the function to call when an alert dialog
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_web_dialog_alert_hook_set
 *
 * @ingroup Web
 */
#define elm_obj_web_dialog_alert_hook_set(func, data) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_DIALOG_ALERT_HOOK_SET), EO_TYPECHECK(Elm_Web_Dialog_Alert, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_web_dialog_confirm_hook_set
 * @since 1.8
 *
 * Sets the function to call when an confirm dialog
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_web_dialog_confirm_hook_set
 *
 * @ingroup Web
 */
#define elm_obj_web_dialog_confirm_hook_set(func, data) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_DIALOG_CONFIRM_HOOK_SET), EO_TYPECHECK(Elm_Web_Dialog_Confirm, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_web_dialog_prompt_hook_set
 * @since 1.8
 *
 * Sets the function to call when an prompt dialog
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_web_dialog_prompt_hook_set
 *
 * @ingroup Web
 */
#define elm_obj_web_dialog_prompt_hook_set(func, data) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_DIALOG_PROMPT_HOOK_SET), EO_TYPECHECK(Elm_Web_Dialog_Prompt, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_web_dialog_file_selector_hook_set
 * @since 1.8
 *
 * Sets the function to call when an file selector dialog
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_web_dialog_file_selector_hook_set
 *
 * @ingroup Web
 */
#define elm_obj_web_dialog_file_selector_hook_set(func, data) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_DIALOG_FILE_SELECTOR_HOOK_SET), EO_TYPECHECK(Elm_Web_Dialog_File_Selector, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_web_console_message_hook_set
 * @since 1.8
 *
 * Sets the function to call when a console message is emitted from JS
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_web_console_message_hook_set
 *
 * @ingroup Web
 */
#define elm_obj_web_console_message_hook_set(func, data) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_CONSOLE_MESSAGE_HOOK_SET), EO_TYPECHECK(Elm_Web_Console_Message, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_web_useragent_set
 * @since 1.8
 *
 * Change useragent of a elm_web object
 *
 * @param[in] user_agent
 *
 * @see elm_web_useragent_set
 *
 * @ingroup Web
 */
#define elm_obj_web_useragent_set(user_agent) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_USERAGENT_SET), EO_TYPECHECK(const char *, user_agent)

/**
 * @def elm_obj_web_useragent_get
 * @since 1.8
 *
 * Return current useragent of elm_web object
 *
 * @param[out] ret
 *
 * @see elm_web_useragent_get
 *
 * @ingroup Web
 */
#define elm_obj_web_useragent_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_USERAGENT_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_web_tab_propagate_get
 * @since 1.8
 *
 * Get the status of the tab propagation
 *
 * @param[out] ret
 *
 * @see elm_web_tab_propagate_get
 *
 * @ingroup Web
 */
#define elm_obj_web_tab_propagate_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TAB_PROPAGATE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_tab_propagate_set
 * @since 1.8
 *
 * Sets whether to use tab propagation
 *
 * @param[in] propagate
 *
 * @see elm_web_tab_propagate_set
 *
 * @ingroup Web
 */
#define elm_obj_web_tab_propagate_set(propagate) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TAB_PROPAGATE_SET), EO_TYPECHECK(Eina_Bool, propagate)

/**
 * @def elm_obj_web_url_set
 * @since 1.8
 *
 * Sets the URL for the web object
 *
 * @param[in] url
 * @param[out] ret
 *
 * @see elm_web_url_set
 *
 * @ingroup Web
 */
#define elm_obj_web_url_set(url, ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_URL_SET), EO_TYPECHECK(const char *, url), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_url_get
 * @since 1.8
 *
 * Get the current URL for the object
 *
 * @param[out] ret
 *
 * @see elm_web_url_get
 *
 * @ingroup Web
 */
#define elm_obj_web_url_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_URL_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_web_html_string_load
 * @since 1.9
 *
 * Loads the specified @a html string as the content of the web object
 *
 * @param[in] html
 * @param[in] base_url
 * @param[in] unreachable_url
 * @param[out] ret
 *
 * @see elm_web_html_string_load
 *
 * @ingroup Web
 */
#define elm_obj_web_html_string_load(html, base_url, unreachable_url, ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_HTML_STRING_LOAD), EO_TYPECHECK(const char *, html), EO_TYPECHECK(const char *, base_url), EO_TYPECHECK(const char *, unreachable_url), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_title_get
 * @since 1.8
 *
 * Get the current title
 *
 * @param[out] ret
 *
 * @see elm_web_title_get
 *
 * @ingroup Web
 */
#define elm_obj_web_title_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TITLE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_web_bg_color_set
 * @since 1.8
 *
 * Sets the background color to be used by the web object
 *
 * @param[in] r
 * @param[in] g
 * @param[in] b
 * @param[in] a
 *
 * @see elm_web_bg_color_set
 *
 * @ingroup Web
 */
#define elm_obj_web_bg_color_set(r, g, b, a) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_BG_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def elm_obj_web_bg_color_get
 * @since 1.8
 *
 * Get the background color to be used by the web object
 *
 * @param[out] r
 * @param[out] g
 * @param[out] b
 * @param[out] a
 *
 * @see elm_web_bg_color_get
 *
 * @ingroup Web
 */
#define elm_obj_web_bg_color_get(r, g, b, a) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_BG_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def elm_obj_web_selection_get
 * @since 1.8
 *
 * Get a copy of the currently selected text
 *
 * @param[out] ret
 *
 * @see elm_web_selection_get
 *
 * @ingroup Web
 */
#define elm_obj_web_selection_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_SELECTION_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_web_popup_selected_set
 * @since 1.8
 *
 * Tells the web object which index in the currently open popup was selected
 *
 * @param[in] idx
 *
 * @see elm_web_popup_selected_set
 *
 * @ingroup Web
 */
#define elm_obj_web_popup_selected_set(idx) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_POPUP_SELECTED_SET), EO_TYPECHECK(int, idx)

/**
 * @def elm_obj_web_popup_destroy
 * @since 1.8
 *
 * Dismisses an open dropdown popup
 *
 * @param[out] ret
 *
 * @see elm_web_popup_destroy
 *
 * @ingroup Web
 */
#define elm_obj_web_popup_destroy(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_POPUP_DESTROY), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_text_search
 * @since 1.8
 *
 * Searches the given string in a document.
 *
 * @param[in] string
 * @param[in] case_sensitive
 * @param[in] forward
 * @param[in] wrap
 * @param[out] ret
 *
 * @see elm_web_text_search
 *
 * @ingroup Web
 */
#define elm_obj_web_text_search(string, case_sensitive, forward, wrap, ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TEXT_SEARCH), EO_TYPECHECK(const char *, string), EO_TYPECHECK(Eina_Bool, case_sensitive), EO_TYPECHECK(Eina_Bool, forward), EO_TYPECHECK(Eina_Bool, wrap), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_text_matches_mark
 * @since 1.8
 *
 * Marks matches of the given string in a document.
 *
 * @param[in] string
 * @param[in] case_sensitive
 * @param[in] highlight
 * @param[in] limit
 * @param[out] ret
 *
 * @see elm_web_text_matches_mark
 *
 * @ingroup Web
 */
#define elm_obj_web_text_matches_mark(string, case_sensitive, highlight, limit, ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_MARK), EO_TYPECHECK(const char *, string), EO_TYPECHECK(Eina_Bool, case_sensitive), EO_TYPECHECK(Eina_Bool, highlight), EO_TYPECHECK(unsigned int, limit), EO_TYPECHECK(unsigned int *, ret)

/**
 * @def elm_obj_web_text_matches_unmark_all
 * @since 1.8
 *
 * Clears all marked matches in the document
 *
 * @param[out] ret
 *
 * @see elm_web_text_matches_unmark_all
 *
 * @ingroup Web
 */
#define elm_obj_web_text_matches_unmark_all(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_UNMARK_ALL), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_text_matches_highlight_set
 * @since 1.8
 *
 * Sets whether to highlight the matched marks
 *
 * @param[in] highlight
 * @param[out] ret
 *
 * @see elm_web_text_matches_highlight_set
 *
 * @ingroup Web
 */
#define elm_obj_web_text_matches_highlight_set(highlight, ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_HIGHLIGHT_SET), EO_TYPECHECK(Eina_Bool, highlight), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_text_matches_highlight_get
 * @since 1.8
 *
 * Get whether highlighting marks is enabled
 *
 * @param[out] ret
 *
 * @see elm_web_text_matches_highlight_get
 *
 * @ingroup Web
 */
#define elm_obj_web_text_matches_highlight_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_TEXT_MATCHES_HIGHLIGHT_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_load_progress_get
 * @since 1.8
 *
 * Get the overall loading progress of the page
 *
 * @param[out] ret
 *
 * @see elm_web_load_progress_get
 *
 * @ingroup Web
 */
#define elm_obj_web_load_progress_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_LOAD_PROGRESS_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_web_stop
 * @since 1.8
 *
 * Stops loading the current page
 *
 * @param[out] ret
 *
 * @see elm_web_stop
 *
 * @ingroup Web
 */
#define elm_obj_web_stop(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_STOP), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_reload
 * @since 1.8
 *
 * Requests a reload of the current document in the object
 *
 * @param[out] ret
 *
 * @see elm_web_reload
 *
 * @ingroup Web
 */
#define elm_obj_web_reload(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_RELOAD), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_reload_full
 * @since 1.8
 *
 * Requests a reload of the current document, avoiding any existing caches
 *
 * @param[out] ret
 *
 * @see elm_web_reload_full
 *
 * @ingroup Web
 */
#define elm_obj_web_reload_full(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_RELOAD_FULL), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_back
 * @since 1.8
 *
 * Goes back one step in the browsing history
 *
 * @param[out] ret
 *
 * @see elm_web_back
 *
 * @ingroup Web
 */
#define elm_obj_web_back(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_BACK), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_forward
 * @since 1.8
 *
 * Goes forward one step in the browsing history
 *
 * @param[out] ret
 *
 * @see elm_web_forward
 *
 * @ingroup Web
 */
#define elm_obj_web_forward(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_FORWARD), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_navigate
 * @since 1.8
 *
 * Jumps the given number of steps in the browsing history
 *
 * @param[in] steps
 * @param[out] ret
 *
 * @see elm_web_navigate
 *
 * @ingroup Web
 */
#define elm_obj_web_navigate(steps, ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_NAVIGATE), EO_TYPECHECK(int, steps), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_back_possible_get
 * @since 1.8
 *
 * Queries whether it's possible to go back in history
 *
 * @param[out] ret
 *
 * @see elm_web_back_possible_get
 *
 * @ingroup Web
 */
#define elm_obj_web_back_possible_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_BACK_POSSIBLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_forward_possible_get
 * @since 1.8
 *
 * Queries whether it's possible to go forward in history
 *
 * @param[out] ret
 *
 * @see elm_web_forward_possible_get
 *
 * @ingroup Web
 */
#define elm_obj_web_forward_possible_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_FORWARD_POSSIBLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_navigate_possible_get
 * @since 1.8
 *
 * Queries whether it's possible to jump the given number of steps
 *
 * @param[in] steps
 * @param[out] ret
 *
 * @see elm_web_navigate_possible_get
 *
 * @ingroup Web
 */
#define elm_obj_web_navigate_possible_get(steps, ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_NAVIGATE_POSSIBLE_GET), EO_TYPECHECK(int, steps), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_history_enabled_get
 * @since 1.8
 *
 * Get whether browsing history is enabled for the given object
 *
 * @param[out] ret
 *
 * @see elm_web_history_enabled_get
 *
 * @ingroup Web
 */
#define elm_obj_web_history_enabled_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_HISTORY_ENABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_web_history_enabled_set
 * @since 1.8
 *
 * Enables or disables the browsing history
 *
 * @param[in] enable
 *
 * @see elm_web_history_enabled_set
 *
 * @ingroup Web
 */
#define elm_obj_web_history_enabled_set(enable) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_HISTORY_ENABLED_SET), EO_TYPECHECK(Eina_Bool, enable)

/**
 * @def elm_obj_web_zoom_set
 * @since 1.8
 *
 * Sets the zoom level of the web object
 *
 * @param[in] zoom
 *
 * @see elm_web_zoom_set
 *
 * @ingroup Web
 */
#define elm_obj_web_zoom_set(zoom) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_ZOOM_SET), EO_TYPECHECK(double, zoom)

/**
 * @def elm_obj_web_zoom_get
 * @since 1.8
 *
 * Get the current zoom level set on the web object
 *
 * @param[out] ret
 *
 * @see elm_web_zoom_get
 *
 * @ingroup Web
 */
#define elm_obj_web_zoom_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_ZOOM_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_web_zoom_mode_set
 * @since 1.8
 *
 * Sets the zoom mode to use
 *
 * @param[in] mode
 *
 * @see elm_web_zoom_mode_set
 *
 * @ingroup Web
 */
#define elm_obj_web_zoom_mode_set(mode) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_ZOOM_MODE_SET), EO_TYPECHECK(Elm_Web_Zoom_Mode, mode)

/**
 * @def elm_obj_web_zoom_mode_get
 * @since 1.8
 *
 * Get the currently set zoom mode
 *
 * @param[out] ret
 *
 * @see elm_web_zoom_mode_get
 *
 * @ingroup Web
 */
#define elm_obj_web_zoom_mode_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_ZOOM_MODE_GET), EO_TYPECHECK(Elm_Web_Zoom_Mode *, ret)

/**
 * @def elm_obj_web_region_show
 * @since 1.8
 *
 * Shows the given region in the web object
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see elm_web_region_show
 *
 * @ingroup Web
 */
#define elm_obj_web_region_show(x, y, w, h) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_REGION_SHOW), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def elm_obj_web_region_bring_in
 * @since 1.8
 *
 * Brings in the region to the visible area
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see elm_web_region_bring_in
 *
 * @ingroup Web
 */
#define elm_obj_web_region_bring_in(x, y, w, h) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_REGION_BRING_IN), EO_TYPECHECK(int, x), EO_TYPECHECK(int, y), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def elm_obj_web_inwin_mode_set
 * @since 1.8
 *
 * Sets the default dialogs to use an Inwin instead of a normal window
 *
 * @param[in] value
 *
 * @see elm_web_inwin_mode_set
 *
 * @ingroup Web
 */
#define elm_obj_web_inwin_mode_set(value) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_INWIN_MODE_SET), EO_TYPECHECK(Eina_Bool, value)

/**
 * @def elm_obj_web_inwin_mode_get
 * @since 1.8
 *
 * Get whether Inwin mode is set for the current object
 *
 * @param[out] ret
 *
 * @see elm_web_inwin_mode_get
 *
 * @ingroup Web
 */
#define elm_obj_web_inwin_mode_get(ret) ELM_OBJ_WEB_ID(ELM_OBJ_WEB_SUB_ID_INWIN_MODE_GET), EO_TYPECHECK(Eina_Bool *, ret)
