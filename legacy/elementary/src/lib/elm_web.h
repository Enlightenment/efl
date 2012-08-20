/**
 * @defgroup Web Web
 * @ingroup Elementary
 *
 * @image html web_inheritance_tree.png
 * @image latex web_inheritance_tree.eps
 *
 * @image html img/widget/web/preview-00.png
 * @image latex img/widget/web/preview-00.eps
 *
 * A web widget is used for displaying web pages (HTML/CSS/JS)
 * using WebKit-EFL. You must have compiled Elementary with
 * ewebkit support.
 *
 * Signals that you can add callbacks for are:
 * @li "download,request": A file download has been requested. Event info is
 * a pointer to a Elm_Web_Download
 * @li "editorclient,contents,changed": Editor client's contents changed
 * @li "editorclient,selection,changed": Editor client's selection changed
 * @li "frame,created": A new frame was created. Event info is an
 * Evas_Object which can be handled with WebKit's ewk_frame API
 * @li "icon,received": An icon was received by the main frame
 * @li "inputmethod,changed": Input method changed. Event info is an
 * Eina_Bool indicating whether it's enabled or not
 * @li "js,windowobject,clear": JS window object has been cleared
 * @li "link,hover,in": Mouse cursor is hovering over a link. Event info
 * is a char *link[2], where the first string contains the URL the link
 * points to, and the second one the title of the link
 * @li "link,hover,out": Mouse cursor left the link
 * @li "load,document,finished": Loading of a document finished. Event info
 * is the frame that finished loading
 * @li "load,error": Load failed. Event info is a pointer to
 * Elm_Web_Frame_Load_Error
 * @li "load,finished": Load finished. Event info is NULL on success, on
 * error it's a pointer to Elm_Web_Frame_Load_Error
 * @li "load,newwindow,show": A new window was created and is ready to be
 * shown
 * @li "load,progress": Overall load progress. Event info is a pointer to
 * a double containing a value between 0.0 and 1.0
 * @li "load,provisional": Started provisional load
 * @li "load,started": Loading of a document started
 * @li "menubar,visible,get": Queries if the menubar is visible. Event info
 * is a pointer to Eina_Bool where the callback should set EINA_TRUE if
 * the menubar is visible, or EINA_FALSE in case it's not
 * @li "menubar,visible,set": Informs menubar visibility. Event info is
 * an Eina_Bool indicating the visibility
 * @li "popup,created": A dropdown widget was activated, requesting its
 * popup menu to be created. Event info is a pointer to Elm_Web_Menu
 * @li "popup,willdelete": The web object is ready to destroy the popup
 * object created. Event info is a pointer to Elm_Web_Menu
 * @li "ready": Page is fully loaded
 * @li "scrollbars,visible,get": Queries visibility of scrollbars. Event
 * info is a pointer to Eina_Bool where the visibility state should be set
 * @li "scrollbars,visible,set": Informs scrollbars visibility. Event info
 * is an Eina_Bool with the visibility state set
 * @li "statusbar,text,set": Text of the statusbar changed. Even info is
 * a string with the new text
 * @li "statusbar,visible,get": Queries visibility of the status bar.
 * Event info is a pointer to Eina_Bool where the visibility state should be
 * set.
 * @li "statusbar,visible,set": Informs statusbar visibility. Event info is
 * an Eina_Bool with the visibility value
 * @li "title,changed": Title of the main frame changed. Event info is a
 * string with the new title
 * @li "toolbars,visible,get": Queries visibility of toolbars. Event info
 * is a pointer to Eina_Bool where the visibility state should be set
 * @li "toolbars,visible,set": Informs the visibility of toolbars. Event
 * info is an Eina_Bool with the visibility state
 * @li "tooltip,text,set": Show and set text of a tooltip. Event info is
 * a string with the text to show
 * @li "uri,changed": URI of the main frame changed. Event info is a string
 * with the new URI
 * @li "view,resized": The web object internal's view changed sized
 * @li "windows,close,request": A JavaScript request to close the current
 * window was requested
 * @li "zoom,animated,end": Animated zoom finished
 *
 * available styles:
 * - default
 *
 * An example of use of web:
 *
 * - @ref web_example_01
 * - @ref web_example_02
 */

/**
 * @addtogroup Web
 * @{
 */

/**
 * Structure used to report load errors.
 *
 * Load errors are reported as signal by elm_web. All the strings are
 * temporary references and should @b not be used after the signal
 * callback returns. If it's required, make copies with strdup() or
 * eina_stringshare_add() (they are not even guaranteed to be
 * stringshared, so must use eina_stringshare_add() and not
 * eina_stringshare_ref()).
 */
typedef struct _Elm_Web_Frame_Load_Error Elm_Web_Frame_Load_Error;

/**
 * Structure used to report load errors.
 *
 * Load errors are reported as signal by elm_web. All the strings are
 * temporary references and should @b not be used after the signal
 * callback returns. If it's required, make copies with strdup() or
 * eina_stringshare_add() (they are not even guaranteed to be
 * stringshared, so must use eina_stringshare_add() and not
 * eina_stringshare_ref()).
 */
struct _Elm_Web_Frame_Load_Error
{
   int          code; /**< Numeric error code */
   Eina_Bool    is_cancellation; /**< Error produced by canceling a request */
   const char  *domain; /**< Error domain name */
   const char  *description; /**< Error description (already localized) */
   const char  *failing_url; /**< The URL that failed to load */
   Evas_Object *frame; /**< Frame object that produced the error */
};

/**
 * The possibles types that the items in a menu can be
 */
typedef enum
{
   ELM_WEB_MENU_SEPARATOR,
   ELM_WEB_MENU_GROUP,
   ELM_WEB_MENU_OPTION
} Elm_Web_Menu_Item_Type;

/**
 * Structure describing the items in a menu
 */
typedef struct _Elm_Web_Menu_Item Elm_Web_Menu_Item;

/**
 * Structure describing the items in a menu
 */
struct _Elm_Web_Menu_Item
{
   const char            *text; /**< The text for the item */
   Elm_Web_Menu_Item_Type type; /**< The type of the item */
};

/**
 * Structure describing the menu of a popup
 *
 * This structure will be passed as the @c event_info for the "popup,create"
 * signal, which is emitted when a dropdown menu is opened. Users wanting
 * to handle these popups by themselves should listen to this signal and
 * set the @c handled property of the struct to @c EINA_TRUE. Leaving this
 * property as @c EINA_FALSE means that the user will not handle the popup
 * and the default implementation will be used.
 *
 * When the popup is ready to be dismissed, a "popup,willdelete" signal
 * will be emitted to notify the user that it can destroy any objects and
 * free all data related to it.
 *
 * @see elm_web_popup_selected_set()
 * @see elm_web_popup_destroy()
 *
 * @ingroup Web
 */
typedef struct _Elm_Web_Menu Elm_Web_Menu;

/**
 * Structure describing the menu of a popup
 *
 * This structure will be passed as the @c event_info for the "popup,create"
 * signal, which is emitted when a dropdown menu is opened. Users wanting
 * to handle these popups by themselves should listen to this signal and
 * set the @c handled property of the struct to @c EINA_TRUE. Leaving this
 * property as @c EINA_FALSE means that the user will not handle the popup
 * and the default implementation will be used.
 *
 * When the popup is ready to be dismissed, a "popup,willdelete" signal
 * will be emitted to notify the user that it can destroy any objects and
 * free all data related to it.
 *
 * @see elm_web_popup_selected_set()
 * @see elm_web_popup_destroy()
 *
 * @ingroup Web
 */
struct _Elm_Web_Menu
{
   Eina_List *items; /**< List of #Elm_Web_Menu_Item */
   int        x; /**< The X position of the popup, relative to the elm_web object */
   int        y; /**< The Y position of the popup, relative to the elm_web object */
   int        width; /**< Width of the popup menu */
   int        height; /**< Height of the popup menu */

   Eina_Bool  handled : 1; /**< Set to @c EINA_TRUE by the user to indicate that the popup has been handled and the default implementation should be ignored. Leave as @c EINA_FALSE otherwise. */
};

typedef struct _Elm_Web_Download Elm_Web_Download;
struct _Elm_Web_Download
{
   const char *url;
};

/**
 * Types of zoom available.
 */
typedef enum
{
   ELM_WEB_ZOOM_MODE_MANUAL = 0, /**< Zoom controlled normally by elm_web_zoom_set */
   ELM_WEB_ZOOM_MODE_AUTO_FIT, /**< Zoom until content fits in web object */
   ELM_WEB_ZOOM_MODE_AUTO_FILL, /**< Zoom until content fills web object */
   ELM_WEB_ZOOM_MODE_LAST /**< Sentinel value to indicate end */
} Elm_Web_Zoom_Mode;

/**
 * Opaque handler containing the features (such as statusbar, menubar, etc)
 * that are to be set on a newly requested window.
 */
typedef struct _Elm_Web_Window_Features Elm_Web_Window_Features;


/**
 * Definitions of web window features.
 *
 */
typedef enum
{
   ELM_WEB_WINDOW_FEATURE_TOOLBAR,
   ELM_WEB_WINDOW_FEATURE_STATUSBAR,
   ELM_WEB_WINDOW_FEATURE_SCROLLBARS,
   ELM_WEB_WINDOW_FEATURE_MENUBAR,
   ELM_WEB_WINDOW_FEATURE_LOCATIONBAR,
   ELM_WEB_WINDOW_FEATURE_FULLSCREEN
} Elm_Web_Window_Feature_Flag;

/**
 * Callback type for the create_window hook.
 *
 * @param data User data pointer set when setting the hook function.
 * @param obj The elm_web object requesting the new window.
 * @param js Set to @c EINA_TRUE if the request was originated from
 * JavaScript. @c EINA_FALSE otherwise.
 * @param window_features A pointer of #Elm_Web_Window_Features indicating
 * the features requested for the new window.
 *
 * @return The @c elm_web widget where the request will be loaded. That is,
 * if a new window or tab is created, the elm_web widget in it should be
 * returned, and @b NOT the window object. Returning @c NULL should cancel
 * the request.
 *
 * @see elm_web_window_create_hook_set()
 *
 * @ingroup Web
 */
typedef Evas_Object *(*Elm_Web_Window_Open)(void *data, Evas_Object *obj, Eina_Bool js, const Elm_Web_Window_Features *window_features);

/**
 * Callback type for the JS alert hook.
 *
 * @param data User data pointer set when setting the hook function.
 * @param obj The elm_web object requesting the new window.
 * @param message The message to show in the alert dialog.
 *
 * @return The object representing the alert dialog.
 * Elm_Web will run a second main loop to handle the dialog and normal
 * flow of the application will be restored when the object is deleted, so
 * the user should handle the popup properly in order to delete the object
 * when the action is finished.
 * If the function returns @c NULL the popup will be ignored.
 *
 * @see elm_web_dialog_alert_hook_set()
 *
 * @ingroup Web
 */
typedef Evas_Object *(*Elm_Web_Dialog_Alert)(void *data, Evas_Object *obj, const char *message);

/**
 * Callback type for the JS confirm hook.
 *
 * @param data User data pointer set when setting the hook function.
 * @param obj The elm_web object requesting the new window.
 * @param message The message to show in the confirm dialog.
 * @param ret Pointer to store the user selection. @c EINA_TRUE if
 * the user selected @c Ok, @c EINA_FALSE otherwise.
 *
 * @return The object representing the confirm dialog.
 * Elm_Web will run a second main loop to handle the dialog and normal
 * flow of the application will be restored when the object is deleted, so
 * the user should handle the popup properly in order to delete the object
 * when the action is finished.
 * If the function returns @c NULL the popup will be ignored.
 *
 * @see elm_web_dialog_confirm_hook_set()
 *
 * @ingroup Web
 */
typedef Evas_Object *(*Elm_Web_Dialog_Confirm)(void *data, Evas_Object *obj, const char *message, Eina_Bool *ret);

/**
 * Callback type for the JS prompt hook.
 *
 * @param data User data pointer set when setting the hook function.
 * @param obj The elm_web object requesting the new window.
 * @param message The message to show in the prompt dialog.
 * @param def_value The default value to present the user in the entry
 * @param value Pointer to store the value given by the user. Must
 * be a malloc'ed string or @c NULL if the user canceled the popup.
 * @param ret Pointer to store the user selection. @c EINA_TRUE if
 * the user selected @c Ok, @c EINA_FALSE otherwise.
 *
 * @return The object representing the prompt dialog.
 * Elm_Web will run a second main loop to handle the dialog and normal
 * flow of the application will be restored when the object is deleted, so
 * the user should handle the popup properly in order to delete the object
 * when the action is finished.
 * If the function returns @c NULL the popup will be ignored.
 *
 * @see elm_web_dialog_prompt_hook_set()
 *
 * @ingroup Web
 */
typedef Evas_Object *(*Elm_Web_Dialog_Prompt)(void *data, Evas_Object *obj, const char *message, const char *def_value, const char **value, Eina_Bool *ret);

/**
 * Callback type for the JS file selector hook.
 *
 * @param data User data pointer set when setting the hook function.
 * @param obj The elm_web object requesting the new window.
 * @param allows_multiple @c EINA_TRUE if multiple files can be selected.
 * @param accept_types Mime types accepted.
 * @param selected Pointer to store the list of malloc'ed strings
 * containing the path to each file selected. Must be @c NULL if the file
 * dialog is canceled.
 * @param ret Pointer to store the user selection. @c EINA_TRUE if
 * the user selected @c Ok, @c EINA_FALSE otherwise.
 *
 * @return The object representing the file selector dialog.
 * Elm_Web will run a second main loop to handle the dialog and normal
 * flow of the application will be restored when the object is deleted, so
 * the user should handle the popup properly in order to delete the object
 * when the action is finished.
 * If the function returns @c NULL the popup will be ignored.
 *
 * @see elm_web_dialog_file selector_hook_set()
 *
 * @ingroup Web
 */
typedef Evas_Object *(*Elm_Web_Dialog_File_Selector)(void *data, Evas_Object *obj, Eina_Bool allows_multiple, Eina_List *accept_types, Eina_List **selected, Eina_Bool *ret);

/**
 * Callback type for the JS console message hook.
 *
 * When a console message is added from JavaScript, any set function to the
 * console message hook will be called for the user to handle. There is no
 * default implementation of this hook.
 *
 * @param data User data pointer set when setting the hook function.
 * @param obj The elm_web object that originated the message.
 * @param message The message sent.
 * @param line_number The line number.
 * @param source_id Source id.
 *
 * @see elm_web_console_message_hook_set()
 *
 * @ingroup Web
 */
typedef void (*Elm_Web_Console_Message)(void *data, Evas_Object *obj, const char *message, unsigned int line_number, const char *source_id);

/**
 * Add a new web object to the parent.
 *
 * @param parent The parent object.
 * @return The new object or @c NULL if it cannot be created.
 *
 * @see elm_web_uri_set()
 * @see elm_web_webkit_view_get()
 *
 * @ingroup Web
 */
EAPI Evas_Object      *elm_web_add(Evas_Object *parent);

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
 * Gets the status of the tab propagation
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
 * Sets the URI for the web object
 *
 * It must be a full URI, with resource included, in the form
 * http://www.enlightenment.org or file:///tmp/something.html
 *
 * @param obj The web object
 * @param uri The URI to set
 * @return @c EINA_TRUE if the URI could be set, @c EINA_FALSE if an error occurred.
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_uri_set(Evas_Object *obj, const char *uri);

/**
 * Gets the current URI for the object
 *
 * The returned string must not be freed and is guaranteed to be
 * stringshared.
 *
 * @param obj The web object
 * @return A stringshared internal string with the current URI, or @c NULL on
 * failure
 *
 * @ingroup Web
 */
EAPI const char       *elm_web_uri_get(const Evas_Object *obj);

/**
 * Gets the current title
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
 * Gets the background color to be used by the web object
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
 * Gets a copy of the currently selected text
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
 * Gets whether highlighting marks is enabled
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
 * Gets the overall loading progress of the page
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
 * Gets whether browsing history is enabled for the given object
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
 * Gets the current zoom level set on the web object
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
 * Sets the zoom mode to use
 *
 * The modes can be any of those defined in ::Elm_Web_Zoom_Mode, except
 * ::ELM_WEB_ZOOM_MODE_LAST. The default is ::ELM_WEB_ZOOM_MODE_MANUAL.
 *
 * ::ELM_WEB_ZOOM_MODE_MANUAL means the zoom level will be controlled
 * with the elm_web_zoom_set() function.
 * ::ELM_WEB_ZOOM_MODE_AUTO_FIT will calculate the needed zoom level to
 * make sure the entirety of the web object's contents are shown.
 * ::ELM_WEB_ZOOM_MODE_AUTO_FILL will calculate the needed zoom level to
 * fit the contents in the web object's size, without leaving any space
 * unused.
 *
 * @param obj The web object
 * @param mode The mode to set
 *
 * @ingroup Web
 */
EAPI void              elm_web_zoom_mode_set(Evas_Object *obj, Elm_Web_Zoom_Mode mode);

/**
 * Gets the currently set zoom mode
 *
 * @param obj The web object
 *
 * @return The current zoom mode set for the object,
 * or ::ELM_WEB_ZOOM_MODE_LAST on error
 *
 * @ingroup Web
 */
EAPI Elm_Web_Zoom_Mode elm_web_zoom_mode_get(const Evas_Object *obj);

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
 * Gets whether Inwin mode is set for the current object
 *
 * @param obj The web object
 *
 * @return @c EINA_TRUE if Inwin mode is set, @c EINA_FALSE otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool         elm_web_inwin_mode_get(const Evas_Object *obj);

EAPI void              elm_web_window_features_ref(Elm_Web_Window_Features *wf);
EAPI void              elm_web_window_features_unref(Elm_Web_Window_Features *wf);

/**
 * Gets boolean properties from Elm_Web_Window_Features
 * (such as statusbar, menubar, etc) that are on a window.
 *
 * @param wf The web window features object
 * @param flag The web window feature flag whose value is required.
 *
 * @return @c EINA_TRUE if the flag is set, @c EINA_FALSE otherwise
 *
 * @ingroup Web
 */
EAPI Eina_Bool              elm_web_window_features_property_get(const Elm_Web_Window_Features *wf, Elm_Web_Window_Feature_Flag flag);

/**
 *
 * TODO : Add documentation.
 *
 * @param wf The web window features object
 * @param x, y, w, h - the co-ordinates of the web view window.
 *
 * @ingroup Web
 */
EAPI void              elm_web_window_features_region_get(const Elm_Web_Window_Features *wf, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @}
 */
